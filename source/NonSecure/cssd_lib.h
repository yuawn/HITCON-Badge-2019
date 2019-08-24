/**************************************************************************//**
 * @file     cssd_lib.h
 * @version  V1.00
 * @brief    M2351 Collaborative Secure Software Development Library header file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __CSSD_LIB_H__
#define __CSSD_LIB_H__

/*----------------------------------------------------------------------------
  NonSecure Callable Functions from Secure Region
 *----------------------------------------------------------------------------*/
extern int32_t nsc_SetPixelColor(uint16_t n, uint32_t c);
extern int32_t nsc_SetBrigntness(uint8_t b);
extern int32_t nsc_PixelShow(void);
extern int32_t nsc_SetPixelColorRGB(uint16_t n, uint8_t r, uint8_t g, uint8_t b);
extern uint32_t nsc_GetSystemCoreClock(void);
extern void nsc_CheckButtonSequence(int flag);
extern uint32_t nsc_GetLockReg(void);
extern void ClearPixels(void);
extern uint32_t nsc_display(uint32_t pattern,int32_t controlA,int32_t controlB);
extern uint32_t nsc_randomInt(void);
extern uint32_t nsc_unlock(char*);
extern uint32_t nsc_check_pattern( uint32_t );
extern uint32_t nsc_isPwned(void);

#endif //__CSSD_LIB_H__

