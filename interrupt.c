/***********************************************************************//** 
 * \file  interrupt.c
 * \brief interrupt handlers implementation declared in startup file
 * 		 
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2025-2 <td>V1.0 <td>WNN     <td>new STDLib
 * </table>
 * *********************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* externs--------------------------------------------------------------------*/
/* private function-----------------------------------------------------------*/
/* global variables----------------------------------------------------------*/
/* Private variables---------------------------------------------------------*/


/*************************************************************/
/* hardware vector interrupt Handler
*************************************************************/
extern void delay_nms(unsigned int t);

/*************************************************************/
//CORET Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void CORETHandler(void)
{
	// ISR content ...
}


/*************************************************************/
//SYSCON Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void SYSCONIntHandler(void) 
{
    // ISR content ...

}
/*************************************************************/
//IFC Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void IFCIntHandler(void) 
{
    // ISR content ...

}

/*************************************************************/
//ADC Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void ADCIntHandler(void) 
{
    // ISR content ...

	
}


/*************************************************************/
//DMA Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void DMAIntHandler(void) 
{
	// ISR content ...
//	gpio_configure(GPIOB0, 4, PB4_CMPIN3N_CMPIN10P);
	
	DMA->ICR = 0X01;
}

/*************************************************************/
//WWDT Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void WWDTIntHandler(void)
{
	// ISR content ...
	wwdt_set_cnt(0xff);
	WWDT->ICR = WWDT_INT_EVI;
}

/*************************************************************/
//GPTA0 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/

void GPTA0IntHandler(void) 
{
    // ISR content ...

	
}

/*************************************************************/
//GPTB0 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void GPTB0IntHandler(void) 
{
    // ISR content ...

}


/*************************************************************/
//GPTB1 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void GPTB1IntHandler(void) 
{
    // ISR content ...

}

/*************************************************************/
//GPTB2 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void GPTB2IntHandler(void) 
{
    // ISR content ...

}
/*************************************************************/
//RTC Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void RTCIntHandler(void) 
{
    // ISR content ...
	
}

/*************************************************************/
//FRT Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void FRTIntHandler(void) 
{
    // ISR content ...
	
}

/*************************************************************/
//UART0 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/

void UART0IntHandler(void) 
{
    // ISR content ...

}


/*************************************************************/
//UART1 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void UART1IntHandler(void) 
{
       // ISR content ...

}

/*************************************************************/
//UART2 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void UART2IntHandler(void) 
{
    // ISR content ...
	
}
/*************************************************************/
//SIO Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void SIOIntHandler(void) 
{
    // ISR content ...
	SIO0->ICR = SIO_INT_ALL;
}

/*************************************************************/
//I2C Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void I2CIntHandler(void) 
{
    // ISR content ...

	
}
/*************************************************************/
//SPI Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void SPI0IntHandler(void) 
{
	//ISR content ...


}

/*************************************************************/
//EXT0/16 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void EXI0IntHandler(void) 
{
	// ISR content ...可以根据应用需要修改
	if ((SYSCON->EXIRS & (0x1 << EXI_GRP0)) == 0x1 << EXI_GRP0) 			//EXT0 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP0;
		
	}	
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP16)) == 0x1 << EXI_GRP16) 		//EXT16 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP16;
		
	}
}
/*************************************************************/
//EXT1/17 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void EXI1IntHandler(void) 
{
	//ISR content ...
	if ((SYSCON->EXIRS & (0x1 << EXI_GRP1)) == 0x1 << EXI_GRP1) 			//EXT1 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP1;
		gpio_reverse(GPIOA0, 3);
	}	
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP17)) == 0x1 << EXI_GRP17) 		//EXT17 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP17;
		
	}

}
/*************************************************************/
//EXI2~3 18~19Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void EXI2to3IntHandler(void) 
{
	// ISR content ...可以根据应用需要修改
	if ((SYSCON->EXIRS & (0x1 << EXI_GRP2)) == 0x1 << EXI_GRP2) 			//EXT2 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP2;
		
	} 
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP3)) == 0x1 << EXI_GRP3) 		//EXT3 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP3;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP18)) == 0x1 << EXI_GRP18) 		//EXT18 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP18;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP19)) == 0x1 << EXI_GRP19) 		//EXT19 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP19; 
	}
}
/*************************************************************/
//EXI4~9 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void EXI4to9IntHandler(void) 
{
	// ISR content ...可以根据应用需要修改
	if ((SYSCON->EXIRS & (0x1 << EXI_GRP4)) == 0x1 << EXI_GRP4) 			//EXT4 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP4;
		
	} 
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP5)) == 0x1 << EXI_GRP5) 		//EXT5 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP5;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP6)) == 0x1 << EXI_GRP6) 		//EXT6 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP6;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP7)) == 0x1 << EXI_GRP7) 		//EXT7 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP7; 
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP8)) == 0x1 << EXI_GRP8) 		//EXT8 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP8;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP9)) == 0x1 << EXI_GRP9) 		//EXT9 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP9;
		
	}

}
/*************************************************************/
//EXI4 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void EXI10to15IntHandler(void) 
{
	// ISR content ...可以根据应用需要修改
	if ((SYSCON->EXIRS & (0x1 << EXI_GRP10)) == 0x1 << EXI_GRP10) 			//EXT10 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP10;
		
	} 
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP11)) == 0x1 << EXI_GRP11) 		//EXT11 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP11;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP12)) == 0x1 << EXI_GRP12) 		//EXT12 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP12;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP13)) == 0x1 << EXI_GRP13) 		//EXT13 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP13;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP14)) == 0x1 << EXI_GRP14) 		//EXT14 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP14;
		
	}
	else if ((SYSCON->EXIRS & (0x1 << EXI_GRP15)) == 0x1 << EXI_GRP15) 		//EXT15 Interrupt
	{
		SYSCON->EXICR = 0x1 << EXI_GRP15;
	
	}
}


/*************************************************************/
//CMP0 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void CMP0IntHandler(void) 
{
    // ISR content ...
}

/*************************************************************/
//CMP1 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void CMP1IntHandler(void) 
{
    // ISR content ...
	
}

/*************************************************************/
//BT0 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
extern volatile uint8_t byTimer_100us;
void BT0IntHandler(void) 
{
	// ISR content ...
	byTimer_100us ++;
	
	BT0->ICR = BT_INT_ALL;
}
/*************************************************************/
//BT1 Interrupt
//EntryParameter:NONE
//ReturnValue:NONE
/*************************************************************/
void BT1IntHandler(void) 
{
    // ISR content ...
	BT1->ICR = BT_INT_ALL;
}

/*************************************************************/
/* system abnormal Handler
*************************************************************/
void PriviledgeVioHandler(void) 
{
    // ISR content ...

}

void SystemDesPtr(void) 
{
    // ISR content ...

}

void MisalignedHandler(void) 
{
    // ISR content ...

}

void IllegalInstrHandler(void) 
{
    // ISR content ...

}

void AccessErrHandler(void) 
{
    // ISR content ...

}

void BreakPointHandler(void) 
{
    // ISR content ...
}

void UnrecExecpHandler(void) 
{
    // ISR content ...

}

void Trap0Handler(void) 
{
    // ISR content ...

}

void Trap1Handler(void) 
{
    // ISR content ...

}

void Trap2Handler(void) 
{
    // ISR content ...

}

void Trap3Handler(void) 
{
    // ISR content ...

}

void PendTrapHandler(void) 
{
    // ISR content ...

}
/******************* (C) COPYRIGHT 2024 APT Chip *****END OF FILE****/

