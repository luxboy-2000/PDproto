
/* **********************************************************************
 * \file  main.h
 * \brief chip level information for apt32f031
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2024-7 <td>V1.0 <td>WNN     <td>new STDLib
 * </table>
 * *********************************************************************
*/



/* Define to prevent recursive inclusion -------------------------------------*/ 
#ifndef __MAIN_H__
#define __MAIN_H__
/* Includes ------------------------------------------------------------------*/

#include "inc.h"
#include "pd_protocol.h"
//===================================================================================
#ifndef nop
#define nop		asm("nop")
#endif

#ifndef uint8_t
#define uint8_t		unsigned char
#endif

#ifndef uint16_t
#define uint16_t	unsigned short
#endif

#ifndef uint32_t
#define uint32_t	unsigned int
#endif
//===================================================================================
extern volatile uint8_t 	byTimer_100us;
//===================================================================================
//===================================================================================
extern void apt32f031_init(void);
extern void syscon_iwdt_reload(); 

extern void my_printf(const char *fmt, ...);

extern void delay_nms(unsigned int wT);
extern void delay_nus(unsigned int wT);
//===================================================================================
#endif
/******************* (C) COPYRIGHT 2024 APT Chip *****END OF FILE****/