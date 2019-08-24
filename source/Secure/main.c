#include <arm_cmse.h>
#include <stdio.h>
#include "NuMicro.h"                      /* Device header */
#include <string.h>
#include "partition_M2351.h"
#include <mkromlib.h>
#include <APA102.h>
#include <ledpattern.h>
#include "Adafruit_GFX.h"
#define NEXT_BOOT_BASE  0x10040000
#define JUMP_HERE       0xe7fee7ff      /* Instruction Code of "B ." */

#define PAGE_SIZE 0x800
#define Secure_end 0x40000
#define LockReg Secure_end - PAGE_SIZE
#define PWNED Secure_end - PAGE_SIZE + 0x100
#define key_l 24
//#define KHASH LockReg + 0x10

unsigned long Delayms (unsigned long ms);
/* typedef for NonSecure callback functions */
typedef __NONSECURE_CALL int32_t (*NonSecure_funcptr)(uint32_t);

volatile uint32_t g_u32SysTick;
volatile uint32_t g_ClearButtonHistoryTimer;
int g_ButtonHistoryArray[10] = {0};
int g_ButtonHistoryPointer = 0;

uint8_t led_position[24][2] =   {{60, 1},
        {64, 0},
        {65, 20},
        {58, 10},
        {56, 45},
        {44, 76},
        {29, 89},
        {50, 84},
        {47, 98},
        {21, 80},
        {19, 69},
        {24, 56},
        {19, 57},
        {23, 47},
        {0, 56},
        {25, 39},
        {41, 46},
        {33, 30},
        {1, 29},
        {4, 5},
        {40, 21},
        {48, 15},
        {41, 11},
        {49, 5}
        };


extern uint8_t    g_PixelBrightness;                             // Global brightness setting
XTRNG_T g_rng;
const int specialCode[10] = {1,1,2,2,4,3,4,3,5,6};

char uid[0x20] = {0};

char key[key_l][32+4] = {
        "97f17fdeabd4bcd44bbd67f823b16949",
        "c9bc057e4314757277d28e1b101a744a",
        "8aee2bb61b7281513f70a7bee0def9f8",
        "6e76cebce986763b0f2fbcb2f42d7cbc",
        "b4a2b506da2cdf8c1a7d6cd4c41c3f6e",
        "a9a809d37f4a54edf4ba8d76ade09a93",
        "35051573f8f64718338813a92dd2d3e6",
        "978d0125022baae07bc8c339ace7bf39",
        "2d2d4c052fc330346b5c13e675a53a0e",
        "5b01f24f9d8fd910c202fe000981a432",
        "c2714faa108d1201b942bdc85afdf045",
        "26d2f1eede61de4c28a58d30559b07a3",
        "ec02ea0b09473404a1708306069e6b75",
        "f6a38ad5aece11b823465dc8f387589c",
        "afb1f1c318f67e478c4374fd130d3ec3",
        "81de14074b926b1400038d4b6b62272e",
        "03c5d7d78ead0a43b6cac9ad03606d67",
        "38d5b91859836721b769f1f732034a3c",
        "194f2d629d746da5af09d03b11312b51",
        "fd1ede4a07ca4c1cf19304f20227f854",
        "c549eab0b15054c9926b9c55a99bb0d8",
        "6036ed7d8a58c54e33ac92bb494d9b78",
        "c7cf36f244a85e3b15cb4710036b56d2",
        "7d46ffa7ac16c5e9b12319404b578f02"
};

uint8_t khash[key_l][0x20];

/*----------------------------------------------------------------------------
  Secure functions exported to NonSecure application
  Must place in Non-secure Callable
 *----------------------------------------------------------------------------*/
__NONSECURE_ENTRY
int32_t nsc_SetPixelColor(uint16_t n, uint32_t c)
{
    //printf("[Secure]SetPixelColor\n");
    SetPixelColor(n, c);
    return 0;
}
__NONSECURE_ENTRY
int32_t nsc_SetBrigntness(uint8_t b)
{
    //printf("[Secure]SetBrigntness\n");
    g_PixelBrightness = b;
    return 0;
}

__NONSECURE_ENTRY
int32_t nsc_PixelShow()
{
    //printf("[Secure]PixelShow\n");
    PixelShow();
    return 0;
}

__NONSECURE_ENTRY
int32_t nsc_SetPixelColorRGB(uint16_t n, uint8_t r, uint8_t g, uint8_t b)
{
    //printf("[Secure]SetPixelColorRGB\n");
    SetPixelColorRGB(n,r,g,b);
    return 0;
}

__NONSECURE_ENTRY
int32_t nsc_ClearPixel()
{
    //printf("[Secure]ClearPixels\n");
    ClearPixels();
    return 0;
}


uint32_t hardwareRandom(){

    uint32_t random = 0;

    /* Generate random number 1 */
    XTRNG_Random(&g_rng, &random, 4);

    return random;
}

__NONSECURE_ENTRY
uint32_t nsc_randomInt(){
    return hardwareRandom();
}


//Reading Lock Reg
__NONSECURE_ENTRY
uint32_t nsc_GetLockReg(void)
{
    FMC_Open();
    uint32_t reg = FMC_Read( LockReg );
    //printf("[Secure]LockReg = [0x%08x]\n", reg);
    FMC_Close();
    return reg;
}

__NONSECURE_ENTRY
uint32_t nsc_isPwned(void)
{
    FMC_Open();
    uint32_t flag = FMC_Read( PWNED );
    FMC_Close();

    //printf( "[isPwned] %p\n" , flag );

    return flag == 0xfaceb00c;
}


__NONSECURE_ENTRY
uint32_t nsc_check_pattern( uint32_t pattern ){
    uint32_t lock = nsc_GetLockReg();
    switch(pattern){
        case 0:
            return ((lock >> 0) & 1 ) && ((lock >> 1) & 1) && ((lock >> 2) & 1);
        case 1:
            return ((lock >> 3) & 1 ) && ((lock >> 4) & 1) && ((lock >> 5) & 1);
        case 2:
            return ((lock >> 6) & 1 ) && ((lock >> 7) & 1) && ((lock >> 8) & 1);
        case 3:
            return ((lock >> 9) & 1 ) && ((lock >> 10) & 1) && ((lock >> 11) & 1);
        case 4:
            return ((lock >> 12) & 1 ) && ((lock >> 13) & 1) && ((lock >> 14) & 1);
        case 5:
            return ((lock >> 15) & 1 ) && ((lock >> 16) & 1) && ((lock >> 17) & 1);
        case 6:
            return ((lock >> 18) & 1 ) && ((lock >> 19) & 1) && ((lock >> 20) & 1);
        case 7:
            return ((lock >> 21) & 1 ) && ((lock >> 22) & 1) && ((lock >> 23) & 1);
        case 8:
            return (lock >> 30) & 1; // for badge exploitation challenges
        case 9:
            return (lock >> 31) & 1;
        case 10:
            return nsc_isPwned();
        default:
            return 0;
    }
}


__NONSECURE_ENTRY
uint32_t nsc_display(uint32_t pattern,int32_t controlA,int32_t controlB){
    if( pattern > 10 ) return 0;
    if( nsc_check_pattern( pattern ) ){
        //printf("Unlocked!\n");
        switch(pattern){
        case 0:
            pattern1(controlA,controlB);
            break;
        case 1:
            pattern2(controlA,controlB);
            break;
        case 2:
            pattern3(controlA,controlB);
            break;
        case 3:
            pattern4(controlA,controlB);
            break;
        case 4:
            pattern5(controlA,controlB);
            break;
        case 5:
            pattern6(controlA,controlB);
            break;
        case 6:
            pattern7(controlA,controlB);
            break;
        case 7:
            pattern8(controlA,controlB);
            break;
        case 8:
            pattern9(controlA,controlB);
            break;
        case 9:
            pattern10(controlA,controlB);
            break;
        case 10:
            pattern11(controlA,controlB);
            break;
        default:
            break;
        }
    }
    else{
        //printf("Locked!\n");
        patternError();

    }


}

//Clear Lock Reg
__NONSECURE_ENTRY
uint32_t nsc_ClearRecord(void)
{
    printf("[Secure]Clear Lock Reg!\n");
    FMC_Open();
    FMC_ENABLE_AP_UPDATE();
    if(FMC_Erase(0x40000-0x800) != 0){
        printf("[Secure]Erase Failed\n");
        return 1;
    }
    printf("[Secure]Lock Reg = [0x%08x]\n",FMC_Read( LockReg ));
    FMC_Write(0x40000-0x800, 0);
    FMC_DISABLE_AP_UPDATE();
    printf("[Secure]Lock Reg = [0x%08x]\n",FMC_Read( LockReg ));
    FMC_Close();
    return 0;
}

//set Lock Reg, it will set 1 to the [data] bit
uint32_t WriteRecord(uint32_t data)
{
    printf( "writerecord: %d\n" , data );
    if( data > 23 && data < 30 ){
        return 0;
    }
    printf( "writerecord: %d\n" , data );
    FMC_Open();
    uint32_t conf = FMC_Read(0x40000-0x800);

    FMC_ENABLE_AP_UPDATE();
    if(FMC_Erase(0x40000-0x800) != 0){
        printf("Erase Failed\n");
    }
    printf("[Secure]Lock Reg = [0x%08x]\n",FMC_Read( LockReg ));
    FMC_Write(0x40000-0x800,conf | 1<<data);
    //FMC_DISABLE_AP_UPDATE();
    printf("[Secure]Lock Reg = [0x%08x]\n",FMC_Read( LockReg ));
    FMC_Close();
    return 0;
}

__NONSECURE_ENTRY
void nsc_WinSnake(){
    WriteRecord( 30 );
}

__NONSECURE_ENTRY
void nsc_Pwned_NS(){
    WriteRecord( 31 );
}

__NONSECURE_ENTRY
uint32_t nsc_GetSystemCoreClock(void)
{
    printf("[Secure]System core clock = %d.\n", SystemCoreClock);
    return SystemCoreClock;
}


void clearhistory(){
    printf("[Secure]Clear History!\n");
    memset(g_ButtonHistoryArray, 0, 10);
}

__NONSECURE_ENTRY
void nsc_CheckButtonSequence(int flag){
    g_ButtonHistoryArray[g_ButtonHistoryPointer++] = flag;
    g_ClearButtonHistoryTimer = g_u32SysTick+1000;
    if(g_ButtonHistoryPointer>=10){
        g_ButtonHistoryPointer = 0;
    }
    int BottomSeqCorrect = 0;
    for(int i=g_ButtonHistoryPointer-1,j=9;i>=g_ButtonHistoryPointer-10;i--,j--){
        if(i<0){
            printf("%d,",g_ButtonHistoryArray[i+10]);
            BottomSeqCorrect |= (specialCode[j] != g_ButtonHistoryArray[i+10]);
        }
        else{
            printf("%d,",g_ButtonHistoryArray[i]);
            BottomSeqCorrect |= (specialCode[j] != g_ButtonHistoryArray[i]);
        }
    }
    printf("\n");
    if(!BottomSeqCorrect){
        printf("[Secure]Success!!!!");
    }
}


void s2h( char* s , char* hex , uint32_t l ){

    char buf[4] = {0};

    for( int i = 0 ; i < l ; ++i ){
        snprintf( buf , sizeof(buf) , "%02x" , (uint8_t)s[i] );
        strncat( hex , buf , 2 );
    }
}


volatile uint32_t g_u32IsSHA_done = 0;

void CRPT_IRQHandler()
{
    if(SHA_GET_INT_FLAG(CRPT))
    {
        g_u32IsSHA_done = 1;
        SHA_CLR_INT_FLAG(CRPT);
    }
}


void sha256( char* p , void* e ){

    XSHA_Open(XCRPT, SHA_MODE_SHA256, SHA_IN_OUT_SWAP, 0);

    XSHA_SetDMATransfer(XCRPT, p, 0x18 + 0x20 ); // UID_hex + KEY_hex

    g_u32IsSHA_done = 0;
    XSHA_Start(XCRPT, CRYPTO_DMA_ONE_SHOT);
    while(g_u32IsSHA_done == 0) {}

    XSHA_Read(XCRPT, e);
}


uint32_t init_keys(){

    char p[0x20 + 0x20] = {0};
    uint32_t buf[4];

    FMC_Open();
    for( int i = 0 ; i < 3 ; ++i ){
        buf[i] = FMC_ReadUID(i);
    }
    printf( "\n\n" );
    FMC_Close();

    s2h( (char*)buf , uid , 12 );

    for( int i = 0 ; i < key_l ; i++  ){
        snprintf( p , sizeof(p) , "%s%s" , uid , key[i] );
        sha256( p , khash[i] );
    }

    /*for( int i = 0 ; i < key_l ; ++i ){
        printf( "key\t%d: %s\nhash: " , i , key[i] );
        dump( khash[i] , 0x20 );
        printf( "\n\n" );

    }*/

    printf( "[Crypto] init_key ... done\n" );

}

unsigned char hex_table( char c ){
    switch( c ){
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        default:
            return c - '0';
    }
}


uint32_t CheckHexSymbol( char c ){
    return !(c < '0' || ( c > '9' && c < 'A' ) || ( c > 'F' && c < 'a' ) || c > 'f');
}

unsigned char unpack_request( char *p , char *p_ns , uint32_t size ){
    unsigned char checksum = 0;
    for( uint32_t i = 0 ; i < size ; i += 2 ){
        if( !CheckHexSymbol( p_ns[i] ) || !CheckHexSymbol( p_ns[i+1] ) ){
            break;
        }
        p[i/2] = ( hex_table( p_ns[i] ) << 4 ) + hex_table( p_ns[i+1] );
        checksum ^= p[i/2];
    }
    return checksum;
}

__NONSECURE_ENTRY
uint32_t nsc_unlock( char* request ){

    // checksum xor each byte
    // [size 4byte][checksum 1byte][token ...]

    printf( "[Unlock] payload = %s\n" , request );

    int size;
    unsigned char checksum;
    char *token_NS = request + (5 * 2);
    char token[0x50] = {0};

    unpack_request( &size , request , 4 * 2 );
    unpack_request( &checksum , request + 8 , 1 * 2 );

    size = abs( size );
    //size = -size;

    if( size > 0x40 ){
        printf( "[Unlock] Invalid size.\n" );
        return -2;
    }

    if( unpack_request( token , token_NS , size ) != checksum ){
        printf( "[Unlock] checksum error.\n" );
        return -2;
    }

    for( int i = 0 ; i < key_l ; i++ ){
        if( !strncmp( token , khash[i] , 0x20 ) ){
            WriteRecord( i );
            printf( "[Unlock] Lock %d unlocked!\n" , i );
            return i;
        }
    }

    return -1;
}


__NONSECURE_ENTRY
void nsc_setinfo( char *fmt_ns ){

    char buf[0x200] = {0};
    char fmt[0x200] = {0};

    strncpy( fmt , fmt_ns , strlen( fmt_ns ) );

    FMC_Open();
    snprintf( buf , sizeof( buf ) , fmt , FMC_ReadUID(0) , FMC_ReadUID(1) , FMC_ReadUID(2) , g_u32SysTick / 1000 );
    FMC_Close();

    memset( fmt_ns , 0 , 0x200 );
    memcpy( fmt_ns , buf , strlen( buf ) );

}




/*----------------------------------------------------------------------------
  SysTick IRQ Handler
 *----------------------------------------------------------------------------*/

void SysTick_Handler(void)
{
    g_u32SysTick++;
    if (g_u32SysTick>= g_ClearButtonHistoryTimer && g_ClearButtonHistoryTimer > 0)
    {
        clearhistory();
        g_ClearButtonHistoryTimer = 0;
    }
}

unsigned long Delayms (unsigned long ms) {
        unsigned long systickcnt;
        systickcnt = g_u32SysTick;
        unsigned long count;
        while ((g_u32SysTick - systickcnt) < ms){
            //printf("%d",g_u32SysTick);
        }
        return count;
}


void SYS_Init(void);
void DEBUG_PORT_Init(void);
void Nonsecure_Init(void);

int main(void)
{
    SYS_UnlockReg();

    SYS_Init();

    // Crypto init
    NVIC_EnableIRQ(CRPT_IRQn);
    SHA_ENABLE_INT(CRPT);

    /* Init GPIO Port for LED control */
    //GPIO_SetMode(PA_NS, BIT0 | BIT2, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB_NS, BIT1 | BIT0, GPIO_MODE_OUTPUT);
    
    PB0_NS = 0;
    PB1_NS = 0;

    /* Init SysTick for 1ms */
    SysTick_Config(CyclesPerUs*1000);

    /* Init UART 0 for printf */
    DEBUG_PORT_Init();

    printf("[BOOT]Secure code is running ...\n");

    /* Init RGB LEDs */
    g_PixelBrightness = 200;
    PixelShow();
    XTRNG_RandomInit(&g_rng, XTRNG_PRNG | XTRNG_LIRC32K);


    FMC_Open();

    /* Read BS */
    printf("  Boot Mode ............................. ");
    if(FMC_GetBootSource() == 0)
        printf("[APROM]\n");
    else
    {
        printf("[LDROM]\n");
    }

    uint32_t u32Data = FMC_ReadCID();
    printf("  Company ID ............................ [0x%08x]\n", u32Data);

    u32Data = FMC_ReadPID();
    printf("  Product ID ............................ [0x%08x]\n", u32Data);

    for(int i = 0; i < 3; i++)
    {
        u32Data = FMC_ReadUID(i);
        printf("  Unique ID %d ........................... [0x%08x]\n", i, u32Data);
    }

    FMC_Close();


    //if the lock reg is 0xFFFF.... it means that the reg haven't been initialized (default flash data)
    //so clear all bit to 0
    uint32_t lockreg = nsc_GetLockReg();
    if(lockreg == 0xFFFFFFFF){
        nsc_ClearRecord();
    }
    else{
        printf("Current userConfig:[0x%08x]\n",lockreg);
    }

    nsc_GetLockReg();

    init_keys();
    init_matrix(led_position);

    /* Jump to Non-secure code */
    Nonsecure_Init();
    
    do
    {
        __WFI();
    }
    while(1);
    
}

void Nonsecure_Init(void)
{
    NonSecure_funcptr fp;

    /* SCB_NS.VTOR points to the Non-secure vector table base address. */
    SCB_NS->VTOR = NEXT_BOOT_BASE;

    /* 1st Entry in the vector table is the Non-secure Main Stack Pointer. */
    __TZ_set_MSP_NS(*((uint32_t *)SCB_NS->VTOR)); /* Set up MSP in Non-secure code */

    /* 2nd entry contains the address of the Reset_Handler (CMSIS-CORE) function */
    fp = ((NonSecure_funcptr)(*(((uint32_t *)SCB_NS->VTOR) + 1)));

    /* Clear the LSB of the function address to indicate the function-call
       will cause a state switch from Secure to Non-secure */
    fp = cmse_nsfptr_create(fp);

    /* Check if the Reset_Handler address is in Non-secure space */
    if(cmse_is_nsfptr(fp) && (((uint32_t)fp & 0xf0000000) == 0x10000000))
    {
        printf("[Secure]Execute non-secure code ...\n");
        fp(0); /* Non-secure function call */
    }
    else
    {
        /* Something went wrong */
        printf("[Secure]No code in non-secure region!\n");
        printf("[Secure]CPU will halted at non-secure state\n");

        /* Set nonsecure MSP in nonsecure region */
        __TZ_set_MSP_NS(NON_SECURE_SRAM_BASE + 512);

        /* Try to halted in non-secure state (SRAM) */
        M32(NON_SECURE_SRAM_BASE) = JUMP_HERE;
        fp = (NonSecure_funcptr)(NON_SECURE_SRAM_BASE + 1);
        fp(0);

        while(1);
    }
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Enable Internal RC 48MHz clock */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRC48EN_Msk);

    /* Waiting for Internal RC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRC48STB_Msk);

    /* Switch HCLK clock source to Internal RC and HCLK source divide 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC48, CLK_CLKDIV0_HCLK(1));

    /* Use HIRC48 as USB clock source */
    CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL0_USBSEL_HIRC48, CLK_CLKDIV0_USB(1));

    /* Enable PLL */
    CLK->PLLCTL = CLK_PLLCTL_128MHz_HIRC;

    /* Select USBD */
    SYS->USBPHY = (SYS->USBPHY & ~SYS_USBPHY_USBROLE_Msk) | SYS_USBPHY_OTGPHYEN_Msk | SYS_USBPHY_SBO_Msk;

    /* Enable IP clock */
    CLK_EnableModuleClock(USBD_MODULE);

    /* Enable UART clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

    /* Enable CRYPTO module clock */
    CLK_EnableModuleClock(CRPT_MODULE);

    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
    //SystemCoreClockUpdate();
    //PllClock        = 128000000;           // PLL
    SystemCoreClock = 48000000 / 1;        // HCLK
    CyclesPerUs     = 48000000 / 1000000;  // For SYS_SysTickDelayms()

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set multi-function pins for UART0 RXD and TXD */
    //SYS->GPB_MFPH = (SYS->GPB_MFPH & (~(UART0_RXD_PB12_Msk | UART0_TXD_PB13_Msk))) | UART0_RXD_PB12 | UART0_TXD_PB13;

    SYS->GPF_MFPL = SYS_GPF_MFPL_PF3MFP_UART0_TXD | SYS_GPF_MFPL_PF2MFP_UART0_RXD | SYS_GPF_MFPL_PF1MFP_ICE_CLK | SYS_GPF_MFPL_PF0MFP_ICE_DAT;


    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* USBD multi-function pins for VBUS, D+, D-, and ID pins */
    SYS->GPA_MFPH &= ~(SYS_GPA_MFPH_PA12MFP_Msk | SYS_GPA_MFPH_PA13MFP_Msk | SYS_GPA_MFPH_PA14MFP_Msk | SYS_GPA_MFPH_PA15MFP_Msk);
    SYS->GPA_MFPH |= (SYS_GPA_MFPH_PA12MFP_USB_VBUS | SYS_GPA_MFPH_PA13MFP_USB_D_N | SYS_GPA_MFPH_PA14MFP_USB_D_P | SYS_GPA_MFPH_PA15MFP_USB_OTG_ID);


    /* Enable TIMER module clock */
    CLK_EnableModuleClock(TMR2_MODULE);
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HIRC, 0);
    TIMER_Open(TIMER2_NS, TIMER_PERIODIC_MODE, 20);

}

void DEBUG_PORT_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/

    /* Configure UART and set UART Baudrate */
    DEBUG_PORT->BAUD = UART_BAUD_MODE2 | UART_BAUD_MODE2_DIVIDER(__HIRC, 115200);
    DEBUG_PORT->LINE = UART_WORD_LEN_8 | UART_PARITY_NONE | UART_STOP_BIT_1;
}
