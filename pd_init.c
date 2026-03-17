#include "pd_init.h"
//=====================================================================================
//=====================================================================================
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_cc_recv_deploy(void)
{
	gpio_configure(GPIOB0, 4, PB4_CMPIN3N_CMPIN10P);//CC1
	gpio_configure(GPIOB0, 5, PB5_CMPIN4P);//CC2
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_cc_deploy_deinit(void)
{
	gpio_configure(GPIOB0, 4, PIN_GPD);//CC1
	gpio_configure(GPIOB0, 5, PIN_GPD);//CC2
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_cc_send_deploy(uint8_t	bySendPin)
{
	if(bySendPin == eCC1)
		gpio_configure(GPIOB0, 4, SIO);//CC1
	else
		gpio_configure(GPIOB0, 5, SIO);//CC2
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_cmp_int_deploy(csp_cmp_t *ptCmpBase,functional_status_e eEnable)
{//CMP中断配置
	if (eEnable == ENABLE)
		cmp_int_enable(ptCmpBase,CMP_INT_EDGEDET);
	else
		cmp_int_disable(ptCmpBase,CMP_INT_EDGEDET);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_cmp_init(void)
{
	//gpio_configure(GPIOA0, 12, CMP1_OUT);
	
	actrl_set_intvref(ACTRL_INTVREF_LVL_1V5,ACTRL_INTVREF_EN);				//INTVREF电压选择1.5V
	//N端使用INTVREF 1.5V/16*9, P端输入来自PB4
	cmp_software_reset(CMP0);
	cmp_set_vref(CMP0,CMP_REFSEL_INTVREF);									//CMP参考电压源选择INTVREF
	cmp_configure(CMP0, PHYST_0MV, NHYST_0MV, CMP_CPO_DF_OUT, CMPO_DIRECT);
	cmp_input_configure(CMP0,(U32_T)CMPINREF6,(U32_T)CMPIN10P_CMPIN3N);
	CMP0->INPCR |= 0X03 << 28;
	cmp_edgedet_pol_configure(CMP0,CMP_EDGEDET_POL_BOTH);
	cmp_dflt_configure(CMP0,DF1_DEPTH_8,0,1);   //72M / 2^ DIVN/ (DIVM+1)=40ns*8
	cmp_flt_enable(CMP0,CMP_DF1);
	cmp_enable(CMP0);
	
	cmp_software_reset(CMP1);
	cmp_set_vref(CMP1,CMP_REFSEL_INTVREF);									//CMP参考电压源选择INTVREF
	cmp_configure(CMP1, PHYST_0MV, NHYST_0MV, CMP_CPO_DF_OUT, CMPO_DIRECT);
	cmp_input_configure(CMP1,(U32_T)CMPINREF6,(U32_T)CMPIN4P);
	CMP1->INPCR |= 0X03 << 28;
	cmp_edgedet_pol_configure(CMP1,CMP_EDGEDET_POL_BOTH);
	cmp_dflt_configure(CMP1,DF1_DEPTH_8,0,1);    //72M / 2^ DIVN/ (DIVM+1)=40ns*8
	cmp_flt_enable(CMP1,CMP_DF1);
	cmp_enable(CMP1);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_sio_recv_init(void)
{
	sio_recv_ready();				//复位SIO的同时先把SIO模式切换至接收，否则会出现异常
	pd_cc_recv_deploy();			//配置引脚
	
	sio_software_reset(SIO0);  	  	//SIO软复位
	etcb_deinit();					//ETCB软复位
	dma_software_reset(DMA);		//DMA软复位
	//
	cmp_edgedet_pol_configure(CMP0,CMP_EDGEDET_POL_F);
	cmp_edgedet_pol_configure(CMP1,CMP_EDGEDET_POL_F);
	//接收采样配置
	sio_rx_samp_configure(SIO0, 
							SIO_BST_FALL, //采样触发边沿为：双沿
							SIO_TRG_MD0,//采样触发模式选择：选择30ns滤波后采样信号。
							7, //采样的长度，即一个BIT数据由7+1个采样决定。
							SIO_EXTRACT_BIT5, //接收到的原始数据的1的个数大于(HITHR=5)时提取为H，否则提取为L
							5, //HITHR=5
							SIO_ALIGN_EN, //采样对齐关闭
							SIO_RDIR_MSB, //接收数据方向：MSB->LSB
							SIO_RMODE1); //在当前bit采样结束后, 立即开始采样下一个数据
	//接收配置
	sio_rx_configure(SIO0,0X03,SIO_BREAKUPD_SHIFT,SIO_UPDRXBUF_FROM_SHIFT,31,1599);
			
	sio_rx_clk_configure(SIO0, 14); //FrXCLK = FPCLK / (TCKPRS +1)=PCLK
	sio_set_break_rst(SIO0,SIO_BREAKLVL_HIGH,5,SIO_BREAKPRS_BITLEN);
	//开中断准备接收数据
	if(tPD.C_COM.byScource == eCC1)
		pd_cmp_int_deploy(CMP0,ENABLE);
	else
		pd_cmp_int_deploy(CMP1,ENABLE);
	
	sio_rxdma_enable(SIO0);//使能 RX DMA 
	
	//ETCB配置
	etcb_chxconb_configure(ET_CH6, ENABLE, TRG_HW,ET_DMA_EN, ET_SIO_TRGOUT_RX, ET_DMA_SYNCIN0);
	etcb_chxconb_configure(ET_CH7, ENABLE, TRG_HW,ET_DMA_EN, ET_SIO_TRGOUT_RX, ET_DMA_SYNCIN1);
	etcb_enable();
	
	//DMA配置
	dma_ch_set_srcaddr(DMA,DMA_CH0,(U32_T)(&SIO0->RXBMC),DMA_HINC_CONST,DMA_LINC_CONST);
	dma_ch_set_destaddr(DMA,DMA_CH0,(U32_T)(&wPD_BMC_Recv[0]),DMA_HINC_INC,DMA_LINC_CONST);
	dma_ch_configure(DMA,DMA_CH0,50,1,DMA_DSIZE_HW,DMA_TSIZE_NOR ,DMA_RELOAD_DIS, DMA_SMODE_ONCE , DMA_REQ_HW);
	
	dma_ch_set_srcaddr(DMA,DMA_CH1,(U32_T)(&SIO0->RXBUF),DMA_HINC_CONST,DMA_LINC_CONST);
	dma_ch_set_destaddr(DMA,DMA_CH1,(U32_T)(&wPD_Buf_Recv),DMA_HINC_INC,DMA_LINC_CONST);
	dma_ch_configure(DMA,DMA_CH1,50,1,DMA_DSIZE_W,DMA_TSIZE_NOR ,DMA_RELOAD_DIS, DMA_SMODE_ONCE , DMA_REQ_HW);
	
	pd_dma_start(DMA_CH0);
	pd_dma_start(DMA_CH1);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_sio_send_init(uint8_t* wSrcAdd,uint16_t hwTxCnt)
{
	uint16_t 	hwTxLenTemp;
	uint8_t		byTxLenTemp;
	hwTxLenTemp = hwTxCnt / 5;				//整数
	byTxLenTemp = hwTxCnt % 5;				//余数
	if(byTxLenTemp)
		hwTxLenTemp ++;
	
	pd_cc_send_deploy(tPD.C_COM.byScource);	//配置SIO的输出管脚
	pd_cmp_int_deploy(CMP0,DISABLE);
	pd_cmp_int_deploy(CMP1,DISABLE);

	sio_software_reset(SIO0);  	  			//软件复位
	sio_tx_clk_configure(SIO0,58); 			//FTXCLK = FPCLK / (TCKPRS +1)
	
	sio_tx_configure(SIO0,
					 SIO_IDLEST_Z,  //idle 状态为高电平
					 SIO_TDIR_MSB, 	//发送数据是从最低位发出的
					 4,     		//TXBUF长度为（7+1）
					 hwTxCnt - 1,			//发送序列长度（35+1）
					 3,     		//D0状态时间为：(7+1) x Ttxshft
					 3,				//D1状态时间为：(7+1) x Ttxshft
					 SIO_DHL_BIT4,  //DH对象序列长度为(7+1)it
					 SIO_DHL_BIT4,  //DL对象序列长度为(7+1)it
					 0x03,       	//DH对象序列： 4b'1100
					 0x0c       	//DL对象序列： 4b'0011
					 );
	sio_txdma_enable(SIO0);//使能 TX DMA  
	etcb_chxconb_configure(ET_CH6, ENABLE, TRG_HW,ET_DMA_EN, ET_SIO_TRGOUT_TX, ET_DMA_SYNCIN0);
	etcb_enable();
	
	dma_software_reset(DMA);
	dma_ch_set_srcaddr(DMA,DMA_CH0,(uint32_t)wSrcAdd,DMA_HINC_INC,DMA_LINC_CONST);
	dma_ch_set_destaddr(DMA,DMA_CH0,(uint32_t)(&(SIO0->TXBUF)),DMA_HINC_CONST,DMA_LINC_CONST);
	dma_ch_configure(DMA,DMA_CH0,hwTxLenTemp,1,DMA_DSIZE_W,DMA_TSIZE_NOR ,DMA_RELOAD_DIS, DMA_SMODE_ONCE, DMA_REQ_HW);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_protocol_sink_init(void)
{
	pd_cc_recv_deploy();						//配置CC引脚引脚
	pd_cmp_init();								//初始化配置CMP
	//
	if(pd_judge_com_gpio() == eRequest_ComErr)	//未正确接入CC通讯引脚
	{
		//COM口未接
		pd_cc_recv_deploy();
		
		pd_cmp_int_deploy(CMP0,ENABLE);
		pd_cmp_int_deploy(CMP1,ENABLE);
	}	
	//
	pd_crc32_init();
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_protocol_sink_deinit(void)
{
	pd_cc_deploy_deinit();		//端口设为高阻态
	cmp_software_reset(CMP0);	//软复位CMP0
	cmp_software_reset(CMP1);	//软复位CMP1
	//
	sio_software_reset(SIO0);	//软复位SIO
	dma_software_reset(DMA);	//软复位DMA
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_crc_set_seed(uint32_t wSeedVal)
{
	crc_set_seed(wSeedVal);		//写入CRC种子值
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_crc32_init(void)
{
	crc_enable();                           //CRC模块时钟使能
	crc_configure(CRC_DISABLE,CRC_ENABLE,CRC_ENABLE,CRC_ENABLE,POLY_32);//crc模式配置：配置为crc32
	pd_crc_set_seed(0xffffffff); 			//设置种子为0xffffffff
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void disable_global_interrupt(void)
{
	CK_CPU_DISALLNORMALIRQ;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void enable_global_interrupt(void)
{
	CK_CPU_ENALLNORMALIRQ;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint32_t get_crc_result(void)
{
	return CRC->DATAOUT;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void sio_wait_break_rst(void)
{
	volatile uint32_t	wBreak_rst_Cnt = 40950;
//	sio_set_break_rst(SIO0,SIO_BREAKLVL_HIGH,5,SIO_BREAKPRS_BITLEN);
	while(!(SIO0->RISR & 0X10))//等待接收完成
	{
		if(--wBreak_rst_Cnt == 0)
			break;
	}
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_send_start(void)
{
	dma_hwtrig_start(DMA,DMA_CH0);
	while(!(SIO0->RISR & 0X01));	//等待发送中断
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint8_t	sio_wait_status_busy(void)
{
	uint8_t		byStatus_Sio_busy = TRUE;
	uint16_t	hwDelay_wait_Sio_busy = 40950;
	while(!(SIO0->STATUS & 0x01))//
	{
		if(--hwDelay_wait_Sio_busy == 0)
		{
			byStatus_Sio_busy = FALSE;
			break;
		}
	}
	return byStatus_Sio_busy;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void sio_send_ready(void)
{//SIO配置发送前准备
	sio_software_reset(SIO0);
	SIO0->CR = 0X0000B000;
	SIO0->RXCR2 &= ~0x01;//关闭复位检测
	SIO0->ICR |= 0x1F;//清标志位
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void sio_recv_ready(void)
{
	SIO0->CR = 0X0000B100;		//复位SIO的同时先把SIO模式切换至接收，否则会出现异常
	SIO0->TXCR0 = 0X00;
	SIO0->TXCR1 = 0X00;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_recv_module_reset(void)
{//执行软件复位后重新配置进入接收模式
	sio_software_reset(SIO0);  	 //软件复位
	etcb_deinit();
	dma_software_reset(DMA);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint32_t get_cmp0_out_status(void)
{//CMP0输出状态
	return (!(CMP0->CR & 0X01 << 24));//CC1引脚状态
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint32_t get_cmp1_out_status(void)
{//CMP1输出状态
	return (!(CMP1->CR & 0X01 << 25));//CC2引脚状态
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_dma_start(dma_ch_e eDmaCh)
{
	dma_hwtrig_start(DMA,eDmaCh);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
volatile uint32_t pd_crc_calculation(volatile uint8_t* ptData, uint16_t wNum)
{
	uint32_t i;
	for (i=0; i<wNum; i++)
	{
		*(uint8_t *)(AHB_CRCBASE + 0x14 + (i%4)) = *ptData;
		ptData++;
	}
	return get_crc_result();
}
















