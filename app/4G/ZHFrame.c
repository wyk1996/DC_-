/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*---------------------------------------------------------------------------------------
* FileName			: GPRSMain.c
* Author			: 
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "4GMain.h"
#include "4GRecv.h"
#include "ZHFrame.h"
#include <string.h>
#include "sysconfig.h"
#include "DisplayMain.h"
#include "charging_Interface.h"
#include "RTC_task.h"
#include "PCUMain.h"
#include "DisplayMain.h"
/* Private define-----------------------------------------------------------------------------*/
typedef enum{
	

	zh_cmd_type_7 	= 7,
	zh_cmd_type_8 	= 8,
	zh_cmd_type_11	= 11,
	zh_cmd_type_12 	= 12,
	zh_cmd_type_101	= 101,
	zh_cmd_type_102 = 102,
	zh_cmd_type_103	= 103,
	zh_cmd_type_104	= 104,
	zh_cmd_type_105 = 105,
	zh_cmd_type_106	= 106,
	zh_cmd_type_201	= 201,
	zh_cmd_type_202	= 202,
	zh_cmd_type_107 = 107,
	zh_cmd_type_108	= 108,
	zh_cmd_type_113 = 113,
	zh_cmd_type_114 = 114,	
	zh_cmd_type_117	= 117,
	zh_cmd_type_118	= 118,
	zh_cmd_type_1309	= 1309,
	zh_cmd_type_1310	= 1310,
	zh_cmd_type_1311	= 1311,
	zh_cmd_type_1312	= 1312,
}zh_mqtt_cmd_enum;

//注册
__packed typedef struct{
	INT16U res[2];		//预留
	INT32U ack_num;		//106报文随机数应答匹配码
	INT8U st_flag;		//启停用标志  0x00-正常使用  0x55-暂停服务
	uint8_t     	time[8];		//同步充电桩时间  平台标准BCD时间
}zh_cmd_105;

__packed typedef struct {
  INT16U  res[2];						//预留 
  uint8_t   equipment_id[16];			//充电桩编码	
  uint8_t   offline_charge_flag;		//离线/在线允许充电设置 默认值为0x01，其中高位的0表示在线允许充电，低位的1表示离线禁止充电。如果低位设0，则表示离线允许充电
  uint8_t	stake_version[4];			//充电桩软件版本
  uint8_t  stake_type;					//充电桩类型0x00 直流0x01 交流0x02 混合
  uint32_t  stake_start_times;			//启动次数 终端每次启动，计数保存
  uint8_t   data_up_mode;				//终端每次启动，计数保存   1：应答模式2：主动上报模式
  uint16_t  sign_interval;				//签到间隔时间
  uint8_t   gun_index;					//充电枪个数
  uint8_t   heartInterval;				//心跳上报周期
  uint8_t   heart_out_times;			//心跳包检测超时次数
  uint32_t	stake_charge_record_num;	//充电记录数量
  uint8_t   stake_systime[8];			//当前充电桩系统时间1
  uint8_t   stake_last_charge_time[8];	//最近一次充电时间
  uint8_t   stake_last_start_time[8];	//最近一次启动时间
  uint8_t   signCode[8];				//签到密码（保留）
  uint8_t  ccu_version[4];				//充电桩CCU软件版本
}zh_cmd_106;

//心跳
//101
__packed typedef struct{
	uint16_t heart_index;		//心跳应答   原样返回收到的102心跳序号字段
} zh_cmd_101;
//102
__packed typedef struct{
	uint8_t	equipment_id[16];	//充电桩编码
	uint16_t heart_index;		//心跳序号  省=0，由桩端统一编号，严格递增。达到最大值时，重新从0开始累加
} zh_cmd_102;


//充电桩状态
//103
__packed typedef struct{
	uint8_t	gun_index;					//充电口号
	uint8_t IfAck;						//是否查询    0：应答 1查询
}zh_cmd_103;
//104
__packed typedef struct{				
	uint8_t	equipment_id[16];			//充电桩编码
	uint8_t	gun_cnt;					//充电枪数量
	uint8_t	gun_index;					//充电口号 从0开始
	uint8_t 	gun_type;				//充电枪类型    1=直流； 2=交流；
	uint8_t 	work_stat;				//工作状态0-空闲中 1-已插枪2-充电进行中 3-未拔枪4-预约状态5-自检6-系统故障7-停止中
	uint8_t  soc_percent;				//当前SOC %
	uint8_t  alarm_stat[4];				//告警状态（保留） 0-断开 1-半连接 2-连接直流目前只有0和2状态交流目前有0、1、2三种状态
	uint8_t car_connection_stat;		//车辆连接状态
	uint32_t cumulative_charge_fee;		//本次充电累计充电费用  从本次充电开始到目前的累计充电费用（包括电费与服务费），这里是整型，要乘以0.01才能得到真实的金额
	uint16_t	dc_charge_voltage;		//直流充电电压	
	uint16_t dc_charge_current;			//直流充电电流
	uint16_t bms_need_voltage;			//BMS需求电压
	uint16_t bms_need_current;			//BMS需求电流
	uint8_t bms_charge_mode;			//BMS充电模式  充电有效（直流有效，交流置0）1-恒流 2-恒压
	uint16_t ac_a_vol;					//交流A相充电电压
	uint16_t ac_b_vol;					//流B相充电电压
	uint16_t ac_c_vol;					//交流C相充电电压
	uint16_t ac_a_cur;					//交流A相充电电流
	uint16_t ac_b_cur;					//交流B相充电电流
	uint16_t ac_c_cur;					//交流C相充电电流
	uint16_t charge_full_time_left;		//距离满电剩余充电时间(min)
	uint32_t charged_sec;				//充电时长(秒)
	uint32_t cum_charge_kwh_amount; /*累计充电电量 */
	uint32_t cum_charge_money_amount; /*累计充电金额 */
	uint32_t before_charge_meter_kwh_num; /*充电前电表读数 */
	uint32_t now_meter_kwh_num; /* 当前电表读数 */

	uint8_t start_charge_type;				//充电启动方式   0：本地刷卡启动1：后台启动2：本地管理员启动
	uint8_t charge_policy;					//充电策略  0自动充满 1按时间充满  2定金额  3按电量充满 4按SOC充（直流）  
	int32_t charge_policy_param;			//充电策略参数 时间单位为1秒金额单位为0.01元电量时单位为0.01kw SOC为1%
	uint8_t book_flag;						//预约标志  0-无预约（无效）  1-预约有效
	uint8_t charge_user_id[32];				//订单号
	uint8_t book_timeout_min;				//预约超时时间    单位分钟
	uint8_t book_start_charge_time[8];		//预约/开始充电开始时间
	int32_t before_charge_card_account;		//充电前卡余额（保留）
	int32_t charge_power_kw; /* 充电功率 */	//充电功率 0.1kw
} zh_cmd_104;








//发送数据
//hy_cmd_106 	SendHYCmd106;			//注册
//hy_cmd_8	SendHYCmd8;				//启动应答
//hy_cmd_12	SendHYCmd12;			//停止应答
//hy_cmd_102	SendHYCmd102;			//上传心跳
//hy_cmd_104	SendHYCmd104[GUN_MAX];			//状态上报	
//hy_cmd_202	SendHYCmd202;			//上传充电记录
//hy_cmd_108	SendHYCmd108;			//充电桩故障上报（充电过程中产生）
//hy_cmd_118	SendHYCmd118;			//充电桩故障上报
//hy_cmd_1310 SendHYCmd1310;			//计费设置相应

zh_cmd_106 	SendZHCmd106;			//注册
zh_cmd_102	SendZHCmd102;			//上传心跳
zh_cmd_104	SendZHCmd104[GUN_MAX];			//状态上报	

//接收函数
//hy_cmd_105 	RecvHYCmd105;			//注册
//hy_cmd_7	RecvHYCmd7[GUN_MAX];	//启动
//hy_cmd_11	RecvHYCmd11[GUN_MAX];	//停止
//hy_cmd_101	RecvHYCmd101;			//心跳
//hy_cmd_103	RecvHYCmd103[GUN_MAX];	//状态应答
//hy_cmd_201	RecvHYCmd201;			//充电记录应答
//hy_cmd_107	RecvHYCmd107;			//充电桩故障应答（充电过程中产生）
//hy_cmd_117	RecvHYCmd117;			//充电桩故障应答
//hy_cmd_1309 RecvHYCmd1309[12];		//费率设置	最多4组
zh_cmd_105 	RecvZHCmd105;			//注册
zh_cmd_101	RecvZHCmd101;			//心跳
zh_cmd_103	RecvZHCmd103[GUN_MAX];	//状态应答



#define ZH_SEND_FRAME_LEN   4
#define ZH_RECV_FRAME_LEN	0



static INT8U   ZH_SendRegister(void); //注册
static INT8U   ZH_SendHear(void)  ;//心跳
INT8U   ZH_SendDevStateA(void); //充电桩A状态
INT8U   ZH_SendDevStateB(void);  //充电桩B状态

_4G_SEND_TABLE ZHSendTable[ZH_SEND_FRAME_LEN] = {
	{0,    0,    SYS_DELAY_10s, 	ZH_SendRegister			},  //发送注册帧

	{0,    0,    SYS_DELAY_30s, 	ZH_SendHear				},	//心跳
	
	{0,    0,    SYS_DELAY_1M, 	ZH_SendDevStateA		},	//充电桩A状态

	{0,    0,    SYS_DELAY_1M, 	ZH_SendDevStateB		},	//充电桩B状态

};


//_4G_RECV_TABLE HYRecvTable[ZH_RECV_FRAME_LEN] = {

//};



/*****************************************************************************
* Function     : APP_GetZHQGNetMoney
* Description  :获取账户余额
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT32U APP_GetZHQGNetMoney(INT8U gun)
{
	return 1;
}

/*****************************************************************************
* Function     : APP_GetZHBatchNum
* Description  : 获取交易流水号
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
******************************************************************************/
INT8U *  APP_GetZHBatchNum(INT8U gun)
{
	NOP();
	return NULL;
}

/*****************************************************************************
* Function     : get_crc_Data
* Description  : 累加和
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
static INT8U get_crc_Data(INT8U *pbuf,INT16U datalong)
{
	INT16U i;
	INT8U crcdata;
	crcdata = 0;
	for(i=0;i<datalong;i++)
	{
		crcdata += pbuf[i];
	}
	return crcdata;
}

/*****************************************************************************
* Function     : ZHFreamSend
* Description  : 宇晖帧发送
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   ZHFreamSend(INT16U cmd,INT8U *pdata, INT16U len)
{
	INT8U Sendbuf[500];
	static INT8U count;			//序列号域 
	INT16U datalen = 15+len;
	
	if((pdata == NULL) || (!len) )
	{
		return FALSE;
	}
	//起始域
	Sendbuf[0] = 0xAA;
	Sendbuf[1] = 0xF5;
	//2个字节数据长度
	Sendbuf[2] = datalen& 0x000000ff;
	Sendbuf[3] = (datalen >> 8)& 0x000000ff;
	//1个字节信息域
	Sendbuf[4] = 0x10;
	
	//1个字节序列号
	Sendbuf[5] = count;

	//2个字节命令代码  小端
	Sendbuf[6] = cmd & 0x00ff;
	Sendbuf[7] = (cmd >> 8) & 0x00ff;
	
	memcpy(&Sendbuf[8],pdata,len);
	//1字节CS
	Sendbuf[8+len] = get_crc_Data(Sendbuf,14+len);
	ModuleSIM7600_SendData(0, Sendbuf,(9+len)); //发送数据
	//OSTimeDly(SYS_DELAY_50ms); 
	count++;
	return TRUE;
}

/*****************************************************************************
* Function     : ZH_SendFrameDispose
* Description  : 汇誉接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   ZH_SendFrameDispose(void)
{
	NOP();
	return TRUE;
}

/*******************************周期性发送数据*******************************/
/*****************************************************************************
* Function     : ZH_SendStopAck
* Description  : 停止应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   ZH_SendStopAck(_GUN_NUM gun)
{
	return TRUE;
}

/*****************************************************************************
* Function     : HY_SendRegister
* Description  : 注册
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   ZH_SendRegister(void)
{
	INT8U * pdevnum = APP_GetDevNum(); 
	
	if(APP_GetAppRegisterState(LINK_NUM) == STATE_OK)	//显示已经注册成功了
	{
		return  FALSE;
	}
//	SendHYCmd106.charge_mode_num = 8;
//	SendHYCmd106.charge_mode_rate = 1200;		//120kw
//	memcpy(SendHYCmd106.equipment_id,pdevnum,16);	//充电桩编号
//	SendHYCmd106.offline_charge_flag = 0;			//离线允许充电
//	SendHYCmd106.stake_type = 0x00;					//充电桩类型0x00 直流0x01 交流0x02 混合
//	SendHYCmd106.gun_index = APP_GetGunNum();					//充电桩枪数量
	return ZHFreamSend(zh_cmd_type_106,(INT8U*)&SendZHCmd106,sizeof(SendZHCmd106));

}


/*****************************************************************************
* Function     : HY_SendHear
* Description  : 心跳
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
static INT8U   ZH_SendHear(void) 
{
	//static INT16U count = 0;
	INT8U * pdevnum = APP_GetDevNum(); 
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
//	memcpy(SendZHCmd102.equipment_id,pdevnum,16);	//充电桩编号
//	SendZHCmd102.heart_index = count++;
	return ZHFreamSend(zh_cmd_type_102,(INT8U*)&SendZHCmd102,sizeof(SendZHCmd102));
}


/*****************************************************************************
* Function     : ZH_SendDevStateA
* Description  : 充电桩A状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   ZH_SendDevStateA(void)
{
	//TEMPDATA * pmeter = NULL;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	INT8U * pdevnum = APP_GetDevNum(); 
	USERINFO * puserinfo  = GetChargingInfo(GUN_A);
	//pmeter = APP_GetMeterPowerInfo(GUN_A);
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	
	memset(&SendZHCmd104[GUN_A],0,sizeof(zh_cmd_104));
//	memcpy(SendHYCmd104[GUN_A].equipment_id,pdevnum,16);	//充电桩编号
//	SendHYCmd104[GUN_A].gun_cnt = APP_GetGunNum();					//充电枪数量
//	SendHYCmd104[GUN_A].gun_index = 0;						//充电口号
//	SendHYCmd104[GUN_A].gun_type = 1;						//充电枪类型  1=直流； 2=交流；
//	if(pdisp_conrtol->NetGunState[GUN_A] == GUN_CHARGEING)
//	{
//		SendHYCmd104[GUN_A].work_stat = 2;
//	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_UNLINK)
//	{
//		SendHYCmd104[GUN_A].work_stat = 3;
//	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_STOPING)
//	{
//		SendHYCmd104[GUN_A].work_stat = 7;
//	}else if(pdisp_conrtol->NetGunState[GUN_A] == GUN_FAIL)
//	{
//		SendHYCmd104[GUN_A].work_stat = 6;
//	}else
//	{
//		if(GetGunState(GUN_A) == GUN_DISCONNECTED)
//		{
//			SendHYCmd104[GUN_A].work_stat = 0;		//空闲
//		}else
//		{
//			SendHYCmd104[GUN_A].work_stat = 1;		//已经插枪
//		}
//	}
//	//车辆连接状态
//	if(GetGunState(GUN_A) == GUN_DISCONNECTED)
//	{
//		SendHYCmd104[GUN_A].car_connection_stat = 0;		//空闲
//	}else
//	{
//		SendHYCmd104[GUN_A].car_connection_stat = 2;		//已经插枪
//	}	
//	
//	
//	if(SendHYCmd104[GUN_A].work_stat == 2)
//	{
//		SendHYCmd104[GUN_A].soc_percent = BMS_BCS_Context[GUN_A].SOC; 				//当前spc
//		SendHYCmd104[GUN_A].cumulative_charge_fee = puserinfo->TotalBill;   			//充电电费
//		SendHYCmd104[GUN_A].dc_charge_voltage = PowerModuleInfo[GUN_A].OutputInfo.Vol;			//电压
//		SendHYCmd104[GUN_A].dc_charge_current  =  PowerModuleInfo[GUN_A].OutputInfo.Cur;	//电流
//		SendHYCmd104[GUN_A].bms_need_voltage = BMS_BCL_Context[GUN_A].DemandVol;			//需求电压
//		SendHYCmd104[GUN_A].bms_need_current = BMS_BCL_Context[GUN_A].DemandCur;	//需求电流
//		SendHYCmd104[GUN_A].bms_charge_mode = 0;										//自动充满
//		SendHYCmd104[GUN_A].charge_full_time_left = BMS_BCS_Context[GUN_A].RemainderTime;
//		SendHYCmd104[GUN_A].charged_sec = puserinfo->ChargeTime * 60;
//		SendHYCmd104[GUN_A].cum_charge_money_amount =  puserinfo->TotalBill;		//累计充电金额
//		SendHYCmd104[GUN_A].cum_charge_kwh_amount = puserinfo->TotalPower;			//累计充电电量
//		SendHYCmd104[GUN_A].before_charge_meter_kwh_num = pmeter->StartPower;		//开始充电读表
//		SendHYCmd104[GUN_A].now_meter_kwh_num = pmeter->CurPower;					//当前充电读表
//		SendHYCmd104[GUN_A].charge_power_kw = SendHYCmd104[GUN_A].dc_charge_voltage * SendHYCmd104[GUN_A].dc_charge_current / 10; 
//		memcpy(SendHYCmd104[GUN_A].charge_user_id,RecvHYCmd7[GUN_A].charge_user_id,32);		//订单号
//	}
//	else
//	{
//		;
//	}
	return ZHFreamSend(zh_cmd_type_104,(INT8U*)&SendZHCmd104[GUN_A],sizeof(zh_cmd_104));
}

/*****************************************************************************
* Function     : ZH_SendDevStateB
* Description  : 充电桩B状态
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   ZH_SendDevStateB(void)
{
	//TEMPDATA * pmeter = NULL;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	INT8U * pdevnum = APP_GetDevNum(); 
	USERINFO * puserinfo  = GetChargingInfo(GUN_B);
	//pmeter = APP_GetMeterPowerInfo(GUN_B);
	
	if(APP_GetAppRegisterState(LINK_NUM) != STATE_OK)
	{
		return FALSE;		//注册未成功，无需发送
	}
	if(APP_GetGunNum() == 1)
	{
		return FALSE;
	}
	memset(&SendZHCmd104[GUN_B],0,sizeof(zh_cmd_104));
//	memcpy(SendHYCmd104[GUN_B].equipment_id,pdevnum,16);	//充电桩编号
//	SendHYCmd104[GUN_B].gun_cnt = APP_GetGunNum();					//充电枪数量
//	SendHYCmd104[GUN_B].gun_index = 1;						//充电口号
//	SendHYCmd104[GUN_B].gun_type = 1;						//充电枪类型  1=直流； 2=交流；
//	if(pdisp_conrtol->NetGunState[GUN_B] == GUN_CHARGEING)
//	{
//		SendHYCmd104[GUN_B].work_stat = 2;
//	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_UNLINK)
//	{
//		SendHYCmd104[GUN_B].work_stat = 3;
//	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_STOPING)
//	{
//		SendHYCmd104[GUN_B].work_stat = 7;
//	}else if(pdisp_conrtol->NetGunState[GUN_B] == GUN_FAIL)
//	{
//		SendHYCmd104[GUN_B].work_stat = 6;
//	}else
//	{
//		if(GetGunState(GUN_B) == GUN_DISCONNECTED)
//		{
//			SendHYCmd104[GUN_B].work_stat = 0;		//空闲
//		}else
//		{
//			SendHYCmd104[GUN_B].work_stat = 1;		//已经插枪
//		}
//	}
//	//车辆连接状态
//	if(GetGunState(GUN_B) == GUN_DISCONNECTED)
//	{
//		SendHYCmd104[GUN_B].car_connection_stat = 0;		//空闲
//	}else
//	{
//		SendHYCmd104[GUN_B].car_connection_stat = 2;		//已经插枪
//	}	
//	
//	
//	if(SendHYCmd104[GUN_B].work_stat == 2)
//	{
//		SendHYCmd104[GUN_B].soc_percent = BMS_BCS_Context[GUN_B].SOC; 				//当前spc
//		SendHYCmd104[GUN_B].cumulative_charge_fee = puserinfo->TotalBill;   			//充电电费
//		SendHYCmd104[GUN_B].dc_charge_voltage = PowerModuleInfo[GUN_B].OutputInfo.Vol;			//电压
//		SendHYCmd104[GUN_B].dc_charge_current  =  PowerModuleInfo[GUN_B].OutputInfo.Cur;	//电流
//		SendHYCmd104[GUN_B].bms_need_voltage = BMS_BCL_Context[GUN_B].DemandVol;			//需求电压
//		SendHYCmd104[GUN_B].bms_need_current = BMS_BCL_Context[GUN_B].DemandCur;	//需求电流
//		SendHYCmd104[GUN_B].bms_charge_mode = 0;										//自动充满
//		SendHYCmd104[GUN_B].charge_full_time_left = BMS_BCS_Context[GUN_B].RemainderTime;
//		SendHYCmd104[GUN_B].charged_sec = puserinfo->ChargeTime * 60;
//		SendHYCmd104[GUN_B].cum_charge_money_amount =  puserinfo->TotalBill;		//累计充电金额
//		SendHYCmd104[GUN_B].cum_charge_kwh_amount = puserinfo->TotalPower;			//累计充电电量
//		SendHYCmd104[GUN_B].before_charge_meter_kwh_num = pmeter->StartPower;		//开始充电读表
//		SendHYCmd104[GUN_B].now_meter_kwh_num = pmeter->CurPower;					//当前充电读表
//		SendHYCmd104[GUN_B].charge_power_kw = SendHYCmd104[GUN_B].dc_charge_voltage * SendHYCmd104[GUN_B].dc_charge_current / 10; 
//		memcpy(SendHYCmd104[GUN_B].charge_user_id,RecvHYCmd7[GUN_B].charge_user_id,32);		//订单号
//	}
//	else
//	{
//		;
//	}
	return ZHFreamSend(zh_cmd_type_104,(INT8U*)&SendZHCmd104[GUN_B],sizeof(zh_cmd_104));
}
/*******************************非周期性发送数据*****************************/
/*****************************************************************************
* Function     : ZH_SendRateAck
* Description  : 费率设置应答
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   ZH_SendRateAck(INT8U cmd) 
{
	NOP();
	return TRUE;
}


/*****************************************************************************
* Function     : ZH_SendStartAck
* Description  : 启动应答  
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U   ZH_SendStartAck(_GUN_NUM gun)
{
	NOP();
	return TRUE;
}






/*****************************************************************************
* Function     : PreZHBill
* Description  : 保存汇誉订单
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2021年1月12日
*****************************************************************************/
INT8U   PreZHBill(_GUN_NUM gun,INT8U *pdata)
{
	NOP();
	return TRUE;
}

/*****************************************************************************
* Function     : ZH_SendBill
* Description  : 发送订单
* Input        :
* Output       : 
* Return       :
* Note(s)      :
* Contributor  : 2021年3月19日
*****************************************************************************/
INT8U ZH_SendBill(_GUN_NUM gun)
{
	NOP();
	return TRUE;
}


/******************************************接收函数*******************************************/
/*****************************************************************************
* Function     : HFQG_RecvFrameDispose
* Description  : 合肥乾古接收帧处理
* Input        : void *pdata
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年7月27日
*****************************************************************************/
INT8U   ZH_RecvFrameDispose(INT8U * pdata,INT16U len)
{
	NOP();
	return TRUE;
}


///*****************************************************************************
//* Function     : HY_RecvDevStopAck
//* Description  : 设备主动停止应答
//* Input        :
//* Output       : 
//* Return       :
//* Note(s)      :
//* Contributor  : 2021年3月19日
//*****************************************************************************/
//static INT8U   HY_RecvDevStopAck(INT8U *pdata,INT16U len)
//{
//	return TRUE;
//}

///*****************************************************************************
//* Function     : HY_RecvDevFailAck
//* Description  : 故障状态上报应答
//* Input        :
//* Output       : 
//* Return       :
//* Note(s)      :
//* Contributor  : 2021年3月19日
//*****************************************************************************/
//static INT8U   HY_RecvDevFailAck(INT8U *pdata,INT16U len)
//{
//	return TRUE;
//}


/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

