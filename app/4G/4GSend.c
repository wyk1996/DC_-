/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
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
#include "HYFrame.h"
#include "4GRecv.h"
#include <string.h>
#include "sysconfig.h"
#include "HFQGFrame.h"
#include "APFrame.h"
#include "DisplayMain.h"

/* Private define-----------------------------------------------------------------------------*/
#define   GPRSSEND_Q_LEN  								20
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
static void *GPRSSENDOSQ[GPRSSEND_Q_LEN];					// 消息队列
static OS_EVENT *GPRSSendTaskEvent;				            // 使用的事件
/*****************************************************************************
* Function     : APP_GetSendTaskEvent
* Description  : 获取发送任务事件
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
OS_EVENT * APP_GetSendTaskEvent(void)
{
	return GPRSSendTaskEvent;
}

INT8U OFFlineBuf[300];		//离线交易记录写入读取换成
INT8U OFFFSlineBuf[300];	//离线分时交易记录写入读取换成

/*****************************************************************************
* Function     : Task4GSend
* Description  : 4G发送任务
* Input        : void
* Output       : None
* Return       :
* Note(s)      :
* Contributor  : 2018年6月14日       
*****************************************************************************/
void Task4GSend(void *pdata)
{
    INT8U err,i;    
    _BSP_MESSAGE *pMsg;
	_DISP_CONTROL* pdisp_conrtol = APP_GetDispControl();
	static INT32U curtime = 0,lasttime = 0;
	INT8U* pBillGunA = APP_GetBillInfo(GUN_A);
	INT8U* pBillGunB = APP_GetBillInfo(GUN_B);
	GPRSSendTaskEvent = OSQCreate(GPRSSENDOSQ, GPRSSEND_Q_LEN);//创建显示接收消息队列
	if (GPRSSendTaskEvent == NULL)
	{
		OSTaskSuspend(OS_PRIO_SELF);    //挂起当前任务
		return;
	}
	OSTimeDly(SYS_DELAY_100ms);

	while(1)
    {
		if((APP_GetSIM7600Mode() == MODE_HTTP)  || (APP_GetSIM7600Mode() == MODE_FTP))   //远程升级其他无关数据帧都不不要发送和处理
		{
			OSTimeDly(SYS_DELAY_1000ms);
			continue; //未连接上服务器，AT指令处理
		}
		pMsg = OSQPend(GPRSSendTaskEvent, SYS_DELAY_1s, &err);
		curtime = OSTimeGet();
		if ((OS_ERR_NONE == err) && (pMsg->MsgID == BSP_4G_MAIN) )
		{
			if((pMsg->DivNum != BSP_4G_SENDNET1) && (pMsg->DivNum != BSP_4G_SENDNET2))
			{
				NOP();
			}
		//所有帧的发送只能再发送函数中，接收函数不能直接发送！
			switch (pMsg->DivNum)
            {	
            	case APP_START_ACK:		//开始充电应答
            		if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            		{
            			if(pMsg->DataLen < GUN_MAX) //枪号保护
            			{
            				_4G_SendStartAck((_GUN_NUM)pMsg->DataLen);
							if(NetConfigInfo[SYSSet.NetYXSelct].NetNum > 1)
							{
								ZF_SendStartCharge();				//发送启动帧
							}
            			}
            		}
            		break;

            	case APP_STOP_ACK:		//停止充电应答
            		if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            		{
            			if(pMsg->DataLen < GUN_MAX) //枪号保护
            			{
            				_4G_SendStopAck((_GUN_NUM)pMsg->DataLen);
            				//HFQG_SendBill((_GUN_NUM)pMsg->DataLen);    //发送结算
            			}
            		}
            		break;
            	case APP_STOP_BILL:		//停止结算
            		//if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
            		//{
				
            			if(pMsg->DataLen < GUN_MAX) //枪号保护
            			{
							if(SYSSet.NetYXSelct == XY_AP)
							{
								if(APP_GetStartNetState(pMsg->DataLen) == NET_STATE_ONLINE)
								{
									//在线记录
									WriterFmBill((_GUN_NUM)pMsg->DataLen,1);
									APP_SetResendBillState(pMsg->DataLen,TRUE);
									_4G_SendBill((_GUN_NUM)pMsg->DataLen);
								}
								else
								{
									//不管A,B枪的离线交易记录都用一个，统一用A
									ResendBillControl[GUN_A].OffLineNum = APP_GetNetOFFLineRecodeNum();
									if(ResendBillControl[GUN_A].OffLineNum < 100)
									{
										ResendBillControl[GUN_A].OffLineNum++;			//离线交易记录+1
										APP_SetNetOFFLineRecodeNum(ResendBillControl[GUN_A].OffLineNum);
										PreAPOffLineBill((_GUN_NUM)pMsg->DataLen,OFFlineBuf); 		//读取离线交易记录
										PreAPFSOffLineBill((_GUN_NUM)pMsg->DataLen,OFFFSlineBuf);		//离线分时交易记录
										APP_RWNetOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_WRITE,OFFlineBuf);
										APP_RWNetFSOFFLineRecode(ResendBillControl[GUN_A].OffLineNum,FLASH_ORDER_WRITE,OFFFSlineBuf);
									}
								}
							}
							else
							{
							//OSTimeDly(SYS_DELAY_3s);
								WriterFmBill((_GUN_NUM)pMsg->DataLen,1);
								APP_SetResendBillState(pMsg->DataLen,TRUE);
								_4G_SendBill((_GUN_NUM)pMsg->DataLen);
								
								_4G_SetStartType((_GUN_NUM)pMsg->DataLen,_4G_APP_START);			//设置为卡启动
							}
						}
            		//}
						if(NetConfigInfo[SYSSet.NetYXSelct].NetNum > 1)
						{
							ZF_SendBill();
						}
            		break;
				case	APP_STE_DEVS:     //查询设备状态
					_4G_SendDevState((_GUN_NUM)pMsg->DataLen);			//发送设备状态
					break;
				case	APP_UPDADA_BALANCE:     //更新余额
					if((SYSSet.NetYXSelct == XY_ZSH)||(SYSSet.NetYXSelct == XY_YKC)||(SYSSet.NetYXSelct == XY_TT)|| (SYSSet.NetYXSelct == XY_YL3)|| (SYSSet.NetYXSelct == XY_YL4)|| (SYSSet.NetYXSelct == XY_XJ)||(SYSSet.NetYXSelct == XY_YL1)) 
					{
						YKC_SendBalanceAck((_GUN_NUM)pMsg->DataLen);			//更新余额
					}
					break;
				case	APP_SET_ACK:     //设置返回
					if(SYSSet.NetYXSelct == XY_66)
					{
						_66_SendQuerySetAck();			//设置返回
					}
					break;
				
				case	APP_SJDATA_QUERY:     //读取实时数据
					if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK)) //连接上服务器
					{
						if(pMsg->DataLen == GUN_A)
						{
							YKC_SendSJDataGunACmd13();
						}	
						if(pMsg->DataLen == GUN_B)
						{
							YKC_SendSJDataGunBCmd13();
						}	
					}
					break;
				case	APP_STE_BILL:     //查询订单
					if(pMsg->DataLen == GUN_A)
					{
						ReSendBill(GUN_A,pBillGunA,1);
					}
					else
					{
						ReSendBill(GUN_B,pBillGunB,1);
					}
					break;
				case APP_RATE_ACK:		//停止结算
            		_4G_SendRateAck(pMsg->DataLen);	 //费率设置应答		
            		break;
				case APP_QUERY_RATE:	//费率请求
					_4G_SendQueryRate();
					break;
				case APP_RATE_MODE:		//计费模型验证
					_4G_SendRateMode();
					break;
				case APP_STE_RATE:			//费率应答
					HY_SendQueryRateAck();
					break;
				case APP_STE_TIME:			//校准时间
					_4G_SendSetTimeAck();
				break;
				case APP_UPDATA_ACK:			//远程升级应答
					_4G_SendUpdataAck();
				break;
				case APP_QR_CODE:			//二维码应答
					YKC_SendAPPQR_Ack((_GUN_NUM)pMsg->DataLen);
				break;
				case APP_CARDVIN_CHARGE:
					_4G_SendCardVinCharge((_GUN_NUM)pMsg->DataLen);   //刷卡启动上报
					break;
				case APP_CARD_INFO:						//卡鉴权
					_4G_SendCardInfo((_GUN_NUM)pMsg->DataLen);
					break;
				
				case APP_VIN_INFO:						//Vin鉴权
					if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
					{
						
						HY_SendVinInfo((_GUN_NUM)pMsg->DataLen,1);  //启动充电
					}
					else
					{
						_4G_SendVinInfo((_GUN_NUM)pMsg->DataLen);
					}
					break;
				case	APP_VIN_INFO_START:
					if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
					{
						
						HY_SendVinInfo((_GUN_NUM)pMsg->DataLen,0);  //只是发送VIN信息
					}
					break;
				case APP_CARD_WL:			//卡白名单
					if(SYSSet.NetYXSelct == XY_AP)
					{
						AP_SendCardWLAck();
					}
					break;
				case APP_VIN_WL:			//·	VIN白名单
					if(SYSSet.NetYXSelct == XY_AP)
					{
						AP_SendVinWLAck();
					}
					break;
				case APP_VINCARD_RES:
					if(SYSSet.NetYXSelct == XY_AP)
					{
						AP_SendVinCardResAck();
					}
					break;
				case APP_OFFLINE_ACK:
					if(SYSSet.NetYXSelct == XY_AP)
					{
						if(ResendBillControl[GUN_A].OffLineNum)
						{
							//离线记录应答了 -1
							ResendBillControl[GUN_A].OffLineNum--;
							APP_SetNetOFFLineRecodeNum(ResendBillControl[GUN_A].OffLineNum);
						}
					}
					break;
				case BSP_4G_SENDNET1:
					if(NetConfigInfo[SYSSet.NetYXSelct].NetNum > 1)
					{
						Send_AT_CIPRXGET(0);   //主动读取数据
					}
					break;
				case BSP_4G_SENDNET2:
					if(NetConfigInfo[SYSSet.NetYXSelct].NetNum > 1)
					{
						Send_AT_CIPRXGET(1);	//主动读取数据
					}
					break;
				default:
					break;
				
			}
		}
		for(i = 0;i < NetConfigInfo[SYSSet.NetYXSelct].NetNum;i++)
		{
			if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(i) == STATE_OK)) //连接上服务器
			{
				//发送数据给服务器
				//ModuleSIM7600_SendData(i, (INT8U*)"hello word qiangge\r\n", strlen("hello word qiangge\r\n"));
				if(i == 0)
				{
					_4G_SendFrameDispose();  //合肥乾古周期性发送帧
				}
				else
				{
					ZF_SendFrameDispose();			
				}
			}
		}
		if (NetConfigInfo[SYSSet.NetYXSelct].NetNum > 1)
		{
			//每10s读取一次信号强度
			if((curtime >= lasttime) ? ((curtime - lasttime) >= SYS_DELAY_10s) : \
					((curtime + (0xffffffffU - lasttime)) >= SYS_DELAY_10s))
			{
				lasttime = curtime;
				Send_AT_CSQ();				//读取信号强度
			}
		}
		//处理发送订单
		for(i = 0;i < GUN_MAX;i++)
		{
			if(APP_GetResendBillState(i) == TRUE)
			{
				if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK) && (APP_GetAppRegisterState(0) == STATE_OK)	) //连接上服务器
				{
					if(i == GUN_A)
					{
						ReSendBill(i,pBillGunA,0);
					}
					else
					{
					//	WriterFmBill(1,1);
						ReSendBill(i,pBillGunB,0);
					}
				}
			}
			if((APP_GetSIM7600Status() == STATE_OK) && (APP_GetModuleConnectState(0) == STATE_OK) && (APP_GetAppRegisterState(0) == STATE_OK)	) //连接上服务器
			{
				if(SYSSet.NetYXSelct == XY_AP)
				{
					ReSendOffLineBill();      //离线交易记录获取
				}
			}


		}
	}	
}


/************************(C)COPYRIGHT 2020 杭州汇誉*****END OF FILE****************************/

