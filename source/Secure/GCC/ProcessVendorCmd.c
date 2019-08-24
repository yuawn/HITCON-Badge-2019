/**************************************************************************//**
 * @file     ProcessVendorCmd.c
 * @version  V3.00
 * @brief    Process vendor command.
 *
 * @copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "NuMicro.h"


/*---------------------------------------------------------------------------------------------------------*/
/* Global variables for initial SecureISP function                                                         */
/*---------------------------------------------------------------------------------------------------------*/
extern ISP_INFO_T   g_ISPInfo;
volatile char g_acIDsR[65], g_acIDsS[65];

extern int32_t XOM_CalIDsECDSA(char *pOutR, char *pOutS);

static void BytesSwap(char *buf, int32_t len)
{
    int32_t i;
    char    tmp;

    for(i = 0; i < (len / 2); i++)
    {
        tmp = buf[len - i - 1];
        buf[len - i - 1] = buf[i];
        buf[i] = tmp;
    }
}

void Exec_VendorFunction(uint32_t *pu32Buf, uint32_t u32Len)
{
    uint32_t i, au32Data[12];
    uint32_t au32Sign[8];

    memset((void *)au32Data, 0x0, sizeof(au32Data));
    BL_GetVendorData((uint32_t *)&g_ISPInfo, au32Data, pu32Buf);

    printf("Received data are :\n");
    for(i=0; i<(u32Len/4); i++)
        printf("0x%08x, ", au32Data[i]);
    printf("\n\n");

    if(au32Data[0] == 0x1000) // return IDs
    {
        u32Len = 4 * 8;
        au32Data[0] = SYS->PDID;
        au32Data[1] = BL_ReadUID(0);
        au32Data[2] = BL_ReadUID(1);
        au32Data[3] = BL_ReadUID(2);
        au32Data[4] = BL_ReadUCID(0);
        au32Data[5] = BL_ReadUCID(1);
        au32Data[6] = BL_ReadUCID(2);
        au32Data[7] = BL_ReadUCID(3);
        BL_ReturnVendorData(au32Data, u32Len, pu32Buf);
    }

    if(au32Data[0] == 0x2000) // process and return IDs ECDSA signature-R
    {
        memset((char *)g_acIDsR, 0x0, sizeof(g_acIDsR));
        memset((char *)g_acIDsS, 0x0, sizeof(g_acIDsS));
        XOM_CalIDsECDSA((char *)g_acIDsR, (char *)g_acIDsS);
        printf("IDs R: %s\n", g_acIDsR);
        printf("IDs S: %s\n", g_acIDsS);

        u32Len = sizeof(au32Sign); // 256-bits

        memset((void *)au32Data, 0x0, sizeof(au32Data));
        memset((void *)au32Sign, 0x0, sizeof(au32Sign));
        XECC_Hex2Reg((char *)g_acIDsR, (uint32_t *)au32Sign);
        BytesSwap((char *)au32Sign, sizeof(au32Sign));
        memcpy(au32Data, au32Sign, sizeof(au32Sign));

        BL_ReturnVendorData(au32Data, u32Len, pu32Buf);
    }

    if(au32Data[0] == 0x2001) // process and return IDs ECDSA signature-S
    {
        u32Len = sizeof(au32Sign); // 256-bits

        memset((void *)au32Data, 0x0, sizeof(au32Data));
        memset((void *)au32Sign, 0x0, sizeof(au32Sign));
        XECC_Hex2Reg((char *)g_acIDsS, (uint32_t *)au32Sign);
        BytesSwap((char *)au32Sign, sizeof(au32Sign));
        memcpy(au32Data, au32Sign, sizeof(au32Sign));

        BL_ReturnVendorData(au32Data, u32Len, pu32Buf);
    }
}

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
