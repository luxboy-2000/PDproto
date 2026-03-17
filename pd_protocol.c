#include "pd_protocol.h"
//=====================================================================================
//=====================================================================================
volatile Pd_protocol_t	tPD = 
{
.Power.byType = 0,			//共N种电源可选
.C_COM.byScource = 0,		//默认CC1
.hwGetVoltage = 5000,		//默认5V
.hwGetCurrent = 500,		//默认0.5A
.Power.hwRecordVoltage = 5000,
.Power.hwRecordCurrent = 500,
.hwNowVoltage = 0,
.hwNowCurrent = 0,
.byAllowLeftOffset = 5,
.byAllowRightOffset = 5,
//.Power.byGetType = ePower_Type_PDO,
.Status.bAllowRequest = FALSE,
};
//=====================================================================================
volatile uint8_t	byGetReceive5BData[5];
volatile uint32_t 	wPD_BMC_Recv[50];
volatile uint32_t 	wPD_Buf_Recv;		//此BUF只用来读取SIO数据,保证BMC数据正确，无其他用途

volatile uint32_t	wPD_crc;		//CRC计算用
volatile uint32_t	wGetReceiveCRC;
volatile uint32_t 	wComputeCRC;
volatile uint8_t	byReceiveCRC_Cnt;
volatile uint8_t	MessageID = 0;

//-------------------------------------------------------------------------------------
const uint8_t		switch_4bTo5b_HexTable_Recv[16] = {0x0F,0X12,0X05,0X15,0X0A,0X1A,0X0E,0X1E,0X09,0X19,0X0D,0X1D,0X0B,0X1B,0X07,0X17};//{0X1E,0X09,0X14,0X15,0X0A,0X0B,0X0E,0X0F,0X12,0X13,0X16,0X17,0X1A,0X1B,0X1C,0X1D};
const uint8_t		switch_4bTo5b_HexTable_Send[16] = {0x1E,0X09,0X14,0X15,0X0A,0X0B,0X0E,0X0F,0X12,0X13,0X16,0X17,0X1A,0X1B,0X1C,0X1D};
//数据对称，如原'0'='0B11110'变为‘0b01111’
/*
Sync-1   	:00011		//11000
Sync-2   	:10001		//10001
Sync-3   	:01100		//00110
RST-1   	:11100		//00111
RST-2   	:10011		//11001

SOP 			Sync-1 Sync-1 Sync-1 Sync-2
SOP’ 			Sync-1 Sync-1 Sync-3 Sync-3
SOP'’ 			Sync-1 Sync-3 Sync-1 Sync-3

HARD RESET 		RST-1, RST-1, RST-1, RST-2
Cable Reset 	RST-1, Sync-1, RST-1, Sync-3
*/
#define SOP_PATTERN_Location			60
#define SOP_Adder_Deviation_Max			15		//允许查询的最大偏差，5b数据,15bit对应4b数据则为12bit
#define Wait_cmp_nbit					3
//#define Complete_judgment
#ifdef 	Complete_judgment
#define SOP_PATTERN						0X18C71	//SOP
#define SOP_PRIME_PATTERN				0X18F39	//SOP'
#define SOP_DBLPRIME_PATTERN			0X1E479	//SOP''
#define HARDRESET_PATTERN				0XE7393	//HEAD RESET
#define CABLERESET_PATTERN				0XE0F8C	//CABLE RESET
#define SOP_MATCH_THRESHOLD 			0		//允许容错
#else
#define SOP_PATTERN						0x03
#define RESET_PATTERN					0X1C
#define SOP_MATCH_THRESHOLD 			0		//允许容错
#endif

volatile int16_t	iRecord_location;				//记录寻址位置
//=====================================================================================
volatile uint8_t 	byRequestData[25] = {
0x18,0x18,0x18,0x11,//SOP
0x14,0x0A,0x1E,0x09,//Header
0x12,0x1A,0x1E,0x14,0x15,0x1E,0x09,0x15,//DataMessage
0x14,0x1B,0x1C,0x0B,0x14,0x0A,0x1D,0x0F,0x0D};//CRC+EOP
volatile uint8_t 	byGOODCRC[17] ={
0x18,0x18,0x18,0x11,//SOP
0x09,0x12,0x0A,0x1E,//Header
0x14,0x16,0x12,0x16,0x0E,0x1B,0x1D,0x16,0x0D};//CRC+EOP
volatile uint8_t 	byGet_Source_Cap[17] = {
0x18,0x18,0x18,0x11,//SOP
0x09,0x0A,0x0A,0x1E,//Header
0x14,0x16,0x12,0x16,0x0E,0x1B,0x1D,0x16,0x0D};//CRC+EOP
//发送数据，每个参数只有高10bit有效，组成5bit数据对外发送
//高位先发，高位对齐
volatile uint32_t	wPD_Data_Send[39] = {
0x0d800000,0x36000000,0xD8C00000,0x63400000,0x8D800000,0x36000000,0xD8C00000,0x63400000,0x8D800000,0x36000000,0xD8C00000,0x63400000,0x8D800000,//Preamble
0x13C00000,0x46800000,0x13C00000,0xD1800000,//SOP
0x18C00000,0x63400000,0x3FC00000,0xD3400000,//Header
0x34C00000,0x63C00000,0x6A800000,0x18C00000,0xD8C00000,0x6A800000,0x86000000,0x4D800000,//DataMessage
0x18C00000,0xF6800000,0x1A800000,0xA3400000,0x18C00000,0x63400000,0x8FC00000,0xFF400000,0x8F400000,0x15400000};//CRC+EOP
//=====================================================================================
//5b转bmc码
__attribute__ (( section (".codeinram"))) void 	switch_5bitToBmc(volatile uint8_t *input, uint16_t length,volatile uint32_t *output);
//提取指定位置的指定长度数据，最大长度不超过48bit
int64_t 	extract_received_data(volatile uint32_t* data,uint16_t byStartAddr,uint8_t byData_len);	
//提取跨16位边界的n位数据片段
__attribute__ (( section (".codeinram"))) uint32_t extract_nbit_slice(uint32_t word1,uint32_t word2,uint32_t word3, uint8_t offset,uint8_t length);
//查找SOP或Reset位置，若返回-1则未找到SOP或Reset，返回-2则找到Reset,返回大于等于0的数据则为SOP的位置
__attribute__ (( section (".codeinram"))) int16_t 	detect_sop_position(volatile uint32_t* data,uint16_t hwStatusAddr,uint16_t hwOverAddr,uint8_t byData_len);
//5b转4b
uint8_t 	switch_5bitTo4bit(const uint8_t* byArray,volatile uint8_t* byTarget);
//提取DataMessage数据，输入提取到的数组地址，提取数据起始地址，提取Data数据个数
void 		get_recv_data(volatile uint32_t *output,volatile uint16_t hwStatusAddr,volatile uint8_t byDataNum);
//用于USB-PD解码
void 		pd_sink_receive_decode(void);
//重新取电运行函数
uint8_t 	pd_sink_again_RequestPower(void);
//type c拔出判断
void 		pd_pull_out_judge(void);
//type c插入后处理
void 		pd_insert(void);
//REQUEST处理函数
void pd_request_test(void);
//接收时等待3个上升沿之后开始采集数据
__attribute__ (( section (".codeinram"))) void wait_cmp_nbit_data(void);
//=====================================================================================
/** \brief		:确认pd接入，开始跑PD协议
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_insert(void)
{
	pd_sio_recv_init();
	
	tPD.C_COM.byCC_Pin_Cnt = 0x00;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_sink_receive_decode(void)
{
	disable_global_interrupt();				//禁止全局中断
	if(sio_wait_status_busy() == FALSE)
	{
		pd_sio_recv_init();
		enable_global_interrupt();			//使能全局中断
		return;
	}
	sio_wait_break_rst();					//等待break中断
	sio_send_ready();						//复位SIO状态等待准备下一轮发送或接收
	//判断SOP码
	uint8_t	byAllowLeftOffset,byAllowRightOffset;
	if(tPD.byAllowLeftOffset < SOP_Adder_Deviation_Max)
		byAllowLeftOffset = SOP_PATTERN_Location - tPD.byAllowLeftOffset;//在允许查询的最大偏差范围内
	else
		byAllowLeftOffset = SOP_PATTERN_Location - SOP_Adder_Deviation_Max;//超过允许查询的最大偏差范围，以允许的最大偏差为准
	if(tPD.byAllowRightOffset < SOP_Adder_Deviation_Max)
		byAllowRightOffset = SOP_PATTERN_Location + tPD.byAllowRightOffset;//在允许查询的最大偏差范围内
	else
		byAllowRightOffset = SOP_PATTERN_Location + SOP_Adder_Deviation_Max;//超过允许查询的最大偏差范围，以允许的最大偏差为准
	//查询SOP位置
	iRecord_location = detect_sop_position((volatile uint32_t*)&wPD_BMC_Recv,byAllowLeftOffset,byAllowRightOffset,5);//28.1us
	
	if(iRecord_location < 0)
	{
		//未找到SOP码或找到Reset码，数据无效，重新接收数据	
		pd_insert();
		tPD.byStatus = eSource_NotSOP;
		enable_global_interrupt();		
		return;
	}
	volatile int32_t 		lGetData;					//用于读取数据
	volatile uint8_t		byTemp[4];
	iRecord_location += 20;
	//
	lGetData = extract_received_data((volatile uint32_t*)&wPD_BMC_Recv,iRecord_location,20);//提取对象，提取地址，提取长度
	//
	if(lGetData < 0)
	{
		//提取数据无效
		pd_sio_recv_init();
		enable_global_interrupt();
		return;
	}
	else
	{
		byGetReceive5BData[0] = (lGetData >> 15) & 0x1F;
		byGetReceive5BData[1] = (lGetData >> 10) & 0x1F;
		byGetReceive5BData[2] = (lGetData >> 5) & 0x1F;
		byGetReceive5BData[3] = lGetData & 0x1F;
		
		//回读Source-Header信息
//		tPD.Source.Information.AllBits = 0;//Header信息
		byTemp[0] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byGetReceive5BData[0]);
		byTemp[1] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byGetReceive5BData[1]);
		byTemp[2] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byGetReceive5BData[2]);
		byTemp[3] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byGetReceive5BData[3]);
		tPD.Source.Information.AllBits = byTemp[3] << 12 | byTemp[2] << 8 | byTemp[1] << 4 | byTemp[0];
//		tPD.Source.Information.AllBits = tPD.Source.Information.AllBits & 0xfeff;
	}
	//
	if(tPD.Source.Information.byMessageType == ePD_GOODCRC && tPD.Source.Information.byDataLength == 0x00)//接收到的是source发来的回复Sink——request的GOODCRC，无需处理
	{
		tPD.Source.Information.byMessageID ++;
	}
	else if(tPD.Source.Information.byPortDataRole == ePD_UFP)//接收到的是UFP-SOP'  Vnedor define，无需处理(Source默认未DFP，Sink默认未UFP)
	{
	}
	else 
	{
		byGOODCRC[6] = switch_4bTo5b_HexTable_Send[tPD.Source.Information.byMessageID << 1];

		tPD.Send_CRC.hwHeader = 0X00;
		byTemp[0] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byGOODCRC[4]);
		byTemp[1] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byGOODCRC[5]);
		byTemp[2] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byGOODCRC[6]);
		byTemp[3] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byGOODCRC[7]);
		tPD.Send_CRC.hwHeader |= byTemp[3] << 12 | byTemp[2] << 8 | byTemp[1] << 4 | byTemp[0];

		pd_sio_send_init((uint8_t*)(&wPD_Data_Send),155);//配置发送内容以及发送长度

		pd_crc_set_seed(0xffffffff); //设置种子为0xffffffff
		wPD_crc = pd_crc_calculation((volatile uint8_t*)&tPD.Send_CRC.hwHeader,2);
		
		for(int i = 0;i < 8;i++)
			byGOODCRC[8+i] = switch_4bTo5b_HexTable_Send[(wPD_crc>>(4*i))&0x0000000f];
		//40.75us(SRAM)//89us(ROM)
		switch_5bitToBmc(byGOODCRC,17,&wPD_Data_Send[13]);
		if((wPD_Data_Send[29] & 0x00C00000 )== 0x00400000)
			wPD_Data_Send[30] = 0x15400000;
		else
			wPD_Data_Send[30] = 0x45400000;
		pd_send_start();				//数据开始发送
		//回复数据后将端口切换为高阻态释放端口
		pd_cc_deploy_deinit();
		//判断接收到的数据内容
		if(tPD.Source.Information.byDataLength > 1)
		{
			iRecord_location += 20;					//取数据地址
			tPD.byStatus = eSource_Broadcast;		//Source电源广播
			get_recv_data((volatile uint32_t*)&tPD.Source.wTypeData,iRecord_location,tPD.Source.Information.byDataLength);
			//回读数据结束
			//回读适配器供电能力
			tPD.Power.byType = tPD.Source.Information.byDataLength;		//电源选择
			tPD.Source.Information.byDataLength = 0;
			volatile uint32_t wTemp;
			for (uint8_t k = 0; k < tPD.Power.byType;k ++) 
			{
				wTemp = tPD.Source.wTypeData[k] & 0xc0000000;
				wTemp = wTemp >> 30;
				#ifdef PPS_Request_ENABLE
				switch(wTemp)
				{
					case 0x00://固定电压
						tPD.Power.hwVoltage_Max[k] = ((tPD.Source.wTypeData[k] & 0xffc00) >> 10) * 50;
						tPD.Power.hwVoltage_Min[k] = tPD.Power.hwVoltage_Max[k];
						tPD.Power.hwCurrent[k] = (tPD.Source.wTypeData[k] & 0x3ff) * 10;
					break;
					case 0X01://电池类型
					break;
					case 0X02://可变供应(非电池)
					break;
					case 0x03://增强型可编程电源(APDO)					
						tPD.Power.hwVoltage_Max[k] = ((tPD.Source.wTypeData[k] & 0x1fe0000) >> 17) * 100;
						tPD.Power.hwVoltage_Min[k] = ((tPD.Source.wTypeData[k] & 0xff00) >> 8) * 100;
						tPD.Power.hwCurrent[k] = (tPD.Source.wTypeData[k] & 0x7f) * 5;
					break;
					default:
					break;
				}
				#else
				//若不为PDO电压，不进行数据存储，认为无效数据
				switch(wTemp)
				{
					case 0x00://固定电压
						tPD.Power.hwVoltage[k] = ((tPD.Source.wTypeData[k] & 0xffc00) >> 10) * 50;
						tPD.Power.hwCurrent[k] = (tPD.Source.wTypeData[k] & 0x3ff) * 10;
					break;
					default:
					break;
				}	
				#endif					
			}
			//判断获取电压电流，先遍历符合的电压、电流，然后取电，优先固定电压
			for(uint8_t j = 0;j < tPD.Power.byType; j ++)
			{//从获取的数据当中回读出电压、电流数据
				#ifdef PPS_Request_ENABLE
				if(tPD.Power.hwVoltage_Max[j] == tPD.Power.hwVoltage_Min[j] && tPD.Power.hwVoltage_Max[j] == tPD.hwGetVoltage)
				{
					tPD.Power.byGetlocation = j;
					
					if(tPD.Power.hwCurrent[j] >= tPD.hwGetCurrent)
					{
						tPD.Power.hwRecordVoltage = tPD.Power.hwVoltage_Max[j];
						tPD.Power.hwRecordCurrent = tPD.hwGetCurrent;	//获取所需电流
						
//						tPD.Power.byGetType = ePower_Type_PDO;
						break;
					}
					else
					{
						tPD.Power.hwRecordVoltage = tPD.Power.hwVoltage_Max[j];
						tPD.Power.hwRecordCurrent = tPD.Power.hwCurrent[j];	//获取最大电流
						
//						tPD.Power.byGetType = ePower_Type_PDO;
						break;
					}
				}
				#else
				if(tPD.Power.hwVoltage[j] == tPD.hwGetVoltage)
				{
					tPD.Power.byGetlocation = j;
					
					if(tPD.Power.hwCurrent[j] >= tPD.hwGetCurrent)
					{
						tPD.Power.hwRecordVoltage = tPD.Power.hwVoltage[j];
						tPD.Power.hwRecordCurrent = tPD.hwGetCurrent;			//获取所需电流
						
//						tPD.Power.byGetType = ePower_Type_PDO;
						break;
					}
					else
					{
						tPD.Power.hwRecordVoltage = tPD.Power.hwVoltage[j];
						tPD.Power.hwRecordCurrent = tPD.Power.hwCurrent[j];	//获取最大电流
						
//						tPD.Power.byGetType = ePower_Type_PDO;
						break;
					}
				}
				#endif
				
			}
			if (tPD.Power.byGetlocation >= tPD.Power.byType) 
			{//限制判断，若取电序列号超过实际存储数据序列，认为无效，取5V
				tPD.Power.byGetlocation = 0x00;//取电异常
				//
				#ifdef PPS_Request_ENABLE
				tPD.Power.hwRecordVoltage = tPD.Power.hwVoltage_Max[0];
				#else
				tPD.Power.hwRecordVoltage = tPD.Power.hwVoltage[0];
				#endif
				tPD.Power.hwRecordCurrent = tPD.Power.hwCurrent[0];	
			}
			pd_request_test();			//Sink完成取电
			tPD.hwNowVoltage = tPD.Power.hwRecordVoltage;
			tPD.hwNowCurrent = tPD.Power.hwRecordCurrent;
		}
		else
		{
			//非数据，记录消息内容
			if(tPD.Source.Information.byMessageType == ePD_Accept)			//Source同意了数据请求
				tPD.byStatus = eSource_Accept;
			else if(tPD.Source.Information.byMessageType == ePD_PS_RDY)		//Source已经准备好输出指定电压
				tPD.byStatus = eSource_Ready;
			else if(tPD.Source.Information.byMessageType == ePD_VCONN_Swap)	//Source进行数据协商
				tPD.byStatus = eSource_VCONN_Swap;
			else
				tPD.byStatus = eSource_Otcher_News;							//其他数据
			//收到Source端返回的有效信息，可以进行下一次重新取电
			tPD.Status.bAllowRequest = TRUE;
		}
	}
	pd_sio_recv_init();
	enable_global_interrupt();
}
void pd_request_test()
{
	
	//默认全部PDO取电
			//配置最大电流
			uint32_t	wTemp_Power;
			wTemp_Power = tPD.Power.hwCurrent[tPD.Power.byGetlocation] / 10;
			byRequestData[10] = 0;
			byRequestData[8] = wTemp_Power & 0x0f;
			byRequestData[9] = (wTemp_Power & 0xf0) >> 4;
			byRequestData[10] |= (wTemp_Power & 0x300) >> 8;
			//配置获取的电流
//			wTemp_Power = tPD.Power.hwRecordCurrent / 10;
			wTemp_Power = tPD.Power.hwCurrent[tPD.Power.byGetlocation] / 10;
			byRequestData[10] |= (wTemp_Power & 0x03) << 2;
			byRequestData[11] = (wTemp_Power & 0x3c) >> 2;
			byRequestData[12] = (wTemp_Power & 0x3c0) >> 6;
			//转换4b5b
			for (uint8_t k = 8; k < 13; k++) 
				byRequestData[k] = switch_4bTo5b_HexTable_Send[byRequestData[k]];
			//
			byRequestData[5] = 8;
			byRequestData[5] = switch_4bTo5b_HexTable_Send[byRequestData[5]] ;//版本号
			//if(tPD.Power.byGetlocation)
			//{
//				byRequestData[6] = switch_4bTo5b_HexTable_Send[(tPD.Power.byGetlocation)<<1];//写入MessageID
			//}
			//else        
			//	byRequestData[6] = switch_4bTo5b_HexTable_Send[tPD.Power.byGetlocation<<1];//写入MessageID	
			byRequestData[6] = switch_4bTo5b_HexTable_Send[MessageID<<1];//写入MessageID
			MessageID ++;
			if(MessageID > 7)
				MessageID = 0;
			byRequestData[15] = switch_4bTo5b_HexTable_Send[tPD.Power.byGetlocation + 1];	//写入或取的电压所在数据包的位置
			//Header转5bit数据 
			tPD.Send_CRC.hwHeader = 0x00;
			for(uint8_t j = 0;j < 4;j++)
				tPD.Send_CRC.hwHeader |= (switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byRequestData[4+j]) << (4*j));
			//MessageData转5bit数据
			tPD.Send_CRC.wMessage = 0x00;
			for(uint8_t j = 0;j < 8;j++)
				tPD.Send_CRC.wMessage |= (switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byRequestData[8+j]) << (4*j));
			//SIO配置
			pd_sio_send_init((uint8_t*)(&wPD_Data_Send),195);		//配置TX发送数据
			//CRC校验
			pd_crc_set_seed(0xffffffff); //设置种子为0xffffffff
			wPD_crc = pd_crc_calculation((volatile uint8_t*)&tPD.Send_CRC.hwHeader,2);
			wPD_crc = pd_crc_calculation((volatile uint8_t*)&tPD.Send_CRC.wMessage,4);
			//CRC过转5bit数据
			for(uint8_t j = 0;j < 8;j++)
				byRequestData[16+j] = switch_4bTo5b_HexTable_Send[(wPD_crc>>(4*j))&0x0000000f];
			//BMC编码
			switch_5bitToBmc(byRequestData,25,&wPD_Data_Send[13]);
			//确EOP编码
			if((wPD_Data_Send[37] & 0x00C00000 )== 0x00400000)
				wPD_Data_Send[38] = 0x15400000;
			else
				wPD_Data_Send[38] = 0x45400000;
			//开始传输数据
			pd_send_start();						//数据开始发送
			//数据传输完成，记录当前获取的电压/电流
			tPD.byStatus = eSink_Request;			//Sink完成取电
	
}
//=====================================================================================
/** \brief		:重新取电，需明确非第一次取电
 *  \param[in] 	:none
 *  \return 	:1：取电成功  0：取电失败
//===================================================================================*/
volatile uint16_t	hwGet_VBus,hwGet_Current;
uint8_t	pd_sink_RequestPower(PD_Sink_Request_e eSink_Voltage)
{
	uint8_t	byGainStatus = eRequest_Null;
	uint8_t byRequest_Data = eRequest_OK;

	//未准备好取电
	if(tPD.Status.bAllowRequest == FALSE)
		return FALSE;
	//数据记录当中无数据
	if(!tPD.Power.byType)
		return FALSE;
	//判断取电电压
	volatile uint16_t	hwSink_Voltage = 5000;
	switch(eSink_Voltage)
	{
		case e5V:hwSink_Voltage = 5000;break;
		case e9V:hwSink_Voltage = 9000;break;
		case e12V:hwSink_Voltage = 12000;break;
		case e15V:hwSink_Voltage = 15000;break;
		case e20V:hwSink_Voltage = 20000;break;
		default:byGainStatus = FALSE;break;
	}
	//没有符合的取电内容
	if(byGainStatus == FALSE)
		return FALSE;
	//赋值当前取电电压电流状态
	
	hwGet_VBus = tPD.hwNowVoltage;		
	hwGet_Current = tPD.hwNowCurrent;
	//查询取电电压、电流
	for(uint8_t i = 0; i < tPD.Power.byType; i ++) 
	{
		//
		if(tPD.Power.hwVoltage[i] == hwSink_Voltage)
		{
			hwGet_VBus = tPD.Power.hwVoltage[i];
			hwGet_Current = tPD.Power.hwCurrent[i];
			tPD.Power.byGetlocation = i;
		}
	}
	//---------------------------------------------------------------------
	if(tPD.hwNowVoltage != hwGet_VBus || tPD.hwNowCurrent != hwGet_Current)
	{
		//有新的取电状态
		tPD.hwGetVoltage = hwGet_VBus;
		tPD.hwGetCurrent = hwGet_Current;
		//开始取电			
		tPD.Status.bAllowRequest = FALSE;//修改可取电状态
//		uint8_t byRequest_Data = pd_sink_again_RequestPower();//与Source沟通重新取电
		disable_global_interrupt();	
		pd_request_test();
		enable_global_interrupt();				//CK_CPU_ENALLNORMALIRQ;
		pd_insert();							//配置为接收
		tPD.hwTimer10ms = 0;					//长时间无法取电判断
			
		while(tPD.hwTimer10ms < 100)
		{
			if(tPD.byStatus == eSource_Accept)
			{
				byRequest_Data = eRequest_OK;
				tPD.hwNowVoltage = tPD.Power.hwVoltage[tPD.Power.byGetlocation];
				tPD.hwNowCurrent = tPD.Power.hwCurrent[tPD.Power.byGetlocation];
				break;
			}
			else
				byRequest_Data = eRequest_TimeOut;
		}
		
		if(byRequest_Data == eRequest_OK)
		{//Source返回同意数据请求
			if(tPD.hwGetVoltage != tPD.hwNowVoltage)
				byGainStatus = FALSE;//获取电压与当前取电电压一致，取电成功
			else
				byGainStatus = TRUE;//获取电压与当前取电电压不一致，取电失败
		}
		else
			byGainStatus = FALSE;//Source回复数据超时，取电失败
	}
	else
		byGainStatus = TRUE;		//如果取电状态与当前输出状态一致，认为取电成功直接退出
	
	return byGainStatus;
}
//=====================================================================================
/** \brief		:pd_sink_again_RequestPower
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint8_t pd_sink_again_RequestPower(void)
{
	volatile 	uint8_t		byRequestStatus = eRequest_OK;
	
	disable_global_interrupt();												//禁止中断
	//构造Get_Source_Cap数据包
	byGet_Source_Cap[7] = switch_4bTo5b_HexTable_Send[0x00];				//无拓展，data数据量0
	byGet_Source_Cap[6] = switch_4bTo5b_HexTable_Send[tPD.Source.Information.byMessageID << 1];	//ID号
	byGet_Source_Cap[5] = switch_4bTo5b_HexTable_Send[ePD_Version];			//
	byGet_Source_Cap[4] = switch_4bTo5b_HexTable_Send[ePD_Get_Source_Cap];	//Get_Source_Cap固定为0X07([4 ... 0] = 0X07)

	tPD.Send_CRC.hwHeader = 0x00;
	for(int j = 0;j < 4;j++)
		tPD.Send_CRC.hwHeader |= (switch_5bitTo4bit(switch_4bTo5b_HexTable_Send,&byGet_Source_Cap[4+j]) << (4*j));
	//
	pd_sio_send_init((uint8_t*)(&wPD_Data_Send),155);		//配置TX发送数据
	pd_crc_set_seed(0xffffffff); //设置种子为0xffffffff
	wPD_crc = pd_crc_calculation((volatile uint8_t*)&tPD.Send_CRC.hwHeader,2);

	for(int i = 0;i < 8;i++)
		byGet_Source_Cap[8+i] = switch_4bTo5b_HexTable_Send[(wPD_crc>>(4*i))&0x0000000f];
	
	switch_5bitToBmc(byGet_Source_Cap,17,&wPD_Data_Send[13]);
	
	if((wPD_Data_Send[29] & 0x00C00000 )== 0x00400000)
		wPD_Data_Send[30] = 0x15400000;
	else
		wPD_Data_Send[30] = 0x45400000;
	
	pd_send_start();						//数据开始发送
	
	enable_global_interrupt();				//CK_CPU_ENALLNORMALIRQ;
	pd_insert();							//配置为接收
	tPD.hwTimer10ms = 0;					//长时间无法取电判断
	
	while(tPD.hwTimer10ms < 20)
	{
		if(tPD.byStatus == eSource_Accept)
		{
			byRequestStatus = eRequest_OK;
			break;
		}
		else
			byRequestStatus = eRequest_TimeOut;
	}
	
	return byRequestStatus;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint8_t pd_judge_com_gpio(void)
{
	volatile uint8_t byRequestStatus = eRequest_OK;
	
	pd_cc_recv_deploy();
	
	delay_nus(5);
	for(;;)
	{
		tPD.C_COM.byGet_CC_Status = 0;
		if(!get_cmp0_out_status())		//判断CMP0输入状态
			tPD.C_COM.byGet_CC_Status |= 0x01;
		if(!get_cmp1_out_status())		//判断CMP1输入状态
			tPD.C_COM.byGet_CC_Status |= 0x02;
		
		if(tPD.C_COM.byGet_CC_Status != tPD.C_COM.byGet_CC_Status_bk)
		{//CMP状态与上一次状态不一致
			tPD.C_COM.byCC_Pin_Cnt = 0;
			tPD.C_COM.byGet_CC_Status_bk = tPD.C_COM.byGet_CC_Status;
		}
		//
		if(tPD.C_COM.byGet_CC_Status == 0x01)
		{//CC1			
			if(++tPD.C_COM.byCC_Pin_Cnt >= 3)
			{
				tPD.C_COM.byCC_Pin_Cnt = 0;

				if(!tPD.Status.bCC_Status)
				{
					tPD.Status.bCC_Status = 1;
					tPD.Power.byCC_Ready_Cnt = 0;
					tPD.Status.bCC_Ready = 1;
					tPD.C_COM.byScource = eCC1;
				}
				break;
			}
		}
		else if(tPD.C_COM.byGet_CC_Status == 0x02)
		{//CC2
			if(++tPD.C_COM.byCC_Pin_Cnt >= 3)
			{
				tPD.C_COM.byCC_Pin_Cnt = 0;

				if(!tPD.Status.bCC_Status)
				{
					tPD.Status.bCC_Status = 1;
					tPD.Power.byCC_Ready_Cnt = 0;
					tPD.Status.bCC_Ready = 1;
					tPD.C_COM.byScource = eCC2;
				}
				break;
			}
		}
		else
		{
			if(++tPD.C_COM.byCC_Pin_Cnt >= 5)
			{
				tPD.C_COM.byCC_Pin_Cnt = 0;
				//TYPE_C端口未插入
				tPD.Status.bCC_Status = 0;
				tPD.Status.bCC_Ready = 0;
				//
				tPD.Power.byType = 0;
				tPD.Source.Information.byMessageID = 0;
				tPD.C_COM.byScource = eNull;
				tPD.byStatus = eNull;
				//
				byRequestStatus = eRequest_ComErr;
				break;
			}
		}
	}
	
	return byRequestStatus;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_pull_out_judge(void)
{
	if(tPD.Status.bCC_Status)
	{
		//检查CC状态
		if(!get_cmp0_out_status() || !get_cmp1_out_status())
			tPD.C_COM.byCC_Pin_Cnt = 0;//CC口状态为高
		else
		{
			//CC口拉低
			if(++tPD.C_COM.byCC_Pin_Cnt >= 50)//每1ms检查一次，持续50次均为低电平，认为TYPE C拔出
			{
				tPD.C_COM.byCC_Pin_Cnt = 0;
				//CC口持续拉低
				//TYPE_C端口未插入
				tPD.Status.bCC_Status = 0;
				tPD.byStatus = eNull;
				//
				sio_recv_ready();			//复位SIO的同时先把SIO模式切换至接收，否则会出现异常
				//复位外设
				pd_recv_module_reset();		//
				//
				tPD.Power.byType = 0;
				tPD.Source.Information.byMessageID = 0;
				
				tPD.Power.hwRecordVoltage = 5000;
				tPD.Power.hwRecordCurrent = 500;
				//设置CMP中断，等待下一次TYPE C插入
				pd_cc_recv_deploy();
				pd_cmp_init();
				
				pd_cmp_int_deploy(CMP0,ENABLE);
				pd_cmp_int_deploy(CMP1,ENABLE);
			}
		}
	}
	//
	if(tPD.Status.bCC_Ready)
	{
		if(++tPD.Power.byCC_Ready_Cnt >= 50)
		{
			tPD.Power.byCC_Ready_Cnt = 0;
			
			tPD.Status.bCC_Ready = 0;
			//CC口已完成滤波，开始准备接收数据
			pd_insert();
		}
	}
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void pd_1ms_int_handler(void)
{
	tPD.hwTimer1ms ++;
	tPD.hwTimer10ms ++;
	
	pd_pull_out_judge();			//CC口拔出判断
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/

void pd_cmp_int_handler(void)
{
	disable_global_interrupt();				//禁止全局中断
	if(!tPD.Status.bCC_Status)
	{	
		pd_cmp_int_deploy(CMP0,DISABLE);
		pd_cmp_int_deploy(CMP1,DISABLE);
	
		if(pd_judge_com_gpio() == eRequest_ComErr)
		{
			//COM口未接
			pd_cc_recv_deploy();
			
			pd_cmp_int_deploy(CMP0,ENABLE);
			pd_cmp_int_deploy(CMP1,ENABLE);
		}
	}
	else
	{
		wait_cmp_nbit_data();
		if(tPD.C_COM.byScource == eCC1)
			SIO0->RXCR1 = (SIO0->RXCR1 & 0XFFFFFFE7) | (0X01<<3);
		else
			SIO0->RXCR1 = (SIO0->RXCR1 & 0XFFFFFFE7) | (0X02<<3);	
		pd_sink_receive_decode();//解码
	}
	enable_global_interrupt();				//禁止全局中断
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void switch_5bitToBmc(volatile uint8_t *input, uint16_t length,volatile uint32_t *output)
{
	const uint8_t T_BitTable[8] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
	volatile uint8_t V_LastData = 0x01;
	volatile uint32_t V_Data  = 0x00;
	uint8_t i,j;
	//
	for(j = 0;j < length;j++)
	{
		for(i = 0;i < 5;i++)
		{
			if(input[j] & T_BitTable[i])
			{
				if((V_LastData == 0x01) || (V_LastData == 0x02))
					V_LastData = 0x02;
				else
					V_LastData = 0x03;
			}
			else
			{
				if((V_LastData == 0x01) || (V_LastData == 0x02))
					V_LastData = 0x00;
				else
					V_LastData = 0x01;
			}
			V_Data |= V_LastData;
			V_Data <<=2;
		}
		V_Data <<=20;
		output[j] = V_Data;
		V_Data = 0x00;
	}
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
uint8_t	switch_5bitTo4bit(const uint8_t* byArray,volatile uint8_t* byTarget)
{
	uint8_t	byReturnData = 0;
	
	for(uint8_t i = 0;i < 16 ;i++)//转出4B数据
	{
		if(*byArray == *byTarget)
		{
			byReturnData = i;
			break;
		}
		byArray ++;
	}
	
	return	byReturnData;
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void get_recv_data(volatile uint32_t *output,volatile uint16_t hwStatusAddr,volatile uint8_t byDataNum)
{
	volatile uint8_t 	byTemp[2][8];
	volatile uint16_t	hwGetDataAddr = hwStatusAddr;
	volatile uint64_t	dwGet_DataTemp;
	for(uint8_t i = 0;i < byDataNum;i ++)
	{
		//获取信息
		dwGet_DataTemp = extract_received_data((volatile uint32_t*)&wPD_BMC_Recv,hwGetDataAddr,40);//提取对象，提取地址，提取长度
		//
		byTemp[0][0] = (dwGet_DataTemp >> 35) & 0x1F;
		byTemp[0][1] = (dwGet_DataTemp >> 30) & 0x1F;
		byTemp[0][2] = (dwGet_DataTemp >> 25) & 0x1F;
		byTemp[0][3] = (dwGet_DataTemp >> 20) & 0x1F;
		byTemp[0][4] = (dwGet_DataTemp >> 15) & 0x1F;
		byTemp[0][5] = (dwGet_DataTemp >> 10) & 0x1F;
		byTemp[0][6] = (dwGet_DataTemp >> 5) & 0x1F;
		byTemp[0][7] = dwGet_DataTemp & 0x1F;
		//
		byTemp[1][0] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][0]);
		byTemp[1][1] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][1]);
		byTemp[1][2] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][2]);
		byTemp[1][3] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][3]);
		byTemp[1][4] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][4]);
		byTemp[1][5] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][5]);
		byTemp[1][6] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][6]);
		byTemp[1][7] = switch_5bitTo4bit(switch_4bTo5b_HexTable_Recv,&byTemp[0][7]);
		//
		output[i] = 	byTemp[1][7] << 28 | byTemp[1][6] << 24 |
						byTemp[1][5] << 20 | byTemp[1][4] << 16 |
						byTemp[1][3] << 12 | byTemp[1][2] << 8 |
						byTemp[1][1] << 4 | (byTemp[1][0] & 0x0f);
		hwGetDataAddr += 40;
	}
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
//提取跨16位边界的n位数据片段
uint32_t extract_nbit_slice(uint32_t word1,uint32_t word2,uint32_t word3, uint8_t extractoffset,uint8_t length)
{
	volatile uint64_t combined;
	combined = word1;
	combined = ((combined & 0xffff) << 32) | ((word2 & 0xffff) << 16) | (word3 & 0xffff);
	combined = (combined >> (48 - length - extractoffset)) & 0xFFFFF;
    return combined;//(combined >> (extractoffset + 12)) & 0xFFFFF;// 12=32-20
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
// 计算汉明距离（比特差异计数）//将一个字符串变换成另外一个字符串所需要替换的字符个数
//uint8_t hamming_distance(uint32_t a, uint32_t b)
//{
//    volatile uint32_t x = a ^ b;
//    return __builtin_popcount(x); 				// GCC内置函数
//}

//uint8_t min_u8(uint8_t a, uint8_t b) 
//{
//    return (a < b) ? a : b;
//}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
int16_t detect_sop_position(volatile uint32_t* data,uint16_t hwStatusAddr,uint16_t hwOverAddr,uint8_t byData_len)//数组,数组大小,检查起始位置，检查终止位置
{
    volatile uint32_t 	searchWindow;
	volatile uint8_t	byCurrentMinErr = 0xff;
    volatile int16_t	iBestMatchIdx = -1;		//位置
	volatile uint8_t 	byStart_idx = hwStatusAddr / 16;
    volatile uint8_t 	byStart_shift = hwStatusAddr % 16;

	volatile uint8_t 	byEnd_idx = hwOverAddr / 16;
    volatile uint8_t 	byEnd_shift = hwOverAddr % 16;
	if(byEnd_shift)
		byEnd_idx ++;
	volatile uint8_t 	offset = byStart_shift,wordIdx;
	for(wordIdx = byStart_idx; wordIdx < byEnd_idx; wordIdx++)
	{
		for(;offset < 16; offset++)
		{
			searchWindow = extract_nbit_slice((uint16_t)data[wordIdx], (uint16_t)data[wordIdx+1],(uint16_t)data[wordIdx+2],offset,byData_len);
			searchWindow &= 0x1f;
			if(searchWindow == SOP_PATTERN)
			{
				byCurrentMinErr = 0;
				break;
			}
			else if(searchWindow == RESET_PATTERN)
			{
				byCurrentMinErr = 0x01;
				break;;
			}
		}
		//找到SOP或Reset
		if(!byCurrentMinErr)
		{
			iBestMatchIdx = (wordIdx * 16) + offset;
			break;
		}
		else if(byCurrentMinErr == 0x01)
		{
			iBestMatchIdx = -2;
			break;
		}
		offset = 0;
	}
	return iBestMatchIdx;			//返回偏移量
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
int64_t extract_received_data(volatile uint32_t* data,volatile uint16_t byStartAddr,volatile uint8_t byData_len)//数组，起始地址，数据长度
{
	volatile uint8_t 	start_idx = byStartAddr / 16;
    volatile uint8_t 	bit_shift = byStartAddr % 16;
    volatile uint16_t 	total_needed = bit_shift + byData_len;
//---------------------------------------------------------------------------
	if (start_idx >= 80 || total_needed > (80 - start_idx) * 16)  //检查越界
        return -1;  /* 错误标志 */
	volatile uint64_t 	dwGetData[2];
	volatile uint8_t	byShiftNum;
//------------------------------------------------------------------------
	dwGetData[0] = ((data[start_idx] & 0XFFFF) << 16) | (data[start_idx + 1] & 0XFFFF);
	dwGetData[1] = ((data[start_idx + 2] & 0XFFFF) << 16) | (data[start_idx + 3] & 0XFFFF);
	dwGetData[0] = (dwGetData[0] << 32) | dwGetData[1];
//------------------------------------------------------------------------
	byShiftNum = 64 - bit_shift - byData_len;

	return (dwGetData[0] >> byShiftNum);
}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
//uint8_t validate_packet_structure(volatile uint8_t* data, int sopBitOffset)//帧同步验证
//{
//	uint8_t		byDataValidate = eValidatePASS;
//	uint8_t		byDataTemp[2];
//	byDataTemp[1] = *data;
//	byDataTemp[0] = *(data+1);
//    // 1. 检测包尾EOP (End of Packet)
//    uint16_t eopPattern = extract_16bit_slice(byDataTemp[0], data[1], sopBitOffset);
////    if ((eopPattern & 0xFF) != EOP_PATTERN)
////        byDataValidate = eValidateEOP_Fail;// EOP验证失败
////    // 2. CRC32校验
////    uint32_t calcCrc = calculate_pd_crc(data, sopBitOffset);
////    uint32_t receivedCrc = extract_crc_field(data, sopBitOffset);
//    
////    if (calcCrc != receivedCrc)
////		byDataValidate = eValidateCRC_Fail;
//    
//    return byDataValidate;
//}
//=====================================================================================
/** \brief		:
 *  \param[in] 	:none
 *  \return 	:none
//===================================================================================*/
void wait_cmp_nbit_data(void)
{
	volatile uint8_t 	byWait_cmp_3bit_UP_Num,byWait_cmp_3bit_bk;
	volatile uint16_t 	hwWait_Max_Time = 40950;
	byWait_cmp_3bit_UP_Num = 0;
	byWait_cmp_3bit_bk = 0;
	while(hwWait_Max_Time --)
	{
		if(tPD.C_COM.byScource == eCC1)
		{
			if(!(byWait_cmp_3bit_bk))
			{
				if(!(CMP0->CR & 0X01 << 24))
				{
					byWait_cmp_3bit_bk = 1;
					if(++byWait_cmp_3bit_UP_Num >= Wait_cmp_nbit)
					{
						break;//从第三个低电平开始处理数据
					}
				}
			}
			else
			{
				if((CMP0->CR & 0X01 << 24))
					byWait_cmp_3bit_bk = 0;
			}
		}
		else
		{
			if(!byWait_cmp_3bit_bk)
			{
				if(!(CMP1->CR & 0X01 << 25))
				{
					byWait_cmp_3bit_bk = 1;
					if(++byWait_cmp_3bit_UP_Num >= Wait_cmp_nbit)
					{
						break;//从第三个低电平开始处理数据
					}
				}
			}
			else
			{
				if((CMP1->CR & 0X01 << 25))
					byWait_cmp_3bit_bk = 0;
			}
		}
	}
}

