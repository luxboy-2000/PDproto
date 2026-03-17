/***********************************************************************//** 
 * \file  iostring.c
 * \brief my_printf函数实现
 *
 * 	使用说明：
 * 			1、my_printf支持两种输出方式
 * 				1）通过debug print输出，该方式类似于semihost，无需额外硬件连线，但仅在调试模式下有效
 * 				2）通过UART硬件输出，需要将UART_TX管脚通过TTL转换芯片连接到PC端，脱离调试环境仍然可以输出
 *  		2、操作说明：
 * 				1）使用debug print输出时，确保下面3处操作后，调用my_printf()函数的输出就会出现在serial pane的Debug（print）页
 * 					a、iostring.c中 _DEBUG_UART_IO没有被define
 * 					b、工程配置中使能debug print（魔术棒-> Debug -> Use ICE settings -> ICE -> 勾选Enable debug print）
 * 					c、进入调试状态后，开启顶层菜单 view -> Serial Pane
 * 				2）使用UART硬件输出时，确保硬件连接正确, 且PC端串口工具配置正确后，调用my_printf()函数的输出即可在串口工具端看到
 * 					a、iostring.c中，#define _DEBUG_UART_IO
 * 					b、PB02（UART0_TX）连接PC端 UART_RX；PA07（UART0_RX）连接PC端 UART_TX；并共地
 * 					c、配置PC端串口工具 115200/数据位8 bit/停止位1bit/无校验
 * 			3、其他说明
 * 				my_printf()函数是一个轻量化的输出函数，参数传递方式和printf()相同（没有printf支持的方式多），可实现常用的打印输出
 * 
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2024-10 <td>V1.0 <td>GQQ     <td>new STDLib
 * </table>
 * *********************************************************************
*/


/* Includes ------------------------------------------------------------------*/
#include "inc.h"
#include "stdarg.h"
#include "stddef.h"
#include "stdio.h"

/* externs--------------------------------------------------------------------*/
/* private function-----------------------------------------------------------*/
/* global variablesr----------------------------------------------------------*/
/* Private variablesr---------------------------------------------------------*/
/* defines -------------------------------------------------------------------*/
#define LDCC_DATA_P 		0xe001105c /* LDCC Register. */
#define LDCC_BIT_STATUS 	0x80000000 /* LDCC Status bit. */
#define _DEBUG_UART_IO


/** \brief 输出端口配置函数
 *  \param[in]none
 *  \return none
 */
void __putchar__ (char ch) 
{
#ifdef _DEBUG_UART_IO
	uart_write_byte(UART0,ch);			//uart 0
//	uart_write_byte(UART1,ch);			//uart 1
#else
	//select debug serial Pane
	volatile unsigned int *pdata = (unsigned int *)LDCC_DATA_P;
	while (*pdata & LDCC_BIT_STATUS);	//Waiting for data read.
	*pdata = ch;
#endif
}


/** \brief 整数转换成字符串函数
 *  \param[in]none
 *  \return none
 */
char *myitoa(int value, int* string, int radix)
{

		int tmp[33];
		int* tp = tmp;
		int i;
		unsigned v;
		int sign;
		int* sp;
		
		if (radix > 36 || radix <= 1)
		{
			return 0;
		}
		
		sign = (radix == 10 && value < 0);
		if (sign)
		v = -value;
		else
		v = (unsigned)value;
		while (v || tp == tmp)
		{
			i = v % radix;
			v = v / radix;
			if (i < 10) {
			*tp++ = i+'0';
			
			} else {
			*tp++ = i + 'a' - 10;
			
			}
					
		}
		
		sp = string;
		
		if (sign)
		*sp++ = '-';
		while (tp > tmp)
		*sp++ = *--tp;
		*sp = 0;
		return (char *)string;
}

/** \brief 格式化输出函数
 *  \param[in]none
 *  \return none
 */
void my_printf(const char *fmt, ...)
{

	const char *s;
	const int *str;
	//const int *s;
	int d;
    //char ch, *pbuf, buf[16];
	char ch, *pbuf;
	int buf[16];
    va_list ap;
    va_start(ap, fmt);
    while (*fmt) {
        if (*fmt != '%') {
              __putchar__(*fmt++);
            continue;
        }
        switch (*++fmt) {
            case 's':
                s = va_arg(ap, const char *);
                for ( ; *s; s++) {
                 __putchar__(*s);
                }
                break;
            case 'd':
                d = va_arg(ap, int);
                myitoa(d, buf, 10);
                for (str = buf; *str; str++) {
                 __putchar__(*str);
                }
                break;
                
            case 'x':
            case 'X':
                d = va_arg(ap, int);
                myitoa(d, buf, 16);
                for (str = buf; *str; str++) {
                 __putchar__(*str);
                }
                break;                
            // Add other specifiers here...
            case 'c':
            case 'C':
            	ch = (unsigned char)va_arg(ap, int);
        	    pbuf = &ch;
            	__putchar__(*pbuf);
                break;
            default:  
                __putchar__(*fmt);
                break;
        }
        fmt++;
    }
    va_end(ap);

}
