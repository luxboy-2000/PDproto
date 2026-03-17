/***********************************************************************//** 
 * \file  initial.c
 * \brief GPIO测试
 * 使用说明：
 * 			1、本工程示例外部中断的使用方法
 * 			2、操作说明：
 * 				1）PA01设为外部中断口，当有下降沿事件时，会触发中断，进入EXI1IntHandler()
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 *  <tr><td> 2025-5 <td>V1.0 	<td>WCH     <td>new STDLib
 * </table>
 * *********************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* externs--------------------------------------------------------------------*/
extern void console_init(void);
/* private function-----------------------------------------------------------*/
void syscon_config(void);
void gpio_config(void);

/* global variablesr----------------------------------------------------------*/
/* Private variablesr---------------------------------------------------------*/
/* defines -------------------------------------------------------------------*/

/** \brief delay函数, 非准确定时，且随系统时钟变化
 *  \param[in] wNms: 单位ms
 *  \return none
 */
void delay_nms(unsigned int wNms)
{
    volatile unsigned int i,j ,k=0;
    j = 800* wNms;
    for ( i = 0; i < j; i++ )
    {
        k++;
    }
}

/** \brief delay函数，非准确定时，且随系统时钟变化
 *  \param[in] wNus: 单位us
 *  \return none
 */
void delay_nus(unsigned int wNus)
{
    volatile unsigned int i,j ,k=0;
    j = 1* wNus;
    for ( i = 0; i < j; i++ )
    {
        k++;
    }
}

/** \brief 示例串口（UART0）奇校验配置
 *  \param[in] none
 * 			   
 *  \return none
 */
void uart_config(void)
{	
	gpio_configure(GPIOB0, 0 ,UART0_RX);  						//配置PB0为UART0 RX
	gpio_configure(GPIOB0, 1 ,UART0_TX); 						//配置PB1为UART0 TX
	gpio_pull_configure(GPIOB0,0,PULL_MODE_PULLUP);				//PB0上拉
	

	uart_configure(UART0,625,UART_DATA_8BIT,UART_NBSTOP_1BIT,UART_PAR_NONE); 		//波特率 72000000/625=115200,数据位为 8 bit，1bit 停止位，无校验
											
}


/** \brief 芯片初始化函数
 *  \param[in]none
 *  \return none
 */
void apt32f031_init(void)
{
    syscon_config();      		    //syscon相关初始化
	uart_config();					//uart初始化
	CK_CPU_ENALLNORMALIRQ;          //使能全局中断
	
}

/** \brief 系统控制器初始化
 *  \param[in]none
 *  \return none
 */
void syscon_config(void)
{
	syscon_ip_pclk_enable();
	VIC ->ISER[0] = 0xffffffff;

	/*------------  系统时钟和外设时钟设置  --------------------------------*/	
	syscon_osc_enable(ENDIS_IMOSC);												//使能IMOSC
	syscon_pll_configure(PLL_SRC_IMOSC, PLLX3,PLL_UNLOCK_RST_EN);				//PLL = 3 * IMOSC，PLL失锁时产生系统复位
	syscon_osc_enable(ENDIS_PLL);												//使能pll
	syscon_hclk_pclk_configure(SYSCLK_PLL, HCLK_DIV_1,PCLK_DIV_1,F_48_72MHz);   //将PLL作为系统时钟，HCLK = PLLCLK, PCLK = HCLK/1, 48 MHz <系统时钟频率 <= 72MHz
	
//	gpio_configure(GPIOB0, 3, CLO);
//	syscon_clo_configure(CLO_PCLK,CLO_DIV16);
	
	syscon_iwdt_disable();	
}


/** \brief 示例gpio配置
 *  \param[in] none
 *  \return none
 * exi function: EXI0_INT= EXI_GRP0/EXI_GRP16,
 * 				 EXI1_INT= EXI_GRP1/EXI_GRP17, 
 * 				 EXI2_INT=EXI_GRP2~EXI_GRP3/EXI_GRP18/EXI_GRP19, 
 * 				 EXI3_INT=EXI_GRP4~EXI_GRP9, 
 * 				 EXI4_INT=EXI_GRP10~EXI_GRP15    
 */
void gpio_config(void)
{
	//使用GPIOA01作为EXI group 1
	gpio_configure(GPIOA0, 1, PIN_INPUT); //PA1设为输入
	gpio_pull_configure(GPIOA0, 1,PULL_MODE_PULLUP);//PA1设为上拉
	gpio_igroup_configure(EXI_GRP1, PA0, 1);//PA1设为EXI GRP1
	syscon_exi_mode_configure(EXI_GRP1 , EXI_F);//设置EXI GRP1 触发方式为下降沿触发
	gpio_exi_enable(GPIOA0, 1);//PA1管脚外部中断使能
	syscon_exi_enable(EXI_GRP1);//使能EXI_GRP1中断
	
//	NVIC_SetWakeupIRQ(EXIV1_INT);//低功耗唤醒中断配置
	
}

/******************* (C) COPYRIGHT 2024 APT Chip *****END OF FILE****/