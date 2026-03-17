/***********************************************************************//** 
 * \file  main.c
 * \brief PD诱骗DEMO
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2025-5 <td>V1.0 	<td>WCH     <td>new STDLib
 * </table>
 * *********************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <string.h>
/* externs--------------------------------------------------------------------*/

/* private function-----------------------------------------------------------*/

/* global variables-----------------------------------------------------------*/

/* Private variables----------------------------------------------------------*/

/* defines -------------------------------------------------------------------*/

volatile uint8_t 	byTimer_100us;
uint8_t		byGet_Voltage_mode;
uint16_t	hwGet_Voltage_test_Cnt = 0;
/*************************************************************/
//main
/*************************************************************/
int main(void) 
{ 
	apt32f031_init();	

//	user_adc_init();
	pd_protocol_sink_init();
	user_timer_init();						//100us定时
	
    while(1)
	{
		syscon_iwdt_reload(); 
		if(byTimer_100us)
		{
			byTimer_100us = 0;
			
			switch(byGet_Voltage_mode)
			{
				case 0:
					if(!pd_power_supply(5000,1000))
						hwGet_Voltage_test_Cnt = 0;
					else
					{
						if(++hwGet_Voltage_test_Cnt >= 5000)
						{
							hwGet_Voltage_test_Cnt = 0;
							byGet_Voltage_mode ++;
						}
					}
				break;
				case 1:
					if(!pd_power_supply(9000,1000))//返回0时不满足条件
						hwGet_Voltage_test_Cnt = 0;
					else
					{
						if(++hwGet_Voltage_test_Cnt >= 5000)
						{
							hwGet_Voltage_test_Cnt = 0;
							byGet_Voltage_mode ++;
						}
					}
				break;
				case 2:
					if(!pd_power_supply(12000,1000))
						hwGet_Voltage_test_Cnt = 0;
					else
					{
						if(++hwGet_Voltage_test_Cnt >= 5000)
						{
							hwGet_Voltage_test_Cnt = 0;
							byGet_Voltage_mode ++;
						}
					}
				break;
				case 3:
					if(!pd_power_supply(15000,1000))
						hwGet_Voltage_test_Cnt = 0;
					else
					{
						if(++hwGet_Voltage_test_Cnt >= 5000)
						{
							hwGet_Voltage_test_Cnt = 0;
							byGet_Voltage_mode ++;
						}
					}
				break;
				case 4:
					if(!pd_power_supply(20000,1000))
						hwGet_Voltage_test_Cnt = 0;
					else
					{
						if(++hwGet_Voltage_test_Cnt >= 5000)
						{
							hwGet_Voltage_test_Cnt = 0;
							byGet_Voltage_mode ++;
						}
					}
				break;
				default:
					byGet_Voltage_mode = 0;
				break;
			}
		}
    }
}
/******************* (C) COPYRIGHT 2024 APT Chip *****END OF FILE****/