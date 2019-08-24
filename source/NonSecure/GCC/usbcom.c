#include <stdio.h>
#include "NuMicro.h"
#include "vcom_serial.h"
#include "usbcom.h"
#define CRYSTAL_LESS        1
#define TRIM_INIT           (SYS_BASE+0x10C)
extern volatile uint32_t u32Ticks;
#define CLK_PLLCTL_144MHz_HXT   (CLK_PLLCTL_PLLSRC_HXT  | CLK_PLLCTL_NR(2) | CLK_PLLCTL_NF( 12) | CLK_PLLCTL_NO_1)

/*--------------------------------------------------------------------------*/
STR_VCOM_LINE_CODING g_LineCoding = {115200, 0, 0, 8};   /* Baud rate : 115200    */
/* Stop bit     */
/* parity       */
/* data bits    */
uint16_t g_u16CtrlSignal = 0;     /* BIT0: DTR(Data Terminal Ready) , BIT1: RTS(Request To Send) */

/*--------------------------------------------------------------------------*/
#define RXBUFSIZE           1024 /* RX buffer size */
#define TXBUFSIZE           1024 /* RX buffer size */

#define TX_FIFO_SIZE        16  /* TX Hardware FIFO size */


/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
uint8_t gu8DeviceDescriptor[] =
{
    LEN_DEVICE,     /* bLength */
    DESC_DEVICE,    /* bDescriptorType */
    0x10, 0x01,     /* bcdUSB */
    0x02,           /* bDeviceClass */
    0x00,           /* bDeviceSubClass */
    0x00,           /* bDeviceProtocol */
    EP0_MAX_PKT_SIZE,   /* bMaxPacketSize0 */
    /* idVendor */
    USBD_VID & 0x00FF,
    (USBD_VID & 0xFF00) >> 8,
    /* idProduct */
    USBD_PID & 0x00FF,
    (USBD_PID & 0xFF00) >> 8,
    0x00, 0x03,     /* bcdDevice */
    0x01,           /* iManufacture */
    0x02,           /* iProduct */
    0x00,           /* iSerialNumber - no serial */
    0x01            /* bNumConfigurations */
};

/*!<USB Configure Descriptor */
uint8_t gu8ConfigDescriptor[] =
{
    LEN_CONFIG,     /* bLength              */
    DESC_CONFIG,    /* bDescriptorType      */
    0x43, 0x00,     /* wTotalLength         */
    0x02,           /* bNumInterfaces       */
    0x01,           /* bConfigurationValue  */
    0x00,           /* iConfiguration       */
    0xC0,           /* bmAttributes         */
    0x32,           /* MaxPower             */

    /* INTERFACE descriptor */
    LEN_INTERFACE,  /* bLength              */
    DESC_INTERFACE, /* bDescriptorType      */
    0x00,           /* bInterfaceNumber     */
    0x00,           /* bAlternateSetting    */
    0x01,           /* bNumEndpoints        */
    0x02,           /* bInterfaceClass      */
    0x02,           /* bInterfaceSubClass   */
    0x01,           /* bInterfaceProtocol   */
    0x00,           /* iInterface           */

    /* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */

    /* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x01,           /* Call management functional descriptor */
    0x00,           /* BIT0: Whether device handle call management itself. */
    /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,           /* Interface number of data class interface optionally used for call management */

    /* Communication Class Specified INTERFACE descriptor */
    0x04,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x02,           /* Abstract control management funcational descriptor subtype */
    0x00,           /* bmCapabilities       */

    /* Communication Class Specified INTERFACE descriptor */
    0x05,           /* bLength              */
    0x24,           /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,           /* bDescriptorSubType   */
    0x00,           /* bMasterInterface     */
    0x01,           /* bSlaveInterface0     */

    /* ENDPOINT descriptor */
    LEN_ENDPOINT,                   /* bLength          */
    DESC_ENDPOINT,                  /* bDescriptorType  */
    (EP_INPUT | INT_IN_EP_NUM),     /* bEndpointAddress */
    EP_INT,                         /* bmAttributes     */
    EP4_MAX_PKT_SIZE, 0x00,         /* wMaxPacketSize   */
    0x01,                           /* bInterval        */

    /* INTERFACE descriptor */
    LEN_INTERFACE,  /* bLength              */
    DESC_INTERFACE, /* bDescriptorType      */
    0x01,           /* bInterfaceNumber     */
    0x00,           /* bAlternateSetting    */
    0x02,           /* bNumEndpoints        */
    0x0A,           /* bInterfaceClass      */
    0x00,           /* bInterfaceSubClass   */
    0x00,           /* bInterfaceProtocol   */
    0x00,           /* iInterface           */

    /* ENDPOINT descriptor */
    LEN_ENDPOINT,                   /* bLength          */
    DESC_ENDPOINT,                  /* bDescriptorType  */
    (EP_INPUT | BULK_IN_EP_NUM),    /* bEndpointAddress */
    EP_BULK,                        /* bmAttributes     */
    EP2_MAX_PKT_SIZE, 0x00,         /* wMaxPacketSize   */
    0x00,                           /* bInterval        */

    /* ENDPOINT descriptor */
    LEN_ENDPOINT,                   /* bLength          */
    DESC_ENDPOINT,                  /* bDescriptorType  */
    (EP_OUTPUT | BULK_OUT_EP_NUM),  /* bEndpointAddress */
    EP_BULK,                        /* bmAttributes     */
    EP3_MAX_PKT_SIZE, 0x00,         /* wMaxPacketSize   */
    0x00,                           /* bInterval        */
};

/*!<USB Language String Descriptor */
uint8_t gu8StringLang[4] =
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
uint8_t gu8VendorStringDesc[] =
{
    16,
    DESC_STRING,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
uint8_t gu8ProductStringDesc[] =
{
    32,             /* bLength          */
    DESC_STRING,    /* bDescriptorType  */
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0, 'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0
};

/*!<USB BOS Descriptor */
const uint8_t gu8BOSDescriptor[] =
{
    LEN_BOS,        /* bLength */
    DESC_BOS,       /* bDescriptorType */
    /* wTotalLength */
    0x0C & 0x00FF,
    (0x0C & 0xFF00) >> 8,
    0x01,           /* bNumDeviceCaps */

    /* Device Capability */
    0x7,            /* bLength */
    DESC_CAPABILITY,/* bDescriptorType */
    CAP_USB20_EXT,  /* bDevCapabilityType */
    0x02, 0x00, 0x00, 0x00  /* bmAttributes */
};

uint8_t *gpu8UsbString[4] =
{
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    NULL,
};

uint8_t *gu8UsbHidReport[3] =
{
    NULL,
    NULL,
    NULL,
};

uint32_t gu32UsbHidReportLen[3] =
{
    0,
    0,
    0,
};

uint32_t gu32ConfigHidDescIdx[3] =
{
    0,
    0,
    0,
};

const S_USBD_INFO_T gsInfo =
{
    (uint8_t *)gu8DeviceDescriptor,
    (uint8_t *)gu8ConfigDescriptor,
    (uint8_t **)gpu8UsbString,
    (uint8_t **)gu8UsbHidReport,
    (uint8_t *)gu8BOSDescriptor,
    (uint32_t *)gu32UsbHidReportLen,
    (uint32_t *)gu32ConfigHidDescIdx
};


/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
/* UART0 */
volatile uint8_t g_au8ComRbuf[RXBUFSIZE];
volatile uint16_t g_u16ComRbytes = 0;
volatile uint16_t g_u16ComRhead = 0;
volatile uint16_t g_u16ComRtail = 0;

volatile uint8_t g_au8ComTbuf[TXBUFSIZE];
volatile uint16_t g_u16ComTbytes = 0;
volatile uint16_t g_u16ComThead = 0;
volatile uint16_t g_u16ComTtail = 0;

uint8_t g_au8RxBuf[64] = {0};
uint8_t *g_pu8RxBuf = 0;
uint32_t g_u32RxSize = 0;
uint32_t g_u32TxSize = 0;

volatile int8_t g_i8BulkOutReady = 0;



volatile uint32_t g_USBTX_LED_Timer;
volatile uint32_t g_USBRX_LED_Timer;
extern volatile uint32_t g_u32SysTick;
/*--------------------------------------------------------------------------*/
/*Utilits*/
/*--------------------------------------------------------------------------*/
/**
  * @brief  Return available bytes from USB comport
  * @param  None.
  * @retval available.
  */
uint8_t USB_available(){
	return g_u16ComTbytes;
}
/*--------------------------------------------------------------------------*/
/**
  * @brief  Clear USB buffer
  * @param  None.
  * @retval None.
  */
void USB_flush(){
	__set_PRIMASK(1);
	g_u16ComTbytes = 0;
	g_u16ComThead = 0;
	g_u16ComTtail = 0;
	__set_PRIMASK(0);
	return;
}

/*--------------------------------------------------------------------------*/
/**
  * @brief  read 1 byte from USB buffer
  * @param  None.
  * @retval character, single char
  */
char USB_read(){
    /* Process the software Tx FIFO */
	char c;
    if(g_u16ComTbytes)
    {
    	c = g_au8ComTbuf[g_u16ComThead++];
		if(g_u16ComThead >= TXBUFSIZE)
			g_u16ComThead = 0;

		__set_PRIMASK(1);
		g_u16ComTbytes--;
		__set_PRIMASK(0);
    }
    return c;
}

/*--------------------------------------------------------------------------*/
/**
  * @brief  send 1 byte to USB buffer
  * @param  character, single char
  * @retval None.
  */
void USB_send(char c){
    /* Check if buffer full */
    if(g_u16ComRbytes < RXBUFSIZE)
    {
        /* Enqueue the character */
        g_au8ComRbuf[g_u16ComRtail++] = c;
        if(g_u16ComRtail >= RXBUFSIZE)
            g_u16ComRtail = 0;
        g_u16ComRbytes++;
    }
    else
    {
        /* FIFO over run */
    }
}
/*--------------------------------------------------------------------------*/
/**
  * @brief  send 1 byte to USB buffer, this function is required for usbprintf 
  * @param  character, single char
  * @retval None.
  */
void _putchar(char character){
	USB_send(character);
	VCOM_TransferData(g_u32SysTick);
}
/*--------------------------------------------------------------------------*/
/**
  * @brief  send 1 byte to USB buffer, this function is required for usbprintf 
  * @param  character, single char
  * @retval None.
  */
void USB_output(char character, void* arg)
{
  // opt. evaluate the argument and send the char somewhere
    send_USB(character);
}

/*--------------------------------------------------------------------------*/
/**
  * @brief  Turn on TX/RX LED for 100ms (Only turn on the LED here, there is a check in systick to turn LED off)
  * @param  TXRX, 1 for TX LED, 0 for RX LED.
  * @retval None.
  */

void USB_LEDOn(int TXRX){
    if(TXRX == 1){
        g_USBRX_LED_Timer = g_u32SysTick+100;
        PC0_NS = 1;
    }
    else{
        g_USBTX_LED_Timer = g_u32SysTick+100;
        PC1_NS = 1;
    }
}
/*---------------------------------------------------------------------------------------------------------*/
/* Send Callback function                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void VCOM_TransferData()
{

    if(g_u32SysTick > g_USBTX_LED_Timer){
        PC1_NS = 0;
    }
    if(g_u32SysTick > g_USBRX_LED_Timer){
        PC0_NS = 0;
    }

    int32_t i, i32Len;

    /* Check whether USB is ready for next packet or not*/
    if(g_u32TxSize == 0)
    {
        /* Check whether we have new COM Rx data to send to USB or not */
        if(g_u16ComRbytes)
        {
            i32Len = g_u16ComRbytes;
            if(i32Len > EP2_MAX_PKT_SIZE)
                i32Len = EP2_MAX_PKT_SIZE;

            for(i = 0; i < i32Len; i++)
            {
                g_au8RxBuf[i] = g_au8ComRbuf[g_u16ComRhead++];
                if(g_u16ComRhead >= RXBUFSIZE)
                    g_u16ComRhead = 0;
            }

            __set_PRIMASK(1);
            g_u16ComRbytes -= i32Len;
            __set_PRIMASK(0);

            g_u32TxSize = i32Len;

            USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), (uint8_t *)g_au8RxBuf, i32Len);
            USBD_SET_PAYLOAD_LEN(EP2, i32Len);

        }
        else
        {
            /* Prepare a zero packet if previous packet size is EP2_MAX_PKT_SIZE and
               no more data to send at this moment to note Host the transfer has been done */
            i32Len = USBD_GET_PAYLOAD_LEN(EP2);
            if(i32Len == EP2_MAX_PKT_SIZE)
                USBD_SET_PAYLOAD_LEN(EP2, 0);
        }
    }

    /* Process the Bulk out data when bulk out data is ready. */
    if(g_i8BulkOutReady && (g_u32RxSize <= TXBUFSIZE - g_u16ComTbytes))
    {
        for(i = 0; i < g_u32RxSize; i++)
        {
            g_au8ComTbuf[g_u16ComTtail++] = g_pu8RxBuf[i];
            if(g_u16ComTtail >= TXBUFSIZE)
                g_u16ComTtail = 0;
        }

        __set_PRIMASK(1);
        g_u16ComTbytes += g_u32RxSize;
        __set_PRIMASK(0);

        g_u32RxSize = 0;
        g_i8BulkOutReady = 0; /* Clear bulk out ready flag */

        /* Ready to get next BULK out */

        USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    }

}
/*--------------------------------------------------------------------------*/
/**
  * @brief  USBD IRQ.
  * @param  None.
  * @retval None.
  */
void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    //------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_FLDET)
    {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if(USBD_IS_ATTACHED())
        {
            /* USB Plug In */
            USBD_ENABLE_USB();
        }
        else
        {
            /* USB Un-plug */
            USBD_DISABLE_USB();
        }
    }

    //------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_WAKEUP)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }

    //------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_BUS)
    {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if(u32State & USBD_STATE_USBRST)
        {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if(u32State & USBD_STATE_SUSPEND)
        {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
        }
        if(u32State & USBD_STATE_RESUME)
        {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
        }
    }

    //------------------------------------------------------------------
    if(u32IntSts & USBD_INTSTS_USB)
    {
        extern uint8_t g_USBD_au8SetupPacket[];

        // USB event
        if(u32IntSts & USBD_INTSTS_SETUP)
        {
            // Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

        // EP events
        if(u32IntSts & USBD_INTSTS_EP0)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            // control IN
            USBD_CtrlIn();
        }

        if(u32IntSts & USBD_INTSTS_EP1)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
            // control OUT
            USBD_CtrlOut();

            // In ACK of SET_LINE_CODE
            if(g_USBD_au8SetupPacket[1] == SET_LINE_CODE)
            {
                //if(g_USBD_au8SetupPacket[4] == 0)  /* VCOM-1 */
                    //VCOM_LineCoding(0); /* Apply UART settings */
            }
        }

        if(u32IntSts & USBD_INTSTS_EP2)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            // Bulk IN
            EP2_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP3)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // Bulk Out
            EP3_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP4)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }

        if(u32IntSts & USBD_INTSTS_EP5)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        if(u32IntSts & USBD_INTSTS_EP6)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }

        if(u32IntSts & USBD_INTSTS_EP7)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }

        if(u32IntSts & USBD_INTSTS_EP8)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP8);
        }

        if(u32IntSts & USBD_INTSTS_EP9)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP9);
        }

        if(u32IntSts & USBD_INTSTS_EP10)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP10);
        }

        if(u32IntSts & USBD_INTSTS_EP11)
        {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP11);
        }
    }
}

void EP2_Handler(void)
{
	USB_LEDOn(1);
    g_u32TxSize = 0;
}


void EP3_Handler(void)
{
	USB_LEDOn(0);
    /* Bulk OUT */
    g_u32RxSize = USBD_GET_PAYLOAD_LEN(EP3);
    g_pu8RxBuf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));

    /* Set a flag to indicate bulk out ready */
    g_i8BulkOutReady = 1;
}




/*--------------------------------------------------------------------------*/
/**
  * @brief  USBD Endpoint Config.
  * @param  None.
  * @retval None.
  */
void VCOM_Init(void)
{
    /* Init setup packet buffer */
    /* Buffer for setup packet -> [0 ~ 0x7] */
	USBD_NS->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Bulk IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | BULK_IN_EP_NUM);
    /* Buffer offset for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Bulk Out endpoint, address 2 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | BULK_OUT_EP_NUM);
    /* Buffer offset for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    /* EP4 ==> Interrupt IN endpoint, address 3 */
    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | INT_IN_EP_NUM);
    /* Buffer offset for EP4 ->  */
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);
}


void VCOM_ClassRequest(void)
{
    uint8_t au8Buf[8];

    USBD_GetSetupPacket(au8Buf);

    if(au8Buf[0] & 0x80)    /* request data transfer direction */
    {
        // Device to host
        switch(au8Buf[1])
        {
            case GET_LINE_CODE:
            {
                if(au8Buf[4] == 0)    /* VCOM-1 */
                {
                    USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)), (uint8_t *)&g_LineCoding, 7);
                }
                /* Data stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 7);
                /* Status stage */
                USBD_PrepareCtrlOut(0, 0);
                break;
            }
            default:
            {
                /* Setup error, stall the device */
                USBD_SetStall(0);
                break;
            }
        }
    }
    else
    {
        // Host to device
        switch(au8Buf[1])
        {
            case SET_CONTROL_LINE_STATE:
            {
                if(au8Buf[4] == 0)    /* VCOM-1 */
                {
                    g_u16CtrlSignal = au8Buf[3];
                    g_u16CtrlSignal = (g_u16CtrlSignal << 8) | au8Buf[2];
                    //printf("RTS=%d  DTR=%d\n", (g_u16CtrlSignal0 >> 1) & 1, g_u16CtrlSignal0 & 1);
                }

                /* Status stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 0);
                break;
            }
            case SET_LINE_CODE:
            {
                if(au8Buf[4] == 0)  /* VCOM-1 */
                    USBD_PrepareCtrlOut((uint8_t *)&g_LineCoding, 7);

                /* Status stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 0);

                break;
            }
            default:
            {
                // Stall
                /* Setup error, stall the device */
                USBD_SetStall(0);
                break;
            }
        }
    }
}

void VCOM_LineCoding(uint8_t u8Port)
{
    uint32_t u32Reg;
    uint32_t u32Baud_Div;

    if(u8Port == 0)
    {
        NVIC_DisableIRQ(UART0_IRQn);
        // Reset software FIFO
        g_u16ComRbytes = 0;
        g_u16ComRhead = 0;
        g_u16ComRtail = 0;

        g_u16ComTbytes = 0;
        g_u16ComThead = 0;
        g_u16ComTtail = 0;

        // Reset hardware FIFO
        UART0->FIFO = (UART_FIFO_TXRST_Msk | UART_FIFO_RXRST_Msk);

        // Set baudrate
        u32Baud_Div = UART_BAUD_MODE2_DIVIDER(__HIRC, g_LineCoding.u32DTERate);

        if(u32Baud_Div > 0xFFFF)
            UART0->BAUD = (UART_BAUD_MODE0 | UART_BAUD_MODE0_DIVIDER(__HIRC, g_LineCoding.u32DTERate));
        else
            UART0->BAUD = (UART_BAUD_MODE2 | u32Baud_Div);

        // Set parity
        if(g_LineCoding.u8ParityType == 0)
            u32Reg = 0; // none parity
        else if(g_LineCoding.u8ParityType == 1)
            u32Reg = 0x08; // odd parity
        else if(g_LineCoding.u8ParityType == 2)
            u32Reg = 0x18; // even parity
        else
            u32Reg = 0;

        // bit width
        switch(g_LineCoding.u8DataBits)
        {
            case 5:
                u32Reg |= 0;
                break;
            case 6:
                u32Reg |= 1;
                break;
            case 7:
                u32Reg |= 2;
                break;
            case 8:
                u32Reg |= 3;
                break;
            default:
                break;
        }

        // stop bit
        if(g_LineCoding.u8CharFormat > 0)
            u32Reg |= 0x4; // 2 or 1.5 bits

        UART0->LINE = u32Reg;

        // Re-enable UART interrupt
        NVIC_EnableIRQ(UART0_IRQn);
    }
}






