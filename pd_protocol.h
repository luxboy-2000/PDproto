
/* **********************************************************************
 * \file  
 * \brief chip level information for apt32f031
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2024-7 <td>V1.0 <td>WNN     <td>new STDLib
 * </table>
 * *********************************************************************
*/



/* Define to prevent recursive inclusion -------------------------------------*/ 
#ifndef _PD_PROTOCOL_H_
#define _PD_PROTOCOL_H_
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "pd_init.h"
//===================================================================================
typedef enum
{
	e5V = 0,					
	e9V,
	e12V,
	e15V,
	e20V,
}PD_Sink_Request_e;
//===================================================================================
extern void 	pd_protocol_sink_init(void);
extern void 	pd_protocol_sink_deinit(void);

extern void 	pd_1ms_int_handler(void);				//需要定时处理的函数，定时处理时长为1ms
extern void 	pd_cmp_int_handler(void);				//cmp中断需处理
extern uint8_t	pd_sink_RequestPower(PD_Sink_Request_e eSink_Voltage);
extern uint8_t	pd_sink_request_power_ex(uint16_t hwVoltage_mV,uint16_t hwCurrent_mA);
extern uint8_t	pd_power_supply(uint16_t hwVoltage_mV,uint16_t hwCurrent_mA);
//===================================================================================
#endif
/******************* (C) COPYRIGHT 2024 APT Chip *****END OF FILE****/
