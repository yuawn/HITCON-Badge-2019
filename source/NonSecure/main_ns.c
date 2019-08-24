#include <arm_cmse.h>
#include "NuMicro.h"                    /* Device header */
#include "cssd_lib.h"                   /* Collaborative Secure Software Development Library header */


#include <string.h>
#include "vcom_serial.h"
#include "usbprintf.h"
#include "Cmd.h"
#include "usbcom.h"
#include "utility.h"

#define BUTTON_NONE 0
#define BUTTON_UP 1
#define BUTTON_DOWN 2
#define BUTTON_LEFT 3
#define BUTTON_RIGHT 4
#define BUTTON_A 5
#define BUTTON_B 6

#define BUTTON_UP_IO PF4_NS
#define BUTTON_DOWN_IO PA1_NS
#define BUTTON_LEFT_IO PF5_NS
#define BUTTON_RIGHT_IO PA0_NS
#define BUTTON_A_IO PB2_NS
#define BUTTON_B_IO PB3_NS

int g_page = 0 , ready = 0;
char user[0x100] = "sheep";
char IO_buf[0x400] = {0};

volatile uint32_t g_ButtonPressed = 0;
int g_DisplayingPattern = 0;
uint8_t brightness = 100;
//Moving Array
const int UP[24] = {2,-1,4,1,3,5,10,6,8,11,13,14,12,16,19,18,18,21,20,-1,22,23,24,1};
const int RIGHT[24] = {2,-1,-1,-1,-1,8,6,-1,-1,6,6,-1,12,17,13,17,5,-1,18,23,22,4,22,1};
const int LEFT[24] = {24,1,22,24,17,10,10,6,7,-1,-1,13,15,-1,-1,19,16,19,-1,-1,20,23,20,23};
const int DOWN[24] = {4,1,5,3,6,8,-1,9,-1,7,10,11,11,12,-1,14,6,17,15,19,18,21,22,22};

uint16_t g_CurrentPixelPointer = 0;

int g_PixelDegree[24] = {90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90,90};

//SysTick
volatile uint32_t g_u32SysTick;
//volatile int degree = 0;
uint32_t displaypattern_timer = 0;
/*----------------------------------------------------------------------------
  NonSecure Functions from NonSecure Region
 *----------------------------------------------------------------------------*/
int32_t LED_On(void)
{
    if(g_ButtonPressed == BUTTON_NONE || g_DisplayingPattern == 0){
        nsc_SetPixelColor(g_CurrentPixelPointer, 0);
        nsc_PixelShow();
    }
    return 0;
}

int32_t LED_Off(void)
{
    if(g_ButtonPressed == BUTTON_NONE|| g_DisplayingPattern == 0){
        nsc_SetPixelColor(g_CurrentPixelPointer, HSV_Color_Calc(g_PixelDegree[g_CurrentPixelPointer]));
        nsc_PixelShow();
    }

    return 1;
}

/*----------------------------------------------------------------------------
  SysTick IRQ Handler
 *----------------------------------------------------------------------------*/


void SysTick_Handler(void)
{

    VCOM_TransferData();

    if( g_page != 3 )
        cmdPoll();

    if( ready && USB_available()>0 ){
        printfUSB( "%s" , IO_buf );
        memset( IO_buf , 0 , sizeof( IO_buf ) );
        ready = 0;
    }

    ++g_u32SysTick;

}

unsigned long Delayms (unsigned long ms) {
        unsigned long systickcnt;
        systickcnt = g_u32SysTick;
        unsigned long count;
        while( (g_u32SysTick - systickcnt) < ms ){}
        return count;
}


/*----------------------------------------------------------------------------
  Button IRQ Handler
 *----------------------------------------------------------------------------*/
void GPA_IRQHandler(void)
{
    /* To check if PB.2 interrupt occurred */
    if(GPIO_GET_INT_FLAG(PA_NS, BIT0))
    {
        GPIO_CLR_INT_FLAG(PA_NS, BIT0);
        printf("PA.0 INT occurred RIGHT.\n"); //RIGHT
        g_ButtonPressed = BUTTON_RIGHT;
    }
    else if(GPIO_GET_INT_FLAG(PA_NS, BIT1))
    {
        GPIO_CLR_INT_FLAG(PA_NS, BIT1);
        printf("PA.1 INT occurred DOWN.\n"); //DOWN
        g_ButtonPressed = BUTTON_DOWN;
        displaypattern_timer = g_u32SysTick;
    }
    else
    {
        /* Un-expected interrupt. Just clear all PA interrupts */
        //PA_NS->INTSRC = PA_NS->INTSRC;
        printf("Un-expected interrupts.\n");
    }
}
void GPB_IRQHandler(void)
{
    /* To check if PB.2 interrupt occurred */
    if(GPIO_GET_INT_FLAG(PB_NS, BIT2))
    {
        GPIO_CLR_INT_FLAG(PB_NS, BIT2);
        printf("PB.2 INT occurred A.\n");    //A
        g_ButtonPressed = BUTTON_A;
    }
    else if(GPIO_GET_INT_FLAG(PB_NS, BIT3))
    {
        GPIO_CLR_INT_FLAG(PB_NS, BIT3);
        printf("PB.3 INT occurred B.\n");    //B
        g_ButtonPressed = BUTTON_B;
    }
    else
    {
        /* Un-expected interrupt. Just clear all PB interrupts */
        //PB_NS->INTSRC = PB_NS->INTSRC;
        printf("Un-expected interrupts.\n");
    }
}
void GPF_IRQHandler(void)
{
    /* To check if PF.2 interrupt occurred */
    if(GPIO_GET_INT_FLAG(PF_NS, BIT4))
    {
        GPIO_CLR_INT_FLAG(PF_NS, BIT4);
        printf("PF.4 INT occurred UP.\n"); //UP
        g_ButtonPressed = BUTTON_UP;
        displaypattern_timer = g_u32SysTick;
    }
    else if(GPIO_GET_INT_FLAG(PF_NS, BIT5))
    {
        GPIO_CLR_INT_FLAG(PF_NS, BIT5);
        printf("PF.5 INT occurred Left.\n"); //Left
        g_ButtonPressed = BUTTON_LEFT;
    }
    else
    {
        /* Un-expected interrupt. Just clear all PF interrupts */
        //PF_NS->INTSRC = PF_NS->INTSRC;
        printf("Un-expected interrupts.\n");
    }
}

/*----------------------------------------------------------------------------
  USB Console CMD Handler
 *----------------------------------------------------------------------------*/
void hello(int arg_cnt, char **args)
{
    printfUSB("Hello world.");
}

void setcolor(int arg_cnt, char **args)
{
    if (arg_cnt != 5){
        printfUSB("Error\n");
        return;
    }
    uint16_t led;
    uint32_t r,g,b;
    sscanf(args[1], "%hd", &led);
    sscanf(args[2], "%d", &r);
    sscanf(args[3], "%d", &g);
    sscanf(args[4], "%d", &b);
    r = r % 256;
    g = g % 256;
    b = b % 256;
    //printfUSB("%lld,%lld,%lld,%lld",led,r,g,b);
    //printfUSB("%d\n",led);
    //fctprintf(&USB_output, NULL, "%d,%d,%d,%d",led,r,g,b);
    nsc_SetPixelColorRGB(led,r,g,b);
    nsc_PixelShow();

}
void setcolor_group(int arg_cnt, char **args)
{
    if (arg_cnt != 25){
        printfUSB("Error\n");
        return;
    }
    uint32_t c;
    for(int i=0;i<24;i++){
        sscanf(args[1+i], "%d", &c);
        nsc_SetPixelColor(i,c);
    }
    nsc_PixelShow();

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

void setname( int argc, char **argv ){

    // update_username()
    // blacklist
    // [a-zA-Z]

    if( argc == 1 ){
        printfUSB( "usage: setname [name]\r\n       setname -h [hex string]" );
        return;
    }

    if( !strncmp( argv[1] , "-h" , 2 ) ){
        if( argc == 2 ){
            printfUSB( "usage: setname [name]\r\n       setname -h [hex string]" );
            return;
        }

        if( strlen( argv[2] ) & 1 || strlen( argv[2] ) > 0x200 - 1 ){
            printfUSB( "usage: invalid hex string\r\n" );
            return;
        }

        for( int i = 0 ; i < strlen( argv[2] ) ; ++i ){
            unsigned char c = argv[2][i];
            if( c < '0' || ( c > '9' && c < 'A' ) || ( c > 'F' && c < 'a' ) || c > 'f' ){
                printfUSB( "setname: invalid hex string" );
                return;
            }
        }

        memset( user , 0 , sizeof( user ) );
        for( int i = 0 ; i < strlen( argv[2] ) ; i += 2 ){
            user[i/2] = (hex_table( argv[2][i] ) << 4) + hex_table( argv[2][i+1] );
        }

        //update_username( name );
    }
    else{

        //update_username( argv[1] );
        memset( user , 0 , sizeof( user ) );
        strncpy( user , argv[1] , strlen( argv[1] ) >= 0xff ? 0xff : strlen( argv[1] ) );
    }

    printfUSB( "Done" );
}


void info(){

    char msg[0x200] = {0};
    char welcome[0x180] = {0};
    char *fmt = "+-----------------------------+\r\n"
                "|MCU: M2351ZIAAE              |\r\n"
                "+-----------------------------+\r\n"
                "|UID: %08x%08x%08x|\r\n"
                "+-----------------------------+\r\n"
                "|Uptime: %-18d(s)|\r\n"
                "+-----------------------------+\r\n";


    snprintf( welcome , sizeof( welcome ) , "Welcome %s" , user );
    snprintf( msg , sizeof( msg ) , "%s%s" , fmt , welcome );

    nsc_setinfo( msg );

    printfUSB( "%s" , msg );

}

void clear( int argc, char **argv ){
    if( argc == 1 ){
        printfUSB( "[Warning] This command will erase ALL the led records, usage: clear YES" );
        return;
    }

    if( !strncmp( argv[1] , "YES" , 3 ) ){
        nsc_ClearRecord();
        printfUSB( "clear: done." );
    }
}

void show(){
    uint32_t lock = nsc_GetLockReg();

    for( int i = 0 ; i < 8 ; ++i ){
        printfUSB( "Pattern %d: %s\r\n" , i , nsc_check_pattern( i ) ? "UnLock" : "Lock" );
        for( int j = i * 3 ; j < i * 3 + 3 ; ++j ){
            printfUSB( "  led %02d: %s\r\n" , j , ( lock >> j ) & 1 ? "UnLock" : "Lock" );
        }
    }

    printfUSB( "\r\n" );

    printfUSB( "Badge challenge:\r\n" );

    printfUSB( "[Stage 1] Snake pattern: %s\r\n" , ( lock >> 30 ) & 1 ? "UnLock" : "Lock" );
    printfUSB( "[Stage 2] Pwned NS pattern: %s\r\n" , ( lock >> 31 ) & 1 ? "UnLock" : "Lock" );
    printfUSB( "[Stage 3] Pwned the whole badge pattern: %s\r\n" , nsc_isPwned() ? "UnLock" : "Lock" );

}


void unlock( int argc, char **argv ){

    if( argc == 1 ){
        printfUSB( "unlock: unlock [request]" );
        return;
    }

    char request[0x280];
    strncpy( request , argv[1] , 0x280 - 1 );

    int r = nsc_unlock( request );

    if( r < 0 ){
        printfUSB( "unlock: Invalid request." );
    }
    else{
        printfUSB( "unlock: Lock %d unlocked!" , r );
    }
}

void alias( int argc, char **argv ){

    if( argc < 3 ){
        printfUSB( "alias: usage: alias [name] [value]" );
        return;
    }

    if( !cmd_alias( argv[2] , argv[1] ) ){
        printfUSB( "alias: No such command: %s" , argv[2] );
    }
}

void ls( int argc, char **argv ){
    printfUSB( "flag.txt\r\n" );
    printfUSB( "README.md" );
}

void id( int argc, char **argv ){
    if( !strncmp( user , "root" , 4 ) ){
        printfUSB( "uid=0(root) gid=0(root) groups=0(root)" );
    }
    else{
        printfUSB( "uid=1000(%s) gid=1000(%s) groups=1000(%s)" , user , user , user );
    }
}

void echo( int argc, char **argv ){
    if( argc == 2 ){
        printfUSB( "%s" , argv[1] );
    }
}

void ping(){
    printfUSB( "pong" );
}

void cat(){
    printfUSB( "Meow~" );
}

void whoami(){
    printfUSB( "%s" , user );
}

void uname(){
    printfUSB( "Hitcon-badge 1.1 ARMv8" );
}

void yuawn(){
    char *img7 =
                "        __             \r\n"
                "        \\/.--,         \r\n"
                "        //_.'          \r\n"
                "   .---//------..      \r\n"
                "  / . . . . . . . \\    \r\n"
                " / . ./\\. . ./\\ .. \\   \r\n"
                " |. ./  \\. ./  \\ . |  \r\n"
                " \\ . . . . . . . ..|  \r\n"
                " \\. . . . . . . . ./   \r\n"
                "  \\ . . \\___/. . ./    \r\n"
                "   '-.__.__.__._-'     \r\n";

    printfUSB( "%s" , img7 );
}

void angelboy(){
    char *img6 =
                "        __            \r\n"
                "        \\/.--,        \r\n"
                "        //_.'         \r\n"
                "   .-\"\"-/\"\"----..     \r\n"
                "  / . . . . . . . \\   \r\n"
                " / . . \\ . . / . . \\  \r\n"
                " |. ____\\ . /____. |  \r\n"
                " \\ . . . . . . . . |  \r\n"
                " \\. . . . . . . . ./  \r\n"
                "  \\ . . . -- . . ./   \r\n"
                "   '-.__.__.__._-'    \r\n";

    printfUSB( "%s" , img6 );
}



void help(){
    printfUSB( "show\r\n" );
    printfUSB( "info\r\n" );
    printfUSB( "unlock\r\n" );
    printfUSB( "setname\r\n" );
    printfUSB( "clear\r\n" );
    printfUSB( "hello\r\n" );
    printfUSB( "angelboy\r\n" );
    printfUSB( "yuawn\r\n" );
    printfUSB( "ping\r\n" );
    printfUSB( "ls\r\n" );
    printfUSB( "id\r\n" );
    printfUSB( "cat\r\n" );
    printfUSB( "echo\r\n" );
    printfUSB( "alias\r\n" );
    printfUSB( "whoami\r\n" );
    printfUSB( "help" );
}


/*----------------------------------------------------------------------------
  DIsplay Function
 *----------------------------------------------------------------------------*/

void displaylevel(){
    nsc_ClearPixel();

    uint32_t currentLock = nsc_GetLockReg();
    //printf("LockReg = [0x%08x]\n",currentLock);

    int j = 0;
    while(1){
        uint32_t color = HSV_Color_Calc((j++) % 360);
        for(int i=0; i< 24; i++) {
            if(currentLock>>i & 1 == 1){
                nsc_SetPixelColor(i, color);
            }
            else{
                nsc_SetPixelColor(i, 0x707070);
            }
        }
        nsc_PixelShow();

        Delayms(10);


        if(BUTTON_UP_IO == 0){
            if(BUTTON_B_IO == 0){
                if(brightness !=255){
                    brightness++;
                }
                printf("Brightness:%d\n",brightness);
                nsc_SetBrigntness(brightness);
                //nsc_PixelShow();
                //Delayms(5);
            }
            else if(BUTTON_A_IO == 0){
                if(brightness > 1){
                    brightness --;
                }
                printf("Brightness:%d\n",brightness);
                nsc_SetBrigntness(brightness);
                //nsc_PixelShow();
                //Delayms(5);
            }
        }

        if(g_ButtonPressed == BUTTON_LEFT || g_ButtonPressed == BUTTON_RIGHT){
            break;
        }
        g_ButtonPressed = BUTTON_NONE;
    }
    printf("Return main menu\n");
}

void displayPatternNo(uint8_t number){
    nsc_ClearPixel();
    for(int i=0;i<5;i++){
        if(i%2)
            nsc_SetPixelColor(number,0);
        else
            nsc_SetPixelColor(number,0xFFFFFF);

        nsc_PixelShow();
        //if(g_ButtonPressed == BUTTON_UP || g_ButtonPressed == BUTTON_DOWN) return;
        Delayms(300);
    }
}



void displaypattern(){
    int pattern = 0;
    int controlA = 0;
    int controlB = 0;

    printf("displaypattern : %d\n",pattern);
    displayPatternNo(pattern);
    while(1){

        nsc_display(pattern,controlA,controlB);


        if(BUTTON_UP_IO == 0){
            if(BUTTON_B_IO == 0){
                if(brightness !=255){
                    brightness++;
                }
                printf("Brightness:%d\n",brightness);
                nsc_SetBrigntness(brightness);
                g_ButtonPressed = BUTTON_NONE;
            }
            else if(BUTTON_A_IO == 0){
                if(brightness > 1){
                    brightness --;
                }
                printf("Brightness:%d\n",brightness);
                nsc_SetBrigntness(brightness);
                g_ButtonPressed = BUTTON_NONE;
            }
        }
        else{
            if(BUTTON_B_IO == 0){
                controlA++;
                printf("controlA:%d\n",controlA);
                g_ButtonPressed = BUTTON_NONE;
            }
            else if(BUTTON_A_IO == 0){
                controlA --;
                printf("controlA:%d\n",controlA);
                g_ButtonPressed = BUTTON_NONE;
            }

        }
        //printf("displaypattern_timer:%d =>%d  %d\n",displaypattern_timer,(g_u32SysTick-displaypattern_timer),g_ButtonPressed);
        if(g_ButtonPressed == BUTTON_UP && (g_u32SysTick-displaypattern_timer)>500){
            pattern ++;
            controlA = 0;
            printf("displaypattern : %d\n",pattern);
            displayPatternNo(pattern);
            if(pattern >24){
                pattern =24;
            }
            g_ButtonPressed = BUTTON_NONE;
        }
        else if(g_ButtonPressed == BUTTON_DOWN && (g_u32SysTick-displaypattern_timer)>500){
            pattern --;
            controlA = 0;
            printf("displaypattern : %d\n",pattern);

            if(pattern <0){
                pattern =0;
            }
            else{
                displayPatternNo(pattern);
            }
            g_ButtonPressed = BUTTON_NONE;
        }

        if(g_ButtonPressed == BUTTON_LEFT || g_ButtonPressed == BUTTON_RIGHT){
            break;
        }

    }
}

void paint(){

    nsc_ClearPixel();
    for(int i=0;i<24;i++){
        nsc_SetPixelColorRGB(i,100,100,100);
    }
    nsc_PixelShow();


    while(1){
        nsc_SetPixelColor(g_CurrentPixelPointer, HSV_Color_Calc(g_PixelDegree[g_CurrentPixelPointer]));
        nsc_PixelShow();

        while(g_ButtonPressed == BUTTON_NONE || g_ButtonPressed == BUTTON_A || g_ButtonPressed == BUTTON_B){
            if(BUTTON_A_IO == 0){
                g_PixelDegree[g_CurrentPixelPointer] ++;
                g_PixelDegree[g_CurrentPixelPointer] = g_PixelDegree[g_CurrentPixelPointer]%360;

                printf("Degree:%d\n",g_PixelDegree[g_CurrentPixelPointer]);
                nsc_SetPixelColor(g_CurrentPixelPointer, HSV_Color_Calc(g_PixelDegree[g_CurrentPixelPointer]));
                nsc_PixelShow();
                Delayms(10);
            }
            else if(BUTTON_B_IO == 0){
                g_PixelDegree[g_CurrentPixelPointer]--;
                if(g_PixelDegree[g_CurrentPixelPointer]<0) g_PixelDegree[g_CurrentPixelPointer]+=360;

                printf("degree:%d\n",g_PixelDegree[g_CurrentPixelPointer]);
                nsc_SetPixelColor(g_CurrentPixelPointer, HSV_Color_Calc(g_PixelDegree[g_CurrentPixelPointer]));
                nsc_PixelShow();
                Delayms(10);
            }
            if(g_ButtonPressed == BUTTON_A || g_ButtonPressed == BUTTON_B){
                //nsc_CheckButtonSequence(g_ButtonPressed);
                g_ButtonPressed = 0;
            }
            if(BUTTON_A_IO == 0 && BUTTON_B_IO == 0){
                Delayms(1000);
                if(g_ButtonPressed == BUTTON_RIGHT || g_ButtonPressed == BUTTON_LEFT){
                    printf("exit\n");
                    return;
                }
            }
        }

        //nsc_CheckButtonSequence(g_ButtonPressed);

        //g_u32SysTick = 800;
        int next = -1;
        if(g_ButtonPressed == BUTTON_UP){
            next = UP[g_CurrentPixelPointer];
        }
        else if(g_ButtonPressed == BUTTON_DOWN){
            next = DOWN[g_CurrentPixelPointer];
        }
        else if(g_ButtonPressed == BUTTON_LEFT){
            next = LEFT[g_CurrentPixelPointer];
        }
        else if(g_ButtonPressed == BUTTON_RIGHT){
            next = RIGHT[g_CurrentPixelPointer];
        }

        g_ButtonPressed = 0;


        if(next != -1){
            printf("NEXT:%d\n",next-1);
            nsc_SetPixelColor(g_CurrentPixelPointer, HSV_Color_Calc(g_PixelDegree[g_CurrentPixelPointer]));
            g_CurrentPixelPointer = next-1;
        }
    }

}

typedef struct _node{
    unsigned char x;
    unsigned char y;
} node;


uint32_t map[50][50] = {0};
uint32_t len;
node snake[1000];
node food;
int paused = 0;
int w = 32 , h = 18;

void flush_USB(){
    ready = 1;
    while( ready ){}
}

void print_map(){

    int l = 0;

    memcpy( IO_buf , "\x1B[2J" , 4 );
    flush_USB();

    for( int y = 0 ; y < h ; ++y ){
        if( paused && y == 8 ){
            IO_buf[l++] = '#';
            for( int i = 0 ; i < w/2 - 1 - 3 ; ++i ) IO_buf[l++] = ' ';
            strcpy( &IO_buf[l] , "PAUSED" );
            l += 6;
            for( int i = 0 ; i < w/2 - 1 - 3 ; ++i ) IO_buf[l++] = ' ';
            IO_buf[l++] = '#';
            IO_buf[l++] = '\r';
            IO_buf[l++] = '\n';
            continue;
        }
        for( int x = 0 ; x < w ; ++x ){
            if( x == 0 || y == 0 || x == w - 1 || y == h - 1 ){
                IO_buf[l++] = '#';
            }
            else if( map[y][x] == 1 ){
                IO_buf[l++] = '@';
            }
            else if( map[y][x] == 2 ){
                IO_buf[l++] = 'o';
            }
            else{
                IO_buf[l++] = ' ';
            }
        }
        IO_buf[l++] = '\r';
        IO_buf[l++] = '\n';
    }

    flush_USB();
}



uint32_t circleRadis = 0;
int8_t circleStartPoint_x = -100;
int8_t circleStartPoint_y = -100;
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

double interpolate( double val, double y0, double x0, double y1, double x1 ) {
    return (val-x0)*(y1-y0)/(x1-x0) + y0;
}

double base( double val ) {
    if ( val <= -0.75 ) return 0;
    else if ( val <= -0.25 ) return interpolate( val, 0.0, -0.75, 1.0, -0.25 );
    else if ( val <= 0.25 ) return 1.0;
    else if ( val <= 0.75 ) return interpolate( val, 1.0, 0.25, 0.0, 0.75 );
    else return 0.0;
}


// input -1 ~ 1
double red( double gray ) {
    return base( gray - 0.5 );
}
double green( double gray ) {
    return base( gray );
}
double blue( double gray ) {
    return base( gray + 0.5 );
}

#define MIDLEN_COLORBAR 20

void _snake_newfood_animanteLED(){

        nsc_ClearPixel();


        //float randomdistance_x = circleStartPoint_x[circle_count%2] - randomSelectx;
        //float randomdistance_y = circleStartPoint_y[circle_count%2] - randomSelecty;
        //float randomdistance = sqrt(randomdistance_x*randomdistance_x + randomdistance_y*randomdistance_y);

        uint32_t timeLength = (g_u32SysTick - circleRadis)/5;


        float pixel_buffer[24] = {0};


        for(int i=0;i<24;i++){
            float distance_x = led_position[i][0] - circleStartPoint_x;
            float distance_y = led_position[i][1] - circleStartPoint_y;

            float distance = sqrt(distance_x*distance_x + distance_y*distance_y);

            if(distance < timeLength){
                float bright_ratio = exp(-1 *  ((double)timeLength - (double)distance) / 40.0);

                pixel_buffer[i] = bright_ratio;//(uint32_t)r<<16 | (uint32_t)g<<16 | (uint32_t)b;

            }
        }



        float ratio = (float)len/MIDLEN_COLORBAR -1;
        if(ratio>1) ratio = 1;
        uint8_t r = 255.0*red(ratio);
        uint8_t g = 255.0*green(ratio);
        uint8_t b = 255.0*blue(ratio);

        for(int i=0;i<24;i++){
            //printf("%f\n",pixel_buffer[i]);
            if(pixel_buffer[i]>1) pixel_buffer[i] = 1;
            //if(pixel_buffer[i] < 0.2) continue;

            nsc_SetPixelColorRGB(i,r*pixel_buffer[i],g*pixel_buffer[i],b*pixel_buffer[i]);
        }
        nsc_PixelShow();

}

void _snake(){

    printfUSB( "\x1B[2JPress any key to start! Use badge's button to control.\r\n" );

    paused = 0;
    len = 1;
    snake[0].x = 7;
    snake[0].y = 7;

    food.x = 16;
    food.y = 7;

    int dir = BUTTON_RIGHT;
    int score = 0;

    uint32_t updateduration = 500;
    uint32_t now = g_u32SysTick;

    nsc_ClearPixel();
    nsc_PixelShow();

    while(1){

        if( g_ButtonPressed ){
            if(g_ButtonPressed == BUTTON_RIGHT || g_ButtonPressed == BUTTON_LEFT || g_ButtonPressed == BUTTON_UP || g_ButtonPressed == BUTTON_DOWN){
                if((g_ButtonPressed == BUTTON_LEFT && dir != BUTTON_RIGHT)
                 || (g_ButtonPressed == BUTTON_RIGHT && dir != BUTTON_LEFT)
                 || (g_ButtonPressed == BUTTON_UP && dir != BUTTON_DOWN)
                 || (g_ButtonPressed == BUTTON_DOWN && dir != BUTTON_UP))
                dir = g_ButtonPressed;
            }
        }
        if(BUTTON_A_IO == 0 && BUTTON_B_IO == 0){
            paused = !paused;
            if( paused ){
                print_map();
                snprintf( IO_buf , sizeof( IO_buf ) , "[Score] %d pt\r\n" , len - 1 );
                flush_USB();
                //printfUSB( "Pulsed.\r\n" );
                Delayms(2000);
            }
            Delayms(500);

        }
        if( paused ){
            if( g_ButtonPressed == BUTTON_LEFT || g_ButtonPressed == BUTTON_RIGHT ){
                //USB_flush();
                break;
            }
        }
        _snake_newfood_animanteLED();

        //500 => 0
        //100 => more than 20
        if( len <= 25 ){
            updateduration = 500 - len * 16;
        }
        else{
            updateduration = 100;
        }

        if( g_u32SysTick - now > updateduration && !paused ){
            now = g_u32SysTick;

            map[ snake[len-1].y ][ snake[len-1].x ] = 0;

            for( int i = len ; i > 0 ; --i ){
                snake[i].x = snake[i-1].x;
                snake[i].y = snake[i-1].y;
            }

            switch( dir ){
                case BUTTON_UP:
                    --snake[0].y;
                    break;
                case BUTTON_DOWN:
                    ++snake[0].y;
                    break;
                case BUTTON_RIGHT:
                    ++snake[0].x;
                    break;
                case BUTTON_LEFT:
                    --snake[0].x;
                    break;
                default:
                    break;
            }

            if( map[ snake[0].y ][ snake[0].x ] == 1 || !snake[0].x || snake[0].x == w - 1 || !snake[0].y || snake[0].y == h - 1 ){

                g_ButtonPressed = 0;
                break;
            }

            int new_food = 0;
            if( snake[0].y == food.y && snake[0].x == food.x ){
                /*
                if(len/10<24){
                    for(int i=0;i<len/10;i++){
                        nsc_SetPixelColorRGB(i,255,255,255);
                    }

                }
                float ratio_led = (len%10)/10.0;
                nsc_SetPixelColorRGB(len/10,255*ratio_led,255*ratio_led,255*ratio_led);

                nsc_PixelShow();
                */
                circleRadis = g_u32SysTick;
                circleStartPoint_x = nsc_randomInt()%201 - 100;
                circleStartPoint_y = nsc_randomInt()%201 - 100;
                ++len;
                ++score;
                new_food = 1;


            }

            memset( map , 0 , sizeof( map ) );

            for( int i = 0 ; i < len ; ++i ){
                map[ snake[i].y ][ snake[i].x ] = 1;
            }

            map[ snake[0].y ][ snake[0].x ] = 1;


            if( new_food ){
                while( map[ food.y ][ food.x ] != 0 ){
                    food.y = nsc_randomInt() % (h-2) + 1;
                    food.x = nsc_randomInt() % (w-2) + 1;
                }
            }

            map[ food.y ][ food.x ] = 2;

            print_map();

            snprintf( IO_buf , sizeof( IO_buf ) , "[Score] %d pt\r\n" , len - 1 );
            flush_USB();
        }
    }

    if( score < 50 ){
        strcpy( IO_buf , "\x1B[2JGame Over.\r\n" );
        flush_USB();
    }

    else if( score >= 50 ){
        strcpy( IO_buf , "\x1B[2JCongratulations! Snake pattern unlocked!\r\n" );
        flush_USB();
        nsc_WinSnake();
    }

    if( score == 2147483647 ){
        printfUSB( "\x1B[2JHow did you do that :p\r\n" );
        nsc_Pwned_NS();
    }

    USB_flush();
    Delayms(3000);

    if( g_ButtonPressed == BUTTON_LEFT ){
        cmd_display();
    }
}


/*----------------------------------------------------------------------------
  Main function
 *----------------------------------------------------------------------------*/
int main(void)
{
    printf("\nNonsecure code is running ...\n");

    /* Init GPIO Port for Button control */
    init_GPIO();

    /* Init SysTick to 1 ms */
    SysTick_Config(CyclesPerUs*1000);

    /* Init USB Comport */
    init_USB();


    nsc_SetBrigntness(brightness);
    nsc_PixelShow();

    /* Init cmdline */
    cmdInit();

    cmdAdd("hello", hello);
    cmdAdd("set", setcolor);
    cmdAdd("setgroup", setcolor_group);

    cmdAdd( "show", show );
    cmdAdd( "unlock", unlock );
    cmdAdd( "clear" , clear );
    cmdAdd( "info" , info );
    cmdAdd( "setname" , setname );
    cmdAdd( "angelboy" , angelboy );
    cmdAdd( "yuawn" , yuawn );

    cmdAdd( "ls" , ls );
    cmdAdd( "id" , id );
    cmdAdd( "cat" , cat );
    cmdAdd( "echo" , echo );
    cmdAdd( "ping" , ping );
    cmdAdd( "alias" , alias );
    //cmdAdd( "uname" , uname );
    cmdAdd( "whoami" , whoami );
    cmdAdd( "help" ,help );


    // Left, Right => Switch page
    // UP, Down => select pattern
    // Always on => Change Lux (A+ Up or Down)

    while(1){

        printf("Main menu : %d\n",g_page);
        switch(g_page){
        case 0:
            displaylevel();
            break;
        case 1:
            displaypattern();
            break;
        case 2:
            paint();
            break;
        case 3:
            _snake();
            break;
        default:
            break;
        }
        if(g_ButtonPressed == BUTTON_RIGHT){
            g_page ++;
            if(g_page > 3){
                g_page = 0;
            }
        }
        else if(g_ButtonPressed == BUTTON_LEFT){
            g_page --;
            if(g_page < 0){
                g_page =0;
            }
        }
        g_ButtonPressed = BUTTON_NONE;

    }

}


void init_GPIO(){
    GPIO_SetMode(PA_NS, BIT2 , GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB_NS, BIT1 | BIT0, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PC_NS, BIT1 | BIT0, GPIO_MODE_OUTPUT);

    GPIO_SetMode(PA_NS, BIT1 | BIT0, GPIO_MODE_INPUT);
    GPIO_SetMode(PB_NS, BIT3 | BIT2, GPIO_MODE_INPUT);
    GPIO_SetMode(PF_NS, BIT4 | BIT5, GPIO_MODE_INPUT);

    GPIO_SetPullCtl(PA_NS, BIT1 | BIT0, GPIO_PUSEL_PULL_UP);
    GPIO_SetPullCtl(PB_NS, BIT3 | BIT2, GPIO_PUSEL_PULL_UP);
    GPIO_SetPullCtl(PF_NS, BIT4 | BIT5, GPIO_PUSEL_PULL_UP);

    GPIO_EnableInt(PA_NS, 0, GPIO_INT_FALLING);
    GPIO_EnableInt(PA_NS, 1, GPIO_INT_FALLING);
    GPIO_EnableInt(PB_NS, 2, GPIO_INT_FALLING);
    GPIO_EnableInt(PB_NS, 3, GPIO_INT_FALLING);
    GPIO_EnableInt(PF_NS, 4, GPIO_INT_FALLING);
    GPIO_EnableInt(PF_NS, 5, GPIO_INT_FALLING);

    NVIC_EnableIRQ(GPA_IRQn);
    NVIC_EnableIRQ(GPB_IRQn);
    NVIC_EnableIRQ(GPF_IRQn);


    GPIO_SET_DEBOUNCE_TIME(PA_NS, GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_32);
    GPIO_SET_DEBOUNCE_TIME(PB_NS, GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_32);
    GPIO_SET_DEBOUNCE_TIME(PF_NS, GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_32);

    GPIO_ENABLE_DEBOUNCE(PA_NS, BIT1 | BIT0);
    GPIO_ENABLE_DEBOUNCE(PB_NS, BIT3 | BIT2);
    GPIO_ENABLE_DEBOUNCE(PF_NS, BIT4 | BIT5);


    printf("GPIO Setup completed\n");
}

void init_USB(){
    USBD_Open(&gsInfo, VCOM_ClassRequest, NULL);
    /* Endpoint configuration */
    VCOM_Init();
    USBD_Start();
    NVIC_EnableIRQ(USBD_IRQn);
    USBD_NS->INTSTS = USBD_INTSTS_SOFIF_Msk;
    printf("USB opened\n");
}
