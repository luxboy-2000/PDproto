
/* **********************************************************************
 * \file  user_bmc.h
 * \brief chip level information for apt32f031
 * \copyright Copyright (C) 2015-2024 @ APTCHIP
 * <table>
 * <tr><th> Date  <th>Version  <th>Author  <th>Description
 * <tr><td> 2024-7 <td>V1.0 <td>WNN     <td>new STDLib
 * </table>
 * *********************************************************************
*/



/* Define to prevent recursive inclusion -------------------------------------*/ 
#ifndef _PD_INIT_H_
#define _PD_INIT_H_
/* Includes ------------------------------------------------------------------*/

//#include "main.h"
//#include "inc.h"
#include "pd_protocol.h"
//===================================================================================


//===================================================================================
//===================================================================================
//端口数据角色
typedef enum
{
	eCC1 = 1,					
	eCC2,
}PD_Source_Pin_e;
//
typedef enum
{
	eRequest_OK = 0,
	eRequest_TimeOut = 1,
	eRequest_Reset = 2,
	eRequest_UFP = 3,
	eRequest_ComErr = 4,		//type c线未插好
	eRequest_Null = 5,
}PD_FALSE_e;
//PD取电状态
typedef enum
{
	eNull = 0,					//空信息
	eSource_Broadcast = 1,		//Source广播电源能力
	eSink_Request,				//Sink电源请求
	eSource_Accept,				//Source接受Sink电源请求
	eSource_Ready,				//Source准备开始输出电压
	eSource_VCONN_Swap,			//Source发起的数据交换
	eSource_Otcher_News,		//Source发起的其他信息
	eSource_Reset,				//Source发起复位
	eSource_Error,				//错误
	eSource_NotSOP,				//没找到SOP
}PD_Request_Status_e;
//消息类型(message type)
typedef enum
{
	ePD_GOODCRC 		= 	0X01,
	ePD_GotoMin			=	0X02,
	ePD_Accept			=	0X03,
	ePD_Reject			=	0X04,
	ePD_Ping			=	0X05,
	ePD_PS_RDY			=	0x06,
	ePD_Get_Source_Cap 	= 	0x07,
	ePD_Get_Sink_Cap	=	0x08,
	ePD_DR_Swap			=	0x09,
	ePD_PR_Swap			=	0x0a,
	ePD_VCONN_Swap		=	0x0b,
	ePD_Wait			=	0x0c,
	ePD_Soft_Reset		=	0x0d,
	ePD_Data_Reset		=	0x0e,
	ePD_Data_Reset_Complete	=	0x0f,
	ePD_Not_Supported	=	0x10,
	ePD_Get_Source_Cap_Extended	=	0x11,
	ePD_Get_Status		=	0x12,
	ePD_FR_Swap			=	0x13,
	ePD_Get_PPS_Status	=	0x14,
	ePD_Get_Country_codes	=	0x15,
	ePD_Get_Sink_Cap_Extended	=	0x16,
	ePD_Source_Info		=	0x17,
	ePD_Revision		=	0x18,
}PD_MessageType_e;
//PD数据角色
typedef enum
{
	ePD_UFP = 0,
	ePD_DFP,
}PD_Role_e;
//PD版本
typedef enum
{
	ePD_Version1_0 = 0x00,
	ePD_Version2_0 = 0x01,
	ePD_Version3_0 = 0x02,
}PD_Version_e;
//PD内容定义
typedef enum
{
	ePD_Init = 0,				//未开始通讯
	ePD_Work,					//通讯阶段
	ePD_PowerTransmission,		//功率传输阶段
	ePD_RequestVoltage,			//重新请求电压
	ePD_RequestVoltage_Wait,	//重新取电等待确认
	//电源类型
	ePower_Type_PDO = 0x00,		//固定电压
//	ePower_Type_Bat = 0x01,
//	ePower_Type_AVS = 0x02,
	ePower_Type_PPS = 0x03,		//增强型可变电压/电流
	//释放计时
	eRelease_Time = 300000/33,	//us
	//回复Source的PD版本号
	ePD_Version		=	ePD_Version3_0 << 2,
	
}Pd_e;
//CRC校验用的Header及Message
typedef struct
{
	uint16_t	hwHeader;
	uint32_t	wMessage;
}CRC_check_t;
//PD工作状态标志
typedef union
{
	uint8_t	AllBits;
	struct
	{
		uint8_t			bCC_Status:1;
		uint8_t			bCC_Ready:1;
		uint8_t			bAllowRequest:1;
	};
}PD_Status_u;
//Head数据状态
typedef union
{
	uint16_t	AllBits;
	struct
	{
		uint16_t 		byMessageType:5;
		uint16_t		byPortDataRole:1;
		uint16_t		byRevision:2;
		uint16_t		bCablePlug:1;
		uint16_t		byMessageID:3;
		uint16_t		byDataLength:3;
		uint16_t		bExtended:1;
	};
}PD_information_u;
//用来记录电源请求状态
typedef struct
{
	uint8_t				byGetlocation;		//请求对象信息存储位置
//	uint8_t				byGetType;			//请求类型
	
	uint8_t				byCC_Ready_Cnt;

	uint8_t				byType;				//供电头一共提供几种电源选择
	uint16_t			hwVoltage[8];		//最多8中电源选择
	uint16_t			hwCurrent[8];
	
	uint16_t			hwRecordVoltage;	//当前取电电压，上电默认5000mv
	uint16_t			hwRecordCurrent;	//当前取电电流，默认无功率要求
}PD_Power_t;
//用来记录PD电源包数据
typedef struct
{
	uint32_t			wTypeData[8];		//广播数据
	PD_information_u	Information;		//Source-Headered信息
}PD_Source_t;
//用来判断PD协议的通讯口是否正常接入
typedef struct
{
	uint8_t				byGet_CC_Status;	//端口状态
	uint8_t				byGet_CC_Status_bk;
	uint8_t				byCC_Pin_Cnt;
	
	uint8_t 			byScource;	//
}PD_CC_Status_t;
//PD协议内容
typedef struct
{
	uint16_t			hwGetVoltage;	
	uint16_t			hwGetCurrent;
	uint8_t				byAllowLeftOffset;
	uint8_t				byAllowRightOffset;

	uint16_t			hwNowVoltage;		//当前取电成功电压
	uint16_t			hwNowCurrent;		//当前取电成功电流
	
	uint16_t			hwTimer1ms;
	uint16_t			hwTimer10ms;
	
	PD_Power_t			Power;
	PD_CC_Status_t		C_COM;				//端口状态
	
	PD_Status_u			Status;				//状态
	CRC_check_t			Send_CRC;
	PD_Source_t			Source;
	//
	uint8_t				byStatus;
}Pd_protocol_t;
extern volatile Pd_protocol_t	tPD;
//===================================================================================
//===================================================================================
extern volatile uint32_t 	wPD_BMC_Recv[50];
extern volatile uint32_t 	wPD_Buf_Recv;
//===================================================================================
//===================================================================================
extern void 	sio_send_ready(void);								//SIO作为发送数据前准备
extern void 	sio_recv_ready(void);								//SIO作为接收前准备
extern void 	sio_wait_break_rst(void);							//SIO等待break原始中断
extern void 	pd_send_start(void);
extern uint8_t	sio_wait_status_busy(void);							//SIO等待忙状态
extern void 	pd_cc_recv_deploy(void);							//pd协议所需要的接收引脚配置
extern void 	pd_cc_deploy_deinit(void);							//pd协议所需要的引脚释放
extern void 	pd_cc_send_deploy(uint8_t	bySendPin);				//判断协议所需要的发送引脚配置
extern void 	pd_cmp_int_deploy(csp_cmp_t *ptCmpBase,functional_status_e eEnable);//cmp中断配置
extern void 	enable_global_interrupt(void);						//使能全局中断
extern void 	disable_global_interrupt(void);						//禁止全局中断
extern void 	pd_crc_set_seed(uint32_t wSeedVal);					//crc种子值写入
extern uint32_t get_crc_result(void);								//获取crc结果
extern uint32_t get_cmp0_out_status(void);							//获取cmp0输出状态
extern uint32_t get_cmp1_out_status(void);							//获取cmp1输出状态
extern void 	pd_dma_start(dma_ch_e eDmaCh);						//开始dma传输
extern void 	pd_sio_send_init(uint8_t* wSrcAdd,uint16_t hwTxCnt);//pd数据发送初始化
extern void 	pd_sio_recv_init(void);								//pd数据接收初始化
extern void 	pd_cmp_init(void);									//pd使用的cmp初始化
extern void 	pd_crc32_init(void);								//pd使用的crc校验所需要的crc初始化
extern uint8_t 	pd_judge_com_gpio(void);							//判断cc接口是CC1还是CC2
extern void 	pd_recv_module_reset(void);							//执行后重新配置外设模块进入接收模式
extern volatile uint32_t pd_crc_calculation(volatile uint8_t* ptData, uint16_t wNum);//pd数据crc执行函数
//===================================================================================
//===================================================================================
#endif