/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: controlmain.c
* Author			: 叶喜雨     
* Date First Issued	:  
* Version			: V0.1
* Description		: 与电源监控模块的通讯任务，实现对电源模块的控制过程
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //    		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "ucos_ii.h"
#include "sysconfig.h"
#include "bsp.h"
#include "gbstandard.h"
#include "controlmain.h"
#include "connectbms.h"
#include "bmstimeout.h"
#include "DisplayMain.h"
#include "PCUMain.h"
#include  "ADSamplemain.h"
#include "charging_Interface.h"
#include "4GMain.h"
#include "app_conf.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
//告警灯状态
typedef enum
{
    ERR_LEDECLOSED = 0,  	//灭
    ERR_LEDOPENED = 1,      //亮
} ERR_LED_STATE;


/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
OS_EVENT  *Control_PeventA;        //消息队列
OS_EVENT  *Control_PeventB;        //消息队列
void *ControlOSQA[CONTROL_Q_NUM];  //控制任务消息队列容量
void *ControlOSQB[CONTROL_Q_NUM];  //控制任务消息队列容量
_SYS_STATE 			SysState[GUN_MAX];			//系统状态
_CONTROL_STEP 		ControlStep[GUN_MAX];
_CCU_DEMAND_INFO 		ManDemandInfo[GUN_MAX];  	//用户手动设置需求
_CCU_DEMAND_INFO 		BMSDemandInfo[GUN_MAX]; 	//发往PCU任务的需求信息
_CONTROL_INFO 		ControlInfo[GUN_MAX];  	 	//充电控制信息
_PRESET_VALUE 		PresetValue;		//预设值
BCU_YX_ImFormaTion BCURemoteSignal[GUN_MAX];   //此数组实际上就是BCUYX_BYTE1--BCUYX_BYTE5的内容
extern _MSP_DI MSPIo;
extern _ADC_JUST_INFO ADCJustInfo[GUN_MAX];
extern _PCU_CONTROL PcuControl;
static const _CONTROL_CMD ControlCmdTableA[CMD_TABLE_NUM] =
{
    //来自BMS任务
    {APP_ELEC_LOCKED       ,  APP_ElectronicLockLockedGunA      },  //电子锁锁定

    {APP_ELEC_UNLOCKED     ,  APP_ElectronicLockUnlockedGunA    },  //电子锁解锁

    {APP_INSULATIONG_CHECK ,  APP_InsulationCheckGunA           },  //绝缘检测命令

    {APP_BCP_FIT           ,  APP_BCPFitCheckGunA             },  //BCP参数检测

    {APP_CHARGE_READY      ,  APP_ChargeReadyCheckGunA          },  //充电机准备检测

    {APP_POWER_OUTPUT      ,  APP_ChargeRunningGunA             },  //开始充电，让功率模块输出电能

    {APP_CHARGE_PAUSE	   ,  APP_ChargePauseGunA               },  //暂停或继续充电

    {APP_CHARGE_BSD        ,  APP_ChargeBSDGunA                 },  //BSD信息

    {APP_BMS_CHARGE_ERR_A  ,  APP_StopChargeErrAGunA            },  //错误处理A,结束充电
    
	{APP_BMS_CHARGE_ERR_B  ,  APP_StopChargeErrBGunA            },  //错误处理B,结束充电
	
	{APP_BMS_CHARGE_ERR_C  ,  APP_StopChargeErrCGunA            },  //错误处理C,

    {APP_CHARGE_FAIL       ,  APP_SelfCheckFailGunA             },  //充电失败
    
    //来自BCU任务
    {APP_CHARGE_START      ,  APP_StartChargeGunA               },  //开始充电

    {APP_CHARGE_END        ,  APP_StopChargeGunA                },  //结束充电
};

static const _CONTROL_CMD ControlCmdTableB[CMD_TABLE_NUM] =
{
    //来自BMS任务
    {APP_ELEC_LOCKED       ,  APP_ElectronicLockLockedGunB      },  //电子锁锁定

    {APP_ELEC_UNLOCKED     ,  APP_ElectronicLockUnlockedGunB    },  //电子锁解锁

    {APP_INSULATIONG_CHECK ,  APP_InsulationCheckGunB           },  //绝缘检测命令

    {APP_BCP_FIT           ,  APP_BCPFitCheckGunB               },  //BCP参数检测

    {APP_CHARGE_READY      ,  APP_ChargeReadyCheckGunB          },  //充电机准备检测

    {APP_POWER_OUTPUT      ,  APP_ChargeRunningGunB             },  //开始充电，让功率模块输出电能

    {APP_CHARGE_PAUSE	   ,  APP_ChargePauseGunB               },  //暂停或继续充电

    {APP_CHARGE_BSD        ,  APP_ChargeBSDGunB                 },  //BSD信息

    {APP_BMS_CHARGE_ERR_A  ,  APP_StopChargeErrAGunB            },  //错误处理A,结束充电
    
	{APP_BMS_CHARGE_ERR_B  ,  APP_StopChargeErrBGunB            },  //错误处理B,结束充电
	
	{APP_BMS_CHARGE_ERR_C  ,  APP_StopChargeErrCGunB            },  //错误处理C,

    {APP_CHARGE_FAIL       ,  APP_SelfCheckFailGunB             },  //充电失败
    
    //来自BCU任务
    {APP_CHARGE_START      ,  APP_StartChargeGunB               },  //开始充电

    {APP_CHARGE_END        ,  APP_StopChargeGunB                },  //结束充电
};

/*****************************************************************************
* Function      : APP_GetPresetValue
* Description   : 获取预设值
* Input         : None 
* Output        : _PRESET_VALUE 		PresetValue
* Note(s)       : 
* Contributor   : 2016年10月17日  Yxy
*****************************************************************************/
_PRESET_VALUE	APP_GetPresetValue(void)
{
	return	PresetValue;
}

/*****************************************************************************
* Function      : APP_GetSystemState
* Description   : 获取系统工作状态
* Input         : None 
* Output        : SysState.WorkState
* Note(s)       : 
* Contributor   : 2016年10月26日  Yxy
*****************************************************************************/
//_WORK_STATE     APP_GetSystemState(INT8U gun)
//{
//	if(gun >= GUN_MAX)
//	{
//		return WORK_STATE_IDLE;
//	}
//	return	SysState[gun].WorkState;
//}
/*****************************************************************************
* Function      : APP_Get_ERR_Branch
* Description   : 获取停止原因小项
* Input         : None 
* Output        : SysState.ErrBranch
* Note(s)       : 
* Contributor   : 2017年1月6日  Yxy
*****************************************************************************/
_CONTROL_ERR_BRANCH APP_Get_ERR_Branch(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return STOP_BSTOTHERERR;
	}
	return	SysState[gun].ErrBranch;
}
/*****************************************************************************
* Function      : APP_Set_ERR_Branch
* Description   : 设置停止原因小项
* Input         : _CONTROL_ERR_BRANCH 
* Output        : None
* Note(s)       : 
* Contributor   : 2017年1月6日  Yxy
*****************************************************************************/
void APP_Set_ERR_Branch(INT8U gun,_CONTROL_ERR_BRANCH Err_Branch_Value)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
	SysState[gun].ErrBranch=Err_Branch_Value;
}

/*****************************************************************************
* Function      : APP_GetCMLContext
* Description   : 获取输出能力
* Input         : _POWER_OUTPUT_PARA *para  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月15日  叶喜雨
*****************************************************************************/
INT8U APP_GetCMLContext(_BMS_CML_CONTEXT *context)
{
    _PRESET_VALUE		PresetValue_Tmp;
    
    if (context == NULL)
    {
        return FALSE;
    }
	
	PresetValue_Tmp=APP_GetPresetValue();
    //最大输出电压
    context->MaxOutVol = PresetValue_Tmp.VolMaxOutput;
    //最小输出电压
    context->MinOutVol = PresetValue_Tmp.VolMinOutput;
    //最大输出电流
    context->MaxOutCur = 4000 - PresetValue_Tmp.CurMaxOutput;
    //最小输出电流
    context->MinOutCur = 4000;
    
    return TRUE;
}

/*****************************************************************************
* Function      : APP_GetCSSContext
* Description   : 获取CCS报文内容
* Input         : _BMS_CCS_CONTEXT *context  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月15日  叶喜雨
*****************************************************************************/
INT8U APP_GetCSSContext(_GUN_NUM gun,_BMS_CCS_CONTEXT *pcontext)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
    if ((pcontext == NULL) || (gun >= GUN_MAX))
    {
        return FALSE;
    }
	pcontext->OutputVol = PowerModuleInfo[gun].OutputInfo.Vol;
	pcontext->OutputCur = 4000 - (INT16U)(PowerModuleInfo[gun].OutputInfo.Cur);
	
	pcontext->TotalChargeTime = puserinfo->ChargeTime;
    if(APP_GetGBType(gun) == BMS_GB_2015) 
    {
    //    pcontext->ChargePause = 0x01;  //暂时先填允许状态。 后面后特殊的再充电过程中需要暂停态时再增加
        if (GetChargePauseState(gun) == FALSE) //允许充电
        {
            pcontext->ChargePause = 0x01; 
        }
        else //暂停充电
        {
            pcontext->ChargePause = 0x00; 
        }
        pcontext->ChargePause |= 0xFC;
    }
    return TRUE;
}

/*****************************************************************************
* Function      : APP_GetCSTContext
* Description   : 获取CST报文内容
* Input         : _BMS_CST_CONTEXT *context  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月15日  叶喜雨
*****************************************************************************/
INT8U APP_GetCSTContext(_GUN_NUM gun,_BMS_CST_CONTEXT *context)
{
    if ((context == NULL) ||(gun>= GUN_MAX))
    {
        return FALSE;
    }
    //将系统状态停止原因直接赋给入参
    *context = SysState[gun].StopReason.ChargeStop;
    return TRUE;
}

/*****************************************************************************
* Function      : APP_GetCSDContext
* Description   : 获取CSD报文内容
* Input         : _BMS_CSD_CONTEXT *context  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月15日  叶喜雨
*****************************************************************************/
INT8U APP_GetCSDContext(_GUN_NUM gun,_BMS_CSD_CONTEXT *context)
{
	USERINFO * puserinfo  = GetChargingInfo(gun);
    if (context == NULL)
    {
        return FALSE;
    }


	context->TotalChargeTime = puserinfo->ChargeTime;
	context->PowerOut = puserinfo->TotalPower4/1000;

    //充电机编号
    if(APP_GetGBType(gun) == BMS_GB_2015)
    {
        context->ChargeSN = 8;//样机目前先统一为8号机器
    }
    return TRUE;
}


/*****************************************************************************
* Function      : GetGunState
* Description   : 获取枪硬件连接状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
_GUN_STATE GetGunState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return GUN_DISCONNECTED;
	}
    return SysState[gun].RemoteMeterState.GunState;
}

/*****************************************************************************
* Function      : SetGunState
* Description   : 设置枪硬件连接状态
* Input         : _GUN_STATE state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
void SetGunState(INT8U gun,_GUN_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    SysState[gun].RemoteMeterState.GunState = state;
}

/*****************************************************************************
* Function      : GetStartFailType
* Description   : 获取启动失败类型
* Input         : void  
* Output        : _START_FAIL_TYPE
* Note(s)       : 
* Contributor   : 2016年10月21日  Yxy
*****************************************************************************/
_START_FAIL_TYPE GetStartFailType(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return ENDFAIL_OTHERERR;
	}
	return	SysState[gun].StartFailType;
}
/*****************************************************************************
* Function      : SetStartFailType
* Description   : 设置启动失败类型
* Input         : _START_FAIL_TYPE FailType 
* Output        : None
* Note(s)       : 
* Contributor   : 2016年10月21日  Yxy
*****************************************************************************/
void SetStartFailType(INT8U gun,_START_FAIL_TYPE FailType)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
	SysState[gun].StartFailType=FailType;
	
}

/*****************************************************************************
* Function      : GetPowerDCState
* Description   : 获取直流输出接触器K1\K2的状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月14日  叶喜雨
*****************************************************************************/
_POWER_DC_STATE GetPowerDCState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return POWER_DC_CLOSE;
	}
    return SysState[gun].RemoteSingalState.PowerDCState;
}

/*****************************************************************************
* Function      : SetPowerDCState
* Description   : 设置直流输出接触器K1\K2的状态
* Input         : _POWER_DC_STATE state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月14日  叶喜雨
*****************************************************************************/
void SetPowerDCState(INT8U gun,_POWER_DC_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    SysState[gun].RemoteSingalState.PowerDCState = state;
}

/*****************************************************************************
* Function      : GetBMSStartCharge
* Description   : 获取是否需要充电
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetBMSStartCharge(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].StartCharge;
}

/*****************************************************************************
* Function      : SetBMSStartCharge
* Description   : 设置是否需要充电
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSStartCharge(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].StartCharge = state;
}

/*****************************************************************************
* Function      : GetBMSStopCharge
* Description   : 获取是否需要充电
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetBMSStopCharge(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].StopCharge;
}

/*****************************************************************************
* Function      : SetBMSStopOnce
* Description   : 设置是否正在停止充电
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSStopOnce(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].StopOnce = state;
}

/*****************************************************************************
* Function      : GetBMSStopOnce
* Description   : 获取是否需要充电
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetBMSStopOnce(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].StopOnce;
}

/*****************************************************************************
* Function      : SetBMSStopCharge
* Description   : 设置是否正在停止充电
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSStopCharge(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].StopCharge = state;
}

/*****************************************************************************
* Function      : GetBMSInsulationCheck
* Description   : 获取是否需要绝缘检测
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetBMSInsulationCheck(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].InsulationCheck;
}

/*****************************************************************************
* Function      : SetBMSInsulationCheck
* Description   : 设置是否需要绝缘检测
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSInsulationCheck(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].InsulationCheck = state;
}

/*****************************************************************************
* Function      : GetBMSBCPFitCheck
* Description   : 获取是否需要BCP参数检测
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetBMSBCPFitCheck(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].BCPFitCheck;
}

/*****************************************************************************
* Function      : SetBMSBCPFitCheck
* Description   : 设置是否需要BCP参数检测
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSBCPFitCheck(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].BCPFitCheck = state;
}

/*****************************************************************************
* Function      : GetChargeReady
* Description   : 获取充电机是否就绪
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
_CHARGE_READY_STATE GetChargeReady(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return READY_FAIL;
	}
    return (_CHARGE_READY_STATE)ControlStep[gun].ChargeReady;
}

/*****************************************************************************
* Function      : SetChargeReady
* Description   : 设置充电机就绪
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetChargeReady(INT8U gun,_CHARGE_READY_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].ChargeReady = state;
}

/*****************************************************************************
* Function      : GetBMSSelfCheck
* Description   : 获取BMS是否需要重新自检
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetBMSSelfCheck(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].SelfCheck;
}

/*****************************************************************************
* Function      : SetBMSSelfCheck
* Description   : 设置BMS是否需要重新自检
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetBMSSelfCheck(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].SelfCheck = state;
}

/*****************************************************************************
* Function      : GetChargeRunningState
* Description   : 获取是否已经输出电能
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetChargeRunningState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].ChargeRunning;
}

/*****************************************************************************
* Function      : SetChargeRunningState
* Description   : 设置是否输出电能
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetChargeRunningState(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].ChargeRunning = state;
}

/*****************************************************************************
* Function      : GetChargeDemandUpdateState
* Description   : 获取需求是否更新
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月9日  叶喜雨
*****************************************************************************/
//INT8U GetChargeDemandUpdateState(INT8U gun)
//{
//	if(gun >= GUN_MAX)
//	{
//		return FALSE;
//	}
//    return ControlStep[gun].DemandUpdate;
//}

/*****************************************************************************
* Function      : SetChargeDemandUpdateState
* Description   : 设置需求是否要更新
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月9日  叶喜雨
*****************************************************************************/
void SetChargeDemandUpdateState(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].DemandUpdate = state;
}

/*****************************************************************************
* Function      : GetChargePauseState
* Description   : 获取暂停输出状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
INT8U GetChargePauseState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].PauseCharge;
}

/*****************************************************************************
* Function      : SetChargePauseState
* Description   : 暂停输出
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月3日  叶喜雨
*****************************************************************************/
void SetChargePauseState(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].PauseCharge = state;
}

/*****************************************************************************
* Function      : GetDealFaultOverState
* Description   : 获取处理故障状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月29日  叶喜雨
*****************************************************************************/
INT8U GetDealFaultOverState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    return ControlStep[gun].DealFaultOver;
}

/*****************************************************************************
* Function      : SetDealFaultOverState
* Description   : 设置处理故障状态
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月29日  叶喜雨
*****************************************************************************/
void SetDealFaultOverState(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].DealFaultOver = state;
}

/*****************************************************************************
* Function      : GetBMSConnectStep
* Description   : 获取BMS握手步骤
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月29日  叶喜雨
*****************************************************************************/
//INT8U GetBMSConnectStep(INT8U gun)
//{
//	if(gun >= GUN_MAX)
//	{
//		return FALSE;
//	}
//    return ControlStep[gun].BMSStep;
//}

/*****************************************************************************
* Function      : SetBMSConnectStep
* Description   : 设置BMS握手步骤
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月29日  叶喜雨
*****************************************************************************/
void SetBMSConnectStep(INT8U gun,INT8U state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    ControlStep[gun].BMSStep = state;
}

/*****************************************************************************
* Function      : GetElecLockState
* Description   : 获取电子锁硬件状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
_ELEC_LOCK_STATE GetElecLockState(INT8U gun)
{

	if (ENBLE_JG)
	{
		return ELEC_LOCK_LOCKED;
	}
	else
	{
		if(SYSSet.LockState == 1)   //常开
		{
			return SysState[gun].RemoteSingalState.ElecLockState;
		}
		else
		{
			if( SysState[gun].RemoteSingalState.ElecLockState == ELEC_LOCK_LOCKED)
			{
				return ELEC_LOCK_UNLOCKED;
			}

			else
			{
				return ELEC_LOCK_LOCKED;
			}
		}
	}
}

/*****************************************************************************
* Function      : SetElecLockState
* Description   : 设置电子锁硬件状态
* Input         : _ELEC_LOCK_STATE state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
void SetElecLockState(INT8U gun,_ELEC_LOCK_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    SysState[gun].RemoteSingalState.ElecLockState = state;
}

/*****************************************************************************
* Function      : GetEmergencyState
* Description   : 获取急停按钮状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月6日  叶喜雨
*****************************************************************************/
_EMERGENCY_STATE GetEmergencyState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return EMERGENCY_PRESSED;
	}
    return SysState[gun].RemoteSingalState.EmergencyState;
}

/*****************************************************************************
* Function      : SetEmergencyState
* Description   : 设置急停按钮状态
* Input         : _EMERGENCY_STATE state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月6日  叶喜雨
*****************************************************************************/
void SetEmergencyState(INT8U gun,_EMERGENCY_STATE state)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    SysState[gun].RemoteSingalState.EmergencyState = state;
}


/*****************************************************************************
* Function      : GetBatState
* Description   : 获取枪归位状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月17日  叶喜雨
*****************************************************************************/
//_GUN_RETURN_STATE GetGunReturnState(INT8U gun)
//{
//	if(gun >= GUN_MAX)
//	{
//		return GUN_UNRETURN;
//	}
//    return SysState[gun].RemoteSingalState.GunReturnState;
//}

/*****************************************************************************
* Function      : GetBatState
* Description   : 获取枪归位状态
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月17日  叶喜雨
*****************************************************************************/
//void SetGunReturnState(INT8U gun,_GUN_RETURN_STATE state)
//{
//	if(gun >= GUN_MAX)
//	{
//		return;
//	}
//    SysState[gun].RemoteSingalState.GunReturnState = state;
//}


/*****************************************************************************
* Function      : GetChargeMode
* Description   : 获取充电模式，分为自动跟手动
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月4日  叶喜雨
*****************************************************************************/
_CHARGE_MODE_TYPE GetChargeMode(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return MODE_UNDEF;
	}
    return ControlInfo[gun].ChargeMode;
}

/*****************************************************************************
* Function      : SetChargeMode
* Description   : 设置充电模式，分手动跟自动
* Input         : _CHARGE_MODE_TYPE mode  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月4日  叶喜雨
*****************************************************************************/
void SetChargeMode(INT8U gun,_CHARGE_MODE_TYPE mode)
{
	if(gun >= GUN_MAX)
	{
		return ;
	}
    ControlInfo[gun].ChargeMode = mode;
}

/*****************************************************************************
* Function      : SetBcuRemoteSignalState
* Description   : 设置BCU遥信状态
* Input         : INT8U point---BCU_YXPara.BCURemoteSignal[]数组的脚标
                  INT8U bit-----在一个字节里的位    
                  INT8U val-----该位的值    
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月14日  叶喜雨
*****************************************************************************/
INT8U SetBcuRemoteSignalState(INT8U gun,INT8U point, INT8U bit, INT8U val)
{
	INT8U temp[7];
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	OSSchedLock();
	memcpy(temp,&BCURemoteSignal[gun],7);
    if ( (point >= sizeof(temp)) || (bit > 7) || ( (val != 1) && (val != 0) )  || (gun >= GUN_MAX))
    {
        return FALSE;
    }
    if (val)
    {
        temp[point] |= (1 << bit);
    }
    else
    {
         temp[point]  &= ~(1 << bit);
    }
	memcpy(&BCURemoteSignal[gun],temp,7);
	OSSchedUnlock();
    return TRUE;
}
/*****************************************************************************
* Function      : APP_BatChargeAdjust
* Description   : 对电池充电过程调整，暂时只是对电流做最简单的均分调整
* Input         : INT16U vol--最终需要输出的电压  
                  INT16U cur--最终需要输出的电流  
                  INT8U seq---分成几次去调整
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月4日  叶喜雨
*****************************************************************************/
void APP_BatChargeAdjust(INT8U gun,INT16U vol, INT16U cur, INT8U seq)
{
    INT8U i, temp;
    static _CCU_DEMAND_INFO DemandInfo[GUN_MAX];
    static _BSP_MESSAGE send_message[GUN_MAX];

	
	if(gun >= GUN_MAX)
	{
		return ;
	}
    DemandInfo[gun].Vol = vol;
    temp = cur / seq;
    send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
    send_message[gun].DivNum = APP_POWER_ENABLE;
    send_message[gun].GunNum = gun;  //枪号
    send_message[gun].pData = (INT8U *)&DemandInfo[gun].Vol;
    for (i = 1; i < seq; i++)
    {
        DemandInfo[gun].Cur = temp * i;
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
        OSTimeDly(SYS_DELAY_50ms);
    }
    DemandInfo[gun].Cur = cur;
    OSQPost(TaskPCU_pevent ,&send_message[gun]);
    OSTimeDly(SYS_DELAY_100ms);
	printf("Power Model Set Adjust\r\n");
}


/*****************************************************************************
* Function      : SetChargeInfo
* Description   : 设置充电信息字段
* Input         : INT8U offset  
                  void *pdata   
                  INT8U len     
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U SetChargeInfo(INT8U gun,INT8U offset, void *pdata, INT8U len)
{
    if ( (pdata == NULL) || (!len) )
    {
        return FALSE;
    }
    switch (offset)
    {
        case 0:
            if (len != sizeof(_BSPRTC_TIME))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.StartTime, (const void *)pdata, len);
            return TRUE;
        case 1:
            if (len != sizeof(_BSPRTC_TIME))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.EndTime, (const void *)pdata, len);
            return TRUE;
        case 2:
            if (len != sizeof(_POWER_VAL))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.StartPower, (const void *)pdata, len);
            return TRUE;
        case 3:
            if (len != sizeof(_POWER_VAL))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.EndPower, (const void *)pdata, len);
            return TRUE;
        case 4:
            if (len != sizeof(_POWER_VAL))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.CostPower, (const void *)pdata, len);
            return TRUE;
        case 5:
            if (len != sizeof(_COST_VAL))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.Cost, (const void *)pdata, len);
            return TRUE;
        case 6:
            if (len != sizeof(_BMS_BSD_CONTEXT))
            {
                return FALSE;
            }
            memcpy((void *)&SysState[gun].ChargeInfo.BSDContext, (const void *)pdata, len);
            return TRUE;
        default:
            return FALSE;
        
    }
}



/*****************************************************************************
* Function      : APP_ElectronicLock
* Description   : 电子锁锁定
* Input         : void *pdata
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_ElectronicLockLockedGunA(void *pdata)
{
	 _BSP_MESSAGE * pdisposedata = pdata;
	if (pdata == NULL)
	{
		return FALSE;
	}

    memset( (INT8U *)&SysState[GUN_A].TotalErrState, 0, \
        (INT8U *)&SysState[GUN_A].RemoteMeterState.GunState - (INT8U *)&SysState[GUN_A].TotalErrState);
    BSP_CloseLOCK(GUN_A);//

	printf("Electronic Lock Locked\r\n");
    return TRUE;
}

/*****************************************************************************
* Function      : APP_ElectronicLock
* Description   : 电子锁锁定
* Input         : void *pdata
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_ElectronicLockLockedGunB(void *pdata)
{
	 _BSP_MESSAGE * pdisposedata = pdata;
	if (pdata == NULL)
	{
		return FALSE;
	}

    memset( (INT8U *)&SysState[GUN_B].TotalErrState, 0, \
        (INT8U *)&SysState[GUN_B].RemoteMeterState.GunState - (INT8U *)&SysState[GUN_B].TotalErrState);
    BSP_CloseLOCK(GUN_B);//

	printf("Electronic Lock Locked\r\n");
    return TRUE;
}

/*****************************************************************************
* Function      : APP_ElectronicLockUnlocked
* Description   : 电子锁解锁
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
INT8U APP_ElectronicLockUnlockedGunA(void *pdata)
{
	 _BSP_MESSAGE * pdisposedata = pdata;

	if (pdata == NULL)
	{
		return FALSE;
	}
	
    //解锁电子锁
    //关闭低压辅助电源
  	BSP_CloseBMSPower(GUN_A);
	BSP_CloseLOCK(GUN_A);
	printf("Electronic Lock Unlocked\r\n");
    return TRUE;
}

/*****************************************************************************
* Function      : APP_ElectronicLockUnlocked
* Description   : 电子锁解锁
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
INT8U APP_ElectronicLockUnlockedGunB(void *pdata)
{
	 _BSP_MESSAGE * pdisposedata = pdata;
	if (pdata == NULL)
	{
		return FALSE;
	}
	
    //解锁电子锁
    //关闭低压辅助电源
  	BSP_CloseBMSPower(GUN_B);
	BSP_CloseLOCK(GUN_B);
	printf("Electronic Lock Unlocked\r\n");
    return TRUE;
}


/*****************************************************************************
* Function      : APP_SelfCheckFail
* Description   : 自检失败，充电失败
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
INT8U APP_SelfCheckFailGunA(void *pdata)
{


    static _BSP_MESSAGE send_message[GUN_MAX];
	 _BSP_MESSAGE * pdisposedata = pdata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();

	if( (pdata == NULL) || (pdisevent == NULL))
	{
		return FALSE;
	}
	
   	send_message[GUN_A].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[GUN_A].DivNum = APP_CHARGE_FAIL; //启动失败
	send_message[GUN_A].GunNum = GUN_A;
	OSQPost(pdisevent, &send_message[GUN_A]);
	OSTimeDly(SYS_DELAY_10ms);
	SetBMSStartCharge(GUN_A,FALSE);
	printf("App Self Check Fail\r\n");
	BSP_CloseBMSPower(GUN_A);
	BSP_CloseLOCK(GUN_A);
    return TRUE;
}


/*****************************************************************************
* Function      : APP_SelfCheckFail
* Description   : 自检失败，充电失败
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
INT8U APP_SelfCheckFailGunB(void *pdata)
{


    static _BSP_MESSAGE send_message[GUN_MAX];
	 _BSP_MESSAGE * pdisposedata = pdata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
	if( (pdata == NULL) || (pdisevent == NULL))
	{
		return FALSE;
	}
	
   	send_message[GUN_B].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[GUN_B].DivNum = APP_CHARGE_FAIL; //启动失败
	send_message[GUN_B].GunNum = GUN_B;
	OSQPost(pdisevent, &send_message[GUN_B]);
	OSTimeDly(SYS_DELAY_10ms);
	SetBMSStartCharge(GUN_B,FALSE);
	printf("App Self Check Fail\r\n");
	BSP_CloseBMSPower(GUN_B);
	BSP_CloseLOCK(GUN_B);
    return TRUE;
}

/*****************************************************************************
* Function      : APP_SelfCheckEnd
* Description   : 结束自检
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_SelfCheckEnd(void *pdata)
{
    return TRUE;
}


extern _DISP_CONTROL DispControl;       				//显示任务控制结构体
/*****************************************************************************
* Function      : APP_StartCharge
* Description   : 开始充电
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_StartChargeGunA(void *pdata)
{
	_START_CHARGE_CMD * pCharge;
	_START_CHARGE_CMD ChargeInfo[GUN_MAX] = {(_CHARGE_MODE_TYPE)0};
    static _BSP_MESSAGE send_message[GUN_MAX];
	 _BSP_MESSAGE * pdisposedata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
	INT8U gun;
	if( (pdata == NULL) || (pdisevent == NULL))
	{
		return FALSE;
	}
	pdisposedata = ( _BSP_MESSAGE *)pdata;
	pCharge = (_START_CHARGE_CMD *)pdisposedata->pData;

	gun = GUN_A;
	memcpy(&ChargeInfo[gun],pCharge,sizeof(_START_CHARGE_CMD));

	
	
 
    if (  ( (GetBMSStopCharge(gun) == TRUE) && (GetBMSStartCharge(gun) == TRUE) ) )
    {//pMsg为空、正在停止中、开始充电中,PCU未反馈禁能，其他错误
        SysState[gun].SelfCheckState.Other = STATE_ERR;
        SysState[gun].SelfCheckState.NoErr = STATE_ERR;

		SetStartFailType(gun,ENDFAIL_OTHERERR);
		NB_WriterReason(gun,"F0",2);
		send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_CHARGE_FAIL; //启动失败
		send_message[gun].GunNum = gun;
        OSQPost(pdisevent, &send_message[gun]);
        OSTimeDly(SYS_DELAY_10ms);
		SetBMSStartCharge(gun,FALSE);
		printf("123123");
        return FALSE;
    }
	memset(ChargeRecodeInfo[gun].StopReason,0,sizeof(ChargeRecodeInfo[gun].StopReason));			//停止原因清零 主要用于内部人员解决问题使用
	if (GetBMSStartCharge(gun) == FALSE)
    {//开始充电成功,绝缘检测清0
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 0);
        BMS_FrameContextClear(gun);
		BMS_CONNECT_ControlInit(gun);	//重新初始化
	
		SetBMSConnectStep(gun,BMS_CONNECT_DEFAULTVAL);
		//需要重新自检
	    SetBMSSelfCheck(gun,TRUE);
	    //需要检测BCP参数
	    SetBMSBCPFitCheck(gun,TRUE);
		//充电允许
	    SetChargePauseState(gun,FALSE);
	    //充电未结束
	    SetBMSStopCharge(gun,FALSE);
	    //充电机未准备好
	    SetChargeReady(gun,READY_ING);
		//未输出电能
		SetChargeRunningState(gun,FALSE);
		//充电灯关闭
		BSP_CloseRunLed(gun);
		//故障灯关闭
		BSP_CloseFailLed(gun);

		//系统状态全为正常
		memset((INT8U *)&SysState[gun].TotalErrState, 0, \
				   (INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
		//手动充电模式清0
		memset( (INT8U *)&ManDemandInfo[gun].Vol, 0, sizeof(_CCU_DEMAND_INFO) );
		//PCU任务的需求信息清0
		memset( (INT8U *)&BMSDemandInfo[gun].Vol, 0, sizeof(_CCU_DEMAND_INFO) );
		
		//功率模块相关信息清0
		//memset( (INT8U *)&PowerModuleInfo.OutputInfo.Vol, 0, sizeof(_POWER_MODULE_INFO));
		
		memset(&CanFrameControl[gun], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[gun], 0x00, sizeof(_BMS_RTS_FRAME_INF)); 
    }
	if(ChargeInfo[gun].ChargeMode == MODE_MAN)
	{
		if (GetBMSStartCharge(gun) == TRUE)
		{//已经在充电了
//			if ( (ManDemandInfo[gun].Vol != ChargeInfo[gun].ChargeVol) \
//				|| (ManDemandInfo[gun].Cur !=  ChargeInfo[gun].ChargeCur) )
//			{//保存新的需求
				ManDemandInfo[gun].Vol = ChargeInfo[gun].ChargeVol;
				ManDemandInfo[gun].Cur =  ChargeInfo[gun].ChargeCur;
				//更新了需求
				SetChargeDemandUpdateState(gun,TRUE);
				printf("Manual Demand Update Done\r\n");
//			}
		}
    //不管是否已经在充电了，手动模式可以实时调整电压、电流
      
		else
		{
			
			//提取手动充电的电压
			ManDemandInfo[gun].Vol = ChargeInfo[gun].ChargeVol;
			ManDemandInfo[gun].Cur =  ChargeInfo[gun].ChargeCur;
			//手动模式
			SetChargeMode(gun,MODE_MAN); 
			//系统状态全为正常
			memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
				(INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
			//打开低压辅助电源
			BSP_OpenBMSPower(gun);
			SetBMSStartCharge(gun,TRUE);  //开始充电
			SetBMSStopOnce(gun,FALSE);
		}
		send_message[gun].GunNum = gun;
		if(gun == GUN_A)
		{
			APP_ChargeRunningGunA(&send_message[gun]); //通知模块开始充电
		}
		else
		{
			APP_ChargeRunningGunB(&send_message[gun]); //通知模块开始充电
		}
		printf("Manual Mode Start Done\r\n");
	}
	
	else //自动 VIN模式
	{
		//枪未连接
		if (GetGunState(gun) == GUN_DISCONNECTED)
		{
			//枪未连接
			SysState[gun].SelfCheckState.GunDisconnect = STATE_ERR;
			SysState[gun].SelfCheckState.NoErr = STATE_ERR;
			SetStartFailType(gun,ENDFAIL_HANDERR);
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			NB_WriterReason(gun,"F1",2);
			send_message[gun].DivNum = APP_CHARGE_FAIL; //启动失败
			send_message[gun].GunNum = gun;
			OSQPost(pdisevent, &send_message[gun]);
			OSTimeDly(SYS_DELAY_10ms);
			printf("1114\r\n");
			SetBMSStartCharge(gun,FALSE);
			BSP_CloseBMSPower(gun);
			return FALSE; 
		}
		BSP_OpenLOCK(gun);   //打开电子锁
		BSP_OpenBMSPower(gun);
		if(ChargeInfo[gun].ChargeMode == MODE_VIN)
		{
			SetChargeMode(gun,MODE_VIN); //VIN模式
		}
		else
		{
			SetChargeMode(gun,MODE_AUTO); //自动模式
		}
		BSP_CloseK1K2(gun);	 //保险起见，开始充电把接触器断一下
		SetBMSStartCharge(gun,TRUE);  //开始充电
		SetBMSStopOnce(gun,FALSE);
		DispControl.StopOverFrame[gun] = STATE_UNOVER;   //未结算
		printf("Auto Mode Start Done\r\n");
	}
    return TRUE;
}

/*****************************************************************************
* Function      : APP_StartCharge
* Description   : 开始充电
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_StartChargeGunB(void *pdata)
{
	_START_CHARGE_CMD * pCharge;
	_START_CHARGE_CMD ChargeInfo[GUN_MAX] = {(_CHARGE_MODE_TYPE)0};
    static _BSP_MESSAGE send_message[GUN_MAX];
	 _BSP_MESSAGE * pdisposedata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
	INT8U gun;
	if( (pdata == NULL) || (pdisevent == NULL))
	{
		return FALSE;
	}
	pdisposedata = ( _BSP_MESSAGE *)pdata;
	pCharge = (_START_CHARGE_CMD *)pdisposedata->pData;
	gun =pdisposedata->GunNum;
	
	gun = GUN_B;
	memcpy(&ChargeInfo[gun],pCharge,sizeof(_START_CHARGE_CMD));

	
	
 
    if (  ( (GetBMSStopCharge(gun) == TRUE) && (GetBMSStartCharge(gun) == TRUE) ) )
    {//pMsg为空、正在停止中、开始充电中,PCU未反馈禁能，其他错误
        SysState[gun].SelfCheckState.Other = STATE_ERR;
        SysState[gun].SelfCheckState.NoErr = STATE_ERR;

		SetStartFailType(gun,ENDFAIL_OTHERERR);
		
		send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		NB_WriterReason(gun,"F2",2);
		send_message[gun].DivNum = APP_CHARGE_FAIL; //启动失败
		send_message[gun].GunNum = gun;
        OSQPost(pdisevent, &send_message[gun]);
        OSTimeDly(SYS_DELAY_10ms);
		SetBMSStartCharge(gun,FALSE);
		printf("123123");
        return FALSE;
    }
	memset(ChargeRecodeInfo[gun].StopReason,0,sizeof(ChargeRecodeInfo[gun].StopReason));			//停止原因清零 主要用于内部人员解决问题使用
	if (GetBMSStartCharge(gun) == FALSE)
    {//开始充电成功,绝缘检测清0
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 0);
        BMS_FrameContextClear(gun);
		BMS_CONNECT_ControlInit(gun);	//重新初始化
	
		SetBMSConnectStep(gun,BMS_CONNECT_DEFAULTVAL);
		//需要重新自检
	    SetBMSSelfCheck(gun,TRUE);
	    //需要检测BCP参数
	    SetBMSBCPFitCheck(gun,TRUE);
		//充电允许
	    SetChargePauseState(gun,FALSE);
	    //充电未结束
	    SetBMSStopCharge(gun,FALSE);
	    //充电机未准备好
	    SetChargeReady(gun,READY_ING);
		//未输出电能
		SetChargeRunningState(gun,FALSE);
		//充电灯关闭
		BSP_CloseRunLed(gun);
		//故障灯关闭
		BSP_CloseFailLed(gun);

		//系统状态全为正常
		memset((INT8U *)&SysState[gun].TotalErrState, 0, \
				   (INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
		//手动充电模式清0
		memset( (INT8U *)&ManDemandInfo[gun].Vol, 0, sizeof(_CCU_DEMAND_INFO) );
		//PCU任务的需求信息清0
		memset( (INT8U *)&BMSDemandInfo[gun].Vol, 0, sizeof(_CCU_DEMAND_INFO) );
		
		//功率模块相关信息清0
		//memset( (INT8U *)&PowerModuleInfo.OutputInfo.Vol, 0, sizeof(_POWER_MODULE_INFO));
		
		memset(&CanFrameControl[gun], 0x00, sizeof(_CANFRAMECONTROL));
		memset(&BMS_RTS_FrameInf[gun], 0x00, sizeof(_BMS_RTS_FRAME_INF)); 
    }
	if(ChargeInfo[gun].ChargeMode == MODE_MAN)
	{
		if (GetBMSStartCharge(gun) == TRUE)
		{//已经在充电了
//			if ( (ManDemandInfo[gun].Vol != ChargeInfo[gun].ChargeVol) \
//				|| (ManDemandInfo[gun].Cur !=  ChargeInfo[gun].ChargeCur) )
//			{//保存新的需求
				ManDemandInfo[gun].Vol = ChargeInfo[gun].ChargeVol;
				ManDemandInfo[gun].Cur =  ChargeInfo[gun].ChargeCur;
				//更新了需求
				SetChargeDemandUpdateState(gun,TRUE);
				printf("Manual Demand Update Done\r\n");
//			}
		}
    //不管是否已经在充电了，手动模式可以实时调整电压、电流
      
		else
		{
			
			//提取手动充电的电压
			ManDemandInfo[gun].Vol = ChargeInfo[gun].ChargeVol;
			ManDemandInfo[gun].Cur =  ChargeInfo[gun].ChargeCur;
			//手动模式
			SetChargeMode(gun,MODE_MAN); 
			//系统状态全为正常
			memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
				(INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
			//打开低压辅助电源
			BSP_OpenBMSPower(gun);
			SetBMSStartCharge(gun,TRUE);  //开始充电
			SetBMSStopOnce(gun,FALSE);
		}
		send_message[gun].GunNum = gun;
		if(gun == GUN_A)
		{
			APP_ChargeRunningGunA(&send_message[gun]); //通知模块开始充电
		}
		else
		{
			APP_ChargeRunningGunB(&send_message[gun]); //通知模块开始充电
		}
		printf("Manual Mode Start Done\r\n");
	}
	
	else //自动 VIN模式
	{
		//枪未连接
		if (GetGunState(gun) == GUN_DISCONNECTED)
		{
			//枪未连接
			SysState[gun].SelfCheckState.GunDisconnect = STATE_ERR;
			SysState[gun].SelfCheckState.NoErr = STATE_ERR;
			NB_WriterReason(gun,"F3",2);
			SetStartFailType(gun,ENDFAIL_HANDERR);
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			send_message[gun].DivNum = APP_CHARGE_FAIL; //启动失败
			send_message[gun].GunNum = gun;
			OSQPost(pdisevent, &send_message[gun]);
			OSTimeDly(SYS_DELAY_10ms);
			printf("1114\r\n");
			SetBMSStartCharge(gun,FALSE);
			BSP_CloseBMSPower(gun);
			return FALSE; 
		}
		BSP_OpenLOCK(gun);   //打开电子锁
		BSP_OpenBMSPower(gun);
		if(ChargeInfo[gun].ChargeMode == MODE_VIN)
		{
			SetChargeMode(gun,MODE_VIN); //VIN模式
		}
		else
		{
			SetChargeMode(gun,MODE_AUTO); //自动模式
		}
		SetBMSStartCharge(gun,TRUE);  //开始充电
		SetBMSStopOnce(gun,FALSE);
		DispControl.StopOverFrame[gun] = STATE_UNOVER;   //未结算
		printf("Auto Mode Start Done\r\n");
	}
    return TRUE;
}



/*****************************************************************************
* Function      : APP_StopCharge
* Description   : 停止充电
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeGunA(void *pdata)
{

	OS_EVENT* pdisevent =  APP_GetDispEvent();
	static _BSP_MESSAGE send_message[GUN_MAX],send_messagebms[GUN_MAX],send_messagedip[GUN_MAX];
     _BSP_MESSAGE* ptr = (_BSP_MESSAGE *)pdata;
	INT8U gun;
	
    if ( ptr == NULL  )
    {
        return FALSE;
    }
	gun	= GUN_A;

	//保险  主要进入这里不管如何都要关闭模块
	  //通知PCU停止输出
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
	if (GetBMSStopOnce(gun) == TRUE) //已经停止充电
    {
        return FALSE;
    }
    
    if (GetBMSStopCharge(gun) == FALSE)
    {
        SetBMSStopCharge(gun,TRUE);  //正在停止充电
		SetBMSStartCharge(gun,FALSE);//未开始充电
		printf("656654655\r\n");
        SetChargeReady(gun,READY_ING);   //充电机未准备好
  		OSTimeDly(SYS_DELAY_5ms);
      
        //通知BMS结束通讯
		send_messagebms[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_messagebms[gun].DivNum = APP_CHARGE_END;
		send_messagebms[gun].DataLen = sizeof(_SYS_STATE);
		send_messagebms[gun].pData = (INT8U *)&SysState[gun].TotalErrState;
		if(gun == GUN_A)
		{
			OSQPost(TaskConnectBMS_peventA, &send_messagebms[gun]);
		}
		else
		{
			OSQPost(TaskConnectBMS_peventB, &send_messagebms[gun]);
		}
		OSTimeDly(SYS_DELAY_5ms);
        if (ERR_CHECK(ptr->pData[0]) == STOP_ERR) //出现故障停止
        {
            NOP();
        }
        else
        {
            BMS_TIMEOUT_ENTER(gun,BMS_OTH2_STEP, SYS_DELAY_100ms);
            //输出电流是否为5A以下     
			while (PowerModuleInfo[gun].OutputInfo.Cur >= DEF_OUTPUT_CUR)
            {
                 OSTimeDly(SYS_DELAY_5ms);
                //超时100ms
                if (BMS_TIMEOUT_CHECK(gun,BMS_OTH2_STEP, SYS_DELAY_100ms) == BMS_TIME_OUT)
                {
                    break;
                }
            }
        }
        //(1)断开充电机的DC继电器K1\K2
		
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
        //(3)断开辅助电源K3\K4
       BSP_CloseBMSPower(gun);
		
		
        //正常情况下DC继电器外侧电压会在60V以下，延时为了让内侧电压都投入泄放电路
        if (ERR_CHECK(ptr->pData[0]) == STOP_ERR) //出现故障停止
        {
           OSTimeDly(SYS_DELAY_250ms);
        }
        else
        {
			//K1/K2已经断开，这里测后级的电压只是为了防止枪头上依然有大于60V的电压，不管泄放 
			//这里的1s是强制泄放1s，以保证关机后，模块的残余电量得到完全泄放，如果屏蔽掉，就存在
			//残余电量泄放不完的情况（剩余200V左右就不泄放）
            OSTimeDly(SYS_DELAY_1s);
            BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
            //判断DC继电器外侧电压是否小于60V
            while (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
            //while(PowerModuleInfo.OutputInfo.Vol >= VAL_ADJUST(60, 10, 1)) 
            {
                OSTimeDly(SYS_DELAY_5ms);
                if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s) == BMS_TIME_OUT)
                {
                     ChargeErrDeal(gun,ERR_LF_METHOD_A);
                     break;
                }
            }
        }

	
	BSP_CloseLOCK(gun);
	//(5)关闭充电指示灯
	BSP_CloseRunLed(gun); 
		
		
        //(6)如有错误,还需要打开故障灯 ERR_CHECK
        if (ERR_CHECK(ptr->pData[0]) == STOP_ERR) //出现故障停止
        {
//          BSP_RLCOpen(BSPRLC_ERR_LED); 
//			printf("ERR LED 2,%d\r\n",ptr->pData[0]);
        }
        else if (USER_CHECK(ptr->pData[0]) == STOP_USER) //用户主动停止
        {
            //用户主动停止充电
            SysState[gun].StopReason.ChargeStop.State0.OneByte.UserStopBits = STATE_TROUBLE;
			if(SysState[gun].TotalErrState==ERR_NORMAL_TYPE)
			{
				SysState[gun].TotalErrState |= ERR_USER_STOP_TYPE;
			}
			printf("User Stop2\r\n");
        }
        //保存充电结束时间
        SysState[gun].ChargeInfo.CurTick = OSTimeGet();
        BSP_RTCGetTime((_BSPRTC_TIME *)&SysState[gun].ChargeInfo.EndTime.Second);
		
        //未输出电能
        SetChargeRunningState(gun,FALSE);
        //充电模式设置为未定义
        SetChargeMode(gun,MODE_UNDEF);
        SetBMSStopCharge(gun,FALSE);  //未进行停止充电操作

        //通知显示任务
		send_messagedip[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_messagedip[gun].DivNum = APP_CHARGE_END; //结算充电
		send_messagedip[gun].GunNum = gun;
		OSQPost(pdisevent, &send_messagedip[gun]);
		
  
        OSTimeDly(SYS_DELAY_10ms);
        SetBMSStopOnce(gun,TRUE);
		printf("Stop\r\n");
    }
    SetDealFaultOverState(gun,TRUE);
	
	
	 //未输出电能   20220902 安全起见，再操作一次
	SetChargeRunningState(gun,FALSE);
	BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
	BSP_CloseRunLed(gun); 				//关闭充电指示灯
    return TRUE;
}


/*****************************************************************************
* Function      : APP_StopCharge
* Description   : 停止充电
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeGunB(void *pdata)
{

	OS_EVENT* pdisevent =  APP_GetDispEvent();
	static _BSP_MESSAGE send_message[GUN_MAX],send_messagebms[GUN_MAX],send_messagedip[GUN_MAX];
     _BSP_MESSAGE* ptr = (_BSP_MESSAGE *)pdata;
	INT8U gun;
	
    if ( ptr == NULL  )
    {
        return FALSE;
    }
	gun	= GUN_B;

	//保险  主要进入这里不管如何都要关闭模块
	  //通知PCU停止输出
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
	if (GetBMSStopOnce(gun) == TRUE) //已经停止充电
    {
        return FALSE;
    }
    
    if (GetBMSStopCharge(gun) == FALSE)
    {
        SetBMSStopCharge(gun,TRUE);  //正在停止充电
		SetBMSStartCharge(gun,FALSE);//未开始充电
		printf("656654655\r\n");
        SetChargeReady(gun,READY_ING);   //充电机未准备好
  		OSTimeDly(SYS_DELAY_5ms);
      
        //通知BMS结束通讯
		send_messagebms[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_messagebms[gun].DivNum = APP_CHARGE_END;
		send_messagebms[gun].DataLen = sizeof(_SYS_STATE);
		send_messagebms[gun].pData = (INT8U *)&SysState[gun].TotalErrState;
		if(gun == GUN_A)
		{
			OSQPost(TaskConnectBMS_peventA, &send_messagebms[gun]);
		}
		else
		{
			OSQPost(TaskConnectBMS_peventB, &send_messagebms[gun]);
		}
		OSTimeDly(SYS_DELAY_5ms);
        if (ERR_CHECK(ptr->pData[0]) == STOP_ERR) //出现故障停止
        {
            NOP();
        }
        else
        {
            BMS_TIMEOUT_ENTER(gun,BMS_OTH2_STEP, SYS_DELAY_100ms);
            //输出电流是否为5A以下     
			while (PowerModuleInfo[gun].OutputInfo.Cur >= DEF_OUTPUT_CUR)
            {
                 OSTimeDly(SYS_DELAY_5ms);
                //超时100ms
                if (BMS_TIMEOUT_CHECK(gun,BMS_OTH2_STEP, SYS_DELAY_100ms) == BMS_TIME_OUT)
                {
                    break;
                }
            }
        }
        //(1)断开充电机的DC继电器K1\K2
		
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
        //(3)断开辅助电源K3\K4
       BSP_CloseBMSPower(gun);
		
		
        //正常情况下DC继电器外侧电压会在60V以下，延时为了让内侧电压都投入泄放电路
        if (ERR_CHECK(ptr->pData[0]) == STOP_ERR) //出现故障停止
        {
           OSTimeDly(SYS_DELAY_250ms);
        }
        else
        {
			//K1/K2已经断开，这里测后级的电压只是为了防止枪头上依然有大于60V的电压，不管泄放 
			//这里的1s是强制泄放1s，以保证关机后，模块的残余电量得到完全泄放，如果屏蔽掉，就存在
			//残余电量泄放不完的情况（剩余200V左右就不泄放）
            OSTimeDly(SYS_DELAY_1s);
            BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
            //判断DC继电器外侧电压是否小于60V
            while (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
            //while(PowerModuleInfo.OutputInfo.Vol >= VAL_ADJUST(60, 10, 1)) 
            {
                OSTimeDly(SYS_DELAY_5ms);
                if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s) == BMS_TIME_OUT)
                {
                     ChargeErrDeal(gun,ERR_LF_METHOD_A);
                     break;
                }
            }
        }

	
	BSP_CloseLOCK(gun);
	//(5)关闭充电指示灯
	BSP_CloseRunLed(gun); 
		
		
        //(6)如有错误,还需要打开故障灯 ERR_CHECK
        if (ERR_CHECK(ptr->pData[0]) == STOP_ERR) //出现故障停止
        {
//          BSP_RLCOpen(BSPRLC_ERR_LED); 
//			printf("ERR LED 2,%d\r\n",ptr->pData[0]);
        }
        else if (USER_CHECK(ptr->pData[0]) == STOP_USER) //用户主动停止
        {
            //用户主动停止充电
            SysState[gun].StopReason.ChargeStop.State0.OneByte.UserStopBits = STATE_TROUBLE;
			if(SysState[gun].TotalErrState==ERR_NORMAL_TYPE)
			{
				SysState[gun].TotalErrState |= ERR_USER_STOP_TYPE;
			}
			printf("User Stop2\r\n");
        }
        //保存充电结束时间
        SysState[gun].ChargeInfo.CurTick = OSTimeGet();
        BSP_RTCGetTime((_BSPRTC_TIME *)&SysState[gun].ChargeInfo.EndTime.Second);
		
        //未输出电能
        SetChargeRunningState(gun,FALSE);
        //充电模式设置为未定义
        SetChargeMode(gun,MODE_UNDEF);
        SetBMSStopCharge(gun,FALSE);  //未进行停止充电操作

        //通知显示任务
		send_messagedip[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_messagedip[gun].DivNum = APP_CHARGE_END; //结算充电
		send_messagedip[gun].GunNum = gun;
		OSQPost(pdisevent, &send_messagedip[gun]);
		
  
        OSTimeDly(SYS_DELAY_10ms);
        SetBMSStopOnce(gun,TRUE);
		printf("Stop\r\n");
    }
    SetDealFaultOverState(gun,TRUE);
	
	
	 //未输出电能   20220902 安全起见，再操作一次
	SetChargeRunningState(gun,FALSE);
	BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
	BSP_CloseRunLed(gun); 				//关闭充电指示灯
    return TRUE;
}

/*****************************************************************************
* Function      : APP_StopChargeErr
* Description   : 错误A的处理方式
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月27日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeErrAGunA(void *pdata)
{
	_BSP_MESSAGE *pMsg = NULL;
	INT8U gun;
	static _BSP_MESSAGE send_message[GUN_MAX];
	if(pdata == NULL)
	{
		return FALSE;
	}
	OSSchedLock();
	pMsg = pdata;
	gun = GUN_A;
	OSSchedUnlock();
	
	//通知PCU停止输出
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;		
	OSQPost(TaskPCU_pevent, &send_message[gun]);

	BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
	SetPowerDCState(gun,POWER_DC_OPEN);
	BMS_CONNECT_StepSet(gun,BMS_SEND_DEFAULT);			//不再发送BMS帧
    BSP_CloseBMSPower(gun);			
	BSP_CloseLOCK(gun);	//电子锁解锁
    BSP_CloseRunLed(gun); 				//关闭充电指示灯
	SetBMSStartCharge(gun,FALSE);						//未开始充电
	printf("6566655\r\n");
	SetChargeReady(gun,READY_ING);   						//充电机未准备好
	SetChargeRunningState(gun,FALSE);					//未输出电能
	SetChargeMode(gun,MODE_UNDEF);						//充电模式设置为未定义
    SetBMSSelfCheck(gun,TRUE);							//需要插拔枪重新自检
    SetBMSStopCharge(gun,FALSE);						//停止充电完成
    SetDealFaultOverState(gun,TRUE);
	return TRUE;
}
/*****************************************************************************
* Function      : APP_StopChargeErr
* Description   : 错误A的处理方式
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月27日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeErrAGunB(void *pdata)
{
	_BSP_MESSAGE *pMsg = NULL;
	INT8U gun;
	static _BSP_MESSAGE send_message[GUN_MAX];
	if(pdata == NULL)
	{
		return FALSE;
	}
	OSSchedLock();
	pMsg = pdata;
	gun = GUN_B;
	OSSchedUnlock();
	
	//通知PCU停止输出
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;		
	OSQPost(TaskPCU_pevent, &send_message[gun]);

	BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
	SetPowerDCState(gun,POWER_DC_OPEN);
	BMS_CONNECT_StepSet(gun,BMS_SEND_DEFAULT);			//不再发送BMS帧
    BSP_CloseBMSPower(gun);			
	BSP_CloseLOCK(gun);	//电子锁解锁
    BSP_CloseRunLed(gun); 				//关闭充电指示灯
	SetBMSStartCharge(gun,FALSE);						//未开始充电
	printf("6566655\r\n");
	SetChargeReady(gun,READY_ING);   						//充电机未准备好
	SetChargeRunningState(gun,FALSE);					//未输出电能
	SetChargeMode(gun,MODE_UNDEF);						//充电模式设置为未定义
    SetBMSSelfCheck(gun,TRUE);							//需要插拔枪重新自检
    SetBMSStopCharge(gun,FALSE);						//停止充电完成
    SetDealFaultOverState(gun,TRUE);
	return TRUE;
}

/*****************************************************************************
* Function      : APP_StopChargeErrB
* Description   : 错误B的处理方式
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月12日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeErrBGunA(void *pdata)
{
    static _BSP_MESSAGE send_message[GUN_MAX], send_dismessage[GUN_MAX];
     _BSP_MESSAGE* ptr = (_BSP_MESSAGE *)pdata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
     INT8U gun;

	OSSchedLock();
	gun = GUN_A;
	OSSchedUnlock();
	
	//通知PCU停止输出   不管如何都要关闭模块
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
    if (GetBMSStopCharge(gun) == FALSE)
    {
        SetBMSStopCharge(gun,TRUE);  //正在停止充电
		SetBMSStartCharge(gun,FALSE);//未开始充电
        SetChargeReady(gun,READY_ING);   //充电机未准备好
		printf("GUN = %d,654655\r\n",gun);
        //输出电流是否为5A以下    
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_100ms);
		while (PowerModuleInfo[gun].OutputInfo.Cur >= DEF_OUTPUT_CUR)
        {
            OSTimeDly(SYS_DELAY_5ms);
            //超时100ms
			
				if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_100ms) == BMS_TIME_OUT)
				{
					break;
				}
		
        }
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
        //(2)泄放电路动作
		//正常情况下DC继电器外侧电压会在60V以下，延时为了让内侧电压都投入泄放电路
        OSTimeDly(SYS_DELAY_10ms); 
		
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
        //判断DC继电器外侧电压是否小于60V
        while (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
        {
            OSTimeDly(SYS_DELAY_5ms);
		
			 if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s) == BMS_TIME_OUT)
			{
				 ChargeErrDeal(gun,ERR_LF_METHOD_A);
				 break;
			}

        }
		BSP_CloseBMSPower(gun);			
		BSP_CloseLOCK(gun);	//电子锁解锁
		BSP_CloseRunLed(gun); 				//关闭充电指示灯

        OSTimeDly(SYS_DELAY_1s);
        //停止发送CEM
		BMS_CONNECT_StepSet(gun,BMS_SEND_DEFAULT);
        //保存充电结束时间
        SysState[gun].ChargeInfo.CurTick = OSTimeGet();
        BSP_RTCGetTime((_BSPRTC_TIME *)&SysState[gun].ChargeInfo.EndTime.Second);

        //未输出电能
        SetChargeRunningState(gun,FALSE);
        //充电模式设置为未定义
        SetChargeMode(gun,MODE_UNDEF);
        SetBMSStopCharge(gun,FALSE);  //未进行停止充电操作

        //通知BCU任务
		send_dismessage[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_dismessage[gun].DivNum = APP_CHARGE_END; //结算充电
		send_dismessage[gun].GunNum = gun;
        OSQPost(pdisevent, &send_dismessage[gun]);
		
    
		printf("Stop ErrorB\r\n");
    }
	BMS_CONNECT_StateSet(gun,BMS_CONNECT_DEFAULT);//设置当前状态为初始值
    SetBMSSelfCheck(gun,TRUE);                    //需要插拔枪重新自检
    SetBMSStopCharge(gun,FALSE);
    SetDealFaultOverState(gun,TRUE);
	
	
	 //未输出电能   20220902 安全起见，再操作一次
	SetChargeRunningState(gun,FALSE);
	BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
	BSP_CloseRunLed(gun); 				//关闭充电指示灯
    return TRUE;
}

/*****************************************************************************
* Function      : APP_StopChargeErrB
* Description   : 错误B的处理方式
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月12日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeErrBGunB(void *pdata)
{
    static _BSP_MESSAGE send_message[GUN_MAX], send_dismessage[GUN_MAX];
     _BSP_MESSAGE* ptr = (_BSP_MESSAGE *)pdata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
     INT8U gun;

	OSSchedLock();
	gun = GUN_B;
	OSSchedUnlock();
	
	//通知PCU停止输出   不管如何都要关闭模块
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
    if (GetBMSStopCharge(gun) == FALSE)
    {
        SetBMSStopCharge(gun,TRUE);  //正在停止充电
		SetBMSStartCharge(gun,FALSE);//未开始充电
        SetChargeReady(gun,READY_ING);   //充电机未准备好
		printf("GUN = %d,654655\r\n",gun);
        //输出电流是否为5A以下    
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_100ms);
		while (PowerModuleInfo[gun].OutputInfo.Cur >= DEF_OUTPUT_CUR)
        {
            OSTimeDly(SYS_DELAY_5ms);
            //超时100ms
			
				if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_100ms) == BMS_TIME_OUT)
				{
					break;
				}
		
        }
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
        //(2)泄放电路动作
		//正常情况下DC继电器外侧电压会在60V以下，延时为了让内侧电压都投入泄放电路
        OSTimeDly(SYS_DELAY_10ms); 
		
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
        //判断DC继电器外侧电压是否小于60V
        while (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
        {
            OSTimeDly(SYS_DELAY_5ms);
		
			 if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s) == BMS_TIME_OUT)
			{
				 ChargeErrDeal(gun,ERR_LF_METHOD_A);
				 break;
			}

        }
		BSP_CloseBMSPower(gun);			
		BSP_CloseLOCK(gun);	//电子锁解锁
		BSP_CloseRunLed(gun); 				//关闭充电指示灯

        OSTimeDly(SYS_DELAY_1s);
        //停止发送CEM
		BMS_CONNECT_StepSet(gun,BMS_SEND_DEFAULT);
        //保存充电结束时间
        SysState[gun].ChargeInfo.CurTick = OSTimeGet();
        BSP_RTCGetTime((_BSPRTC_TIME *)&SysState[gun].ChargeInfo.EndTime.Second);

        //未输出电能
        SetChargeRunningState(gun,FALSE);
        //充电模式设置为未定义
        SetChargeMode(gun,MODE_UNDEF);
        SetBMSStopCharge(gun,FALSE);  //未进行停止充电操作

        //通知BCU任务
		send_dismessage[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_dismessage[gun].DivNum = APP_CHARGE_END; //结算充电
		send_dismessage[gun].GunNum = gun;
        OSQPost(pdisevent, &send_dismessage[gun]);
		
    
		printf("Stop ErrorB\r\n");
    }
	BMS_CONNECT_StateSet(gun,BMS_CONNECT_DEFAULT);//设置当前状态为初始值
    SetBMSSelfCheck(gun,TRUE);                    //需要插拔枪重新自检
    SetBMSStopCharge(gun,FALSE);
    SetDealFaultOverState(gun,TRUE);
	
	
	 //未输出电能   20220902 安全起见，再操作一次
	SetChargeRunningState(gun,FALSE);
	BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
	BSP_CloseRunLed(gun); 				//关闭充电指示灯
    return TRUE;
}


/*****************************************************************************
* Function      : APP_StopChargeErrC
* Description   : 错误C的处理方式
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeErrCGunA(void *pdata)
{
    static _BSP_MESSAGE send_message[GUN_MAX];
     _BSP_MESSAGE* ptr = (_BSP_MESSAGE *)pdata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
     INT8U gun;
     
	OSSchedLock();
	if(ptr ==NULL)
	{
		OSSchedUnlock();
		return FALSE;
	}
	gun = GUN_A;
	OSSchedUnlock();
    
	
	//通知PCU停止输出
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
    if (GetBMSStopCharge(gun) == FALSE)
    {
        SetBMSStopCharge(gun,TRUE);  //正在停止充电
        //通知PCU停止输出
        OSTimeDly(SYS_DELAY_20ms);
#if 1
        //输出电流是否为5A以下     
		
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_100ms);
		
		while (PowerModuleInfo[gun].OutputInfo.Cur >= DEF_OUTPUT_CUR)
        {
            OSTimeDly(SYS_DELAY_5ms);
            //超时100ms
			
			if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_100ms) == BMS_TIME_OUT)
			{
				break;
			}
        }
#endif
        //断开充电机的DC继电器K1\K2
        BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
        OSTimeDly(SYS_DELAY_1s);
		//断开K1、K2后不用发送CEM，也不用发送CST
		BMS_CONNECT_StepSet(gun,BMS_SEND_DEFAULT); 
    }
    SetBMSStopCharge(gun,FALSE);
    SetDealFaultOverState(gun,TRUE);
	return TRUE;
}

/*****************************************************************************
* Function      : APP_StopChargeErrC
* Description   : 错误C的处理方式
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_StopChargeErrCGunB(void *pdata)
{
    static _BSP_MESSAGE send_message[GUN_MAX];
     _BSP_MESSAGE* ptr = (_BSP_MESSAGE *)pdata;
	OS_EVENT* pdisevent =  APP_GetDispEvent();
     INT8U gun;
     
	OSSchedLock();
	if(ptr ==NULL)
	{
		OSSchedUnlock();
		return FALSE;
	}
	gun =GUN_B;
	OSSchedUnlock();
    
	
	//通知PCU停止输出
	send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
    if (GetBMSStopCharge(gun) == FALSE)
    {
        SetBMSStopCharge(gun,TRUE);  //正在停止充电
        //通知PCU停止输出
        OSTimeDly(SYS_DELAY_20ms);
#if 1
        //输出电流是否为5A以下     
		
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_100ms);
		
		while (PowerModuleInfo[gun].OutputInfo.Cur >= DEF_OUTPUT_CUR)
        {
            OSTimeDly(SYS_DELAY_5ms);
            //超时100ms
			
			if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_100ms) == BMS_TIME_OUT)
			{
				break;
			}
        }
#endif
        //断开充电机的DC继电器K1\K2
        BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
        OSTimeDly(SYS_DELAY_1s);
		//断开K1、K2后不用发送CEM，也不用发送CST
		BMS_CONNECT_StepSet(gun,BMS_SEND_DEFAULT); 
    }
    SetBMSStopCharge(gun,FALSE);
    SetDealFaultOverState(gun,TRUE);
	return TRUE;
}


/*****************************************************************************
* Function      : APP_InsulationCheck
* Description   : 绝缘检测
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_InsulationCheckGunA(void *pdata)
{
//根据电路原理图所得
#define	 ISO_R0   (double)(5*150000+2000)    //不平衡电阻R0
#define	 ISO_R1   (double)(5*15000+300)     //不平衡电阻R1
	
#define	 ISO_M   (double)(1.000 + ISO_R0/ISO_R1)    //(R1+R0)/R1  = 1+1/N
#define	 ISO_N   (double)(ISO_R1/ISO_R0)     		//R1/R0
#define  JY_RESERR		(50000u + 5000u)		    //绝缘检测故障阻值 误差百分之10

	INT16U modulevol = 0;
   INT8U err = BMS_STEP_DEFAULT;
	INT32U Vbus0 = 0,Viso0 = 0,Vbus1 = 0,Viso1 = 0,Vbus2 = 0,Viso2 = 0;
	INT32U Rp,Rn;			//绝缘检测电阻
		
	static _BSP_MESSAGE send_message[GUN_MAX];
	
    _BMS_BHM_CONTEXT BHMContext;
	_PRESET_VALUE		PresetValue_Tmp;
	_BSP_MESSAGE * pmsg;
	INT8U gun;
	if(pdata == NULL)
	{
		SetStartFailType(GUN_A,ENDFAIL_INSOLUTIONERR);	//绝缘检测失败
		ChargeErrDeal(GUN_A,ERR_LF_METHOD_B);
		SetBMSInsulationCheck(GUN_A,FALSE);//设置绝缘检测完成，不再需要绝缘检测
		printf("INSOLUTIONERR parameter1 err\r\n");
		 NB_WriterReason(GUN_A,"F17",3);
		return FALSE;
	}
	pmsg = pdata;
	gun = GUN_A;
	

	
    _SELF_CHECK_STATE *ptr = (_SELF_CHECK_STATE *)&SysState[gun].SelfCheckState.NoErr;
    

	if (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
	{//通过句1和句2，来对那些一上电就闭合了主开关的车，取消绝缘检测 Test
		ptr->ContactorVol = STATE_ERR;
		ptr->NoErr = STATE_ERR;	
		SetStartFailType(gun,ENDFAIL_BATVOLTERR2);
		ChargeErrDeal(gun,ERR_LF_METHOD_B);
		printf("K1/K2 Batter Vol Large than 30V\r\n");
		SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
		 NB_WriterReason(gun,"F18",3);
		return FALSE;
	}
	
	if (GetRS_BatReverse(gun) == TRUE)
    {
       ptr->ContactorVol = STATE_ERR;
		ptr->NoErr = STATE_ERR;	
		SetStartFailType(gun,ENDFAIL_BATREVERSE);	//电池反接
		ChargeErrDeal(gun,ERR_LF_METHOD_B);
		SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
		 NB_WriterReason(gun,"F19",3);
		printf("电池反接\r\n");
		return FALSE;
    }
   


    //此处一种是发消息给绝缘检测任务
    //一种是直接调用绝缘检测接口
    //(1)绝缘检测电路ON,闭合充电机的DC继电器K1、K2
//    BSP_RLCOpen(BSPRLC_INSULATION); //绝缘检测电路ON
	printf("Insulation Check Begin\r\n");
#if 1
        BMSDemandInfo[gun].Vol = VAL_ADJUST(200, 10, 1);//默认的话，200够了
        BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR; //5.1A
        send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_POWER_ENABLE;
		send_message[gun].GunNum = gun;  //枪号
		send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
  
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
        OSTimeDly(SYS_DELAY_100ms);
        PowerModuleInfo[gun].OutputInfo.Vol = 0;                   //进入故障处理会没清零，导致绝缘检测电压没起来
        //超时20s，20秒内必须要达到200V  ,    可能模块在切换，时间会比较久（之前为10s）
		modulevol = 200;
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_30s);
		while ((PowerModuleInfo[gun].OutputInfo.Vol<= (BMSDemandInfo[gun].Vol-BHMCMP_VOLT))\
        ||(PowerModuleInfo[gun].OutputInfo.Vol>= (BMSDemandInfo[gun].Vol+BHMCMP_VOLT))) 
        {//模块输出电压20s还没有升上来，就超时
            if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
                err = BMS_TIME_OUT;
                ptr->Insulation = STATE_ERR;   //绝缘检测故障
                ptr->NoErr = STATE_ERR;
                SetStartFailType(gun,ENDFAIL_CHARGEMODULEERR);	
				NB_WriterReason(gun,"F20",3);				
                printf("20S TIMEOUT\r\n");
                break;
            }
			modulevol++;
			if(modulevol > 205)
			{
				modulevol = 200;
			}
			BMSDemandInfo[gun].Vol = VAL_ADJUST(modulevol, 10, 1);//默认的话，200够了
			BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR; //5.1A
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			send_message[gun].DivNum = APP_POWER_ENABLE;
			send_message[gun].GunNum = gun;  //枪号
			send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
	  
			OSQPost(TaskPCU_pevent ,&send_message[gun]);
            OSTimeDly(SYS_DELAY_500ms);
        }
		if (err != BMS_TIME_OUT)
		{
			if (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
			{//继电器发生了黏连
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F21",3);		
				SetStartFailType(gun,ENDFAIL_OUTSWERR2);
//				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 7, 1);
	//            BSP_RLCOpen(BSPRLC_ERR_LED);
				printf("DCOUT ERR1\r\n");
					//通知PCU任务停止输出
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
				send_message[gun].DivNum = APP_POWER_DISABLE;
				send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
				send_message[gun].GunNum = gun;  //枪号
				OSQPost(TaskPCU_pevent, &send_message[gun]);
				SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
				OSTimeDly(SYS_DELAY_50ms);
				return FALSE;
			}  
			else
			{
//				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 7, 0);
				//闭合充电机的DC继电器K1、K2
				BSP_OpenK1K2(gun);				
				SetPowerDCState(gun,POWER_DC_CLOSE);
				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
				printf("k1K2 Open\r\n");
			}
		}
#endif
    //(2)绝缘检测用电压输出
    //发送400V 5A的消息给PCU任务，开始输出电能
	 if (err != BMS_TIME_OUT)
	 {
		APP_GetBMSBHM(gun,&BHMContext);
		PresetValue_Tmp=APP_GetPresetValue();
		if (BHMContext.MaxChargeWholeVol <= DEF_OUTPUT_VOL)
		{
			BMSDemandInfo[gun].Vol = DEF_OUTPUT_VOL; //350V
		}
		else if (BHMContext.MaxChargeWholeVol <= PresetValue_Tmp.VolMaxOutput)
		{
			BMSDemandInfo[gun].Vol = BHMContext.MaxChargeWholeVol;
		}
		else
		{
			BMSDemandInfo[gun].Vol = VAL_ADJUST(500, 10, 1);//默认的话，500够了
		}
		BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR; //5A
		send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_POWER_ENABLE;
		send_message[gun].GunNum = gun;  //枪号
		send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
		OSTimeDly(SYS_DELAY_100ms);
		printf("BMSDemandInfo.Vol = %d,BMSDemandInfo.Cur = %d\r\n",BMSDemandInfo[gun].Vol,BMSDemandInfo[gun].Cur);
		PowerModuleInfo[gun].OutputInfo.Vol = 0;                   //进入故障处理会没清零，导致绝缘检测电压没起来
		//超时10s，10秒内必须要达到300V
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_20s);

		while ((PowerModuleInfo[gun].OutputInfo.Vol<= (BMSDemandInfo[gun].Vol-BHMCMP_VOLT))\
			||(PowerModuleInfo[gun].OutputInfo.Vol>= (BMSDemandInfo[gun].Vol+BHMCMP_VOLT))) 
		{//模块输出电压20s还没有升上来，就超时
			if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_20s) == BMS_TIME_OUT)
			{
				err = BMS_TIME_OUT;
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F22",3);		
				SetStartFailType(gun,ENDFAIL_CHARGEMODULEERR);		
				printf("20S TIMEOUT\r\n");
				printf("PowerModuleInfo[gun].OutputInfo.Vol =%d,BMSDemandInfo[gun].Vol =%d\r\n,PowerModuleInfo[gun].OutputInfo.Vol,BMSDemandInfo[gun].Vol");
				break;
			}
			OSTimeDly(SYS_DELAY_500ms);
		}
		
		if (err != BMS_TIME_OUT)
		{//模块在20s内达到了需求电压
			//绝缘检测在此处进行  2020"
			//1、读取Vbus0,Viso0;
			Vbus0 = GetRM_DCOutVolt(gun)/10;
			Viso0 = GetJY_DCOutVolt(gun)/10;
			//2、闭合K300等待1s，读取Vbus1,Viso2;
			BSP_OpenPG1(gun);
			OSTimeDly(SYS_DELAY_1s);
			Vbus1 = GetRM_DCOutVolt(gun)/10;
			Viso1 = GetJY_DCOutVolt(gun)/10;
			//3、闭合K301等待1s，读取Vbus2,Viso2;
			BSP_OpenNG2(gun);
			OSTimeDly(SYS_DELAY_1s);
			Vbus2 = GetRM_DCOutVolt(gun)/10;
			Viso2 = GetJY_DCOutVolt(gun)/10;
			//4、断开K300、K301
		    BSP_ClosePG1(gun);
			BSP_CloseNG2(gun);
			printf("JY Vbus0 = %d,Viso0 = %d,JY Vbus1 = %d,Viso1 = %d,JY Vbus2 = %d,Viso2 = %d\r\n",Vbus0,Viso0,Vbus1,Viso1,Vbus2,Viso2);
			/******************判断是否异常***************************/
			if(((Vbus0*100) > (Viso0*2*110)) || ((Vbus0*100) < (Viso0*2*90)))
			{
				//Vbus0 与 Viso0 * 2 误差大于 10%时判断位绝缘检测回路异常，理论上时分压，刚好一半
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F23",3);		
				SetStartFailType(gun,ENDFAIL_INSOLUTIONERR);	
                SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 1);				
				printf("绝缘检测回路故障\r\n");
			}else if((Viso1 < 100) || (Viso1 > (Vbus1 - 200))){
				//正负母线对地阻值很小时，比较电压后续根据场景可以变动，目前是 小于10v后大于输出电压-10v
				//小于10v后大于输出电压-20v  500V去测量，理论上可以测量出30k的电阻  （150K*5 + 2K||30K）/(150K*5 + 2K) 约等于 25倍数  
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F24",3);		
				SetStartFailType(gun,ENDFAIL_INSOLUTIONERR);	
                SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 1);				
				printf("正负母线对地阻值很小\r\n");
			}else{
			
				//计算结缘检测是否正常
				Rp = ISO_R0 *(((Vbus2 - Viso2)*Viso1 - (Vbus1 - Viso1)*Viso2) / \
				((Vbus1 - Viso1)*Viso2 + (ISO_M - 1) * Viso1 * Viso2- (Vbus2 - Viso2)*Viso1) );
				
				Rn = ISO_R0 *(((Vbus2 - Viso2)*Viso1 - (Vbus1 - Viso1)*Viso2) / \
				(ISO_M * (Vbus1 - Viso1)*Viso2 - (Vbus2 - Viso2)*Viso1) );
				
				printf("JY Rp = %d,Rn = %d\r\n",Rp,Rn);
				if((Rp < JY_RESERR) || (Rn < JY_RESERR) )
				{
					ptr->Insulation = STATE_ERR;   //绝缘检测故障
					ptr->NoErr = STATE_ERR;
					NB_WriterReason(gun,"F25",3);		
					SetStartFailType(gun,ENDFAIL_INSOLUTIONERR);	
					SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 1);				
					printf("绝缘检测故障　\r\n");
				}
			}
			
		}
	}
    //(4)绝缘检测电压输出停止
    //通知PCU任务停止输出
   send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 1;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
    OSTimeDly(SYS_DELAY_50ms);
    //绝缘检测电路OFF
//    BSP_RLCClose(BSPRLC_INSULATION);
    if(APP_GetGBType(gun) == BMS_GB_2011)
    {//旧国标可能存在接触器直接闭合的情况，这样会导致把电池的电进行了泄放
     //(6)断开充电机的DC继电器K1、K2
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
    } //#if (BMS_CUR_GB == BMS_OLD_GB2011)  //旧国标 

    if(APP_GetGBType(gun) == BMS_GB_2015)  //新标
    {
        BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
        //判断K1\K2接触器外侧电压是否小于60v
        while (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1))
        {
            //超时5秒
            if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s))
            {
                ptr->LeakVol = STATE_ERR;
                ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F26",3);
				SetStartFailType(gun,ENDFAIL_LEAKOUTTIMEOUT);
                ChargeErrDeal(gun,ERR_LF_METHOD_A);
				printf("1113\r\n");
                break;
            }
            OSTimeDly(SYS_DELAY_100ms);
        }
        //(6)断开充电机的DC继电器K1、K2
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
    }
    else
    {
        OSTimeDly(SYS_DELAY_1s); 
    }

    SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
    if (ptr->NoErr == STATE_ERR) //有错误，返回错误
    {//新国标对于泄放有时间要求，目前为5s
        return FALSE;
    }
    else
    {
        ptr->NoErr = STATE_NO_ERR;
    }
    return TRUE;
}


/*****************************************************************************
* Function      : APP_InsulationCheck
* Description   : 绝缘检测
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_InsulationCheckGunB(void *pdata)
{
//根据电路原理图所得
#define	 ISO_R0   (double)(5*150000+2000)    //不平衡电阻R0
#define	 ISO_R1   (double)(5*15000+300)     //不平衡电阻R1
	
#define	 ISO_M   (double)(1.000 + ISO_R0/ISO_R1)    //(R1+R0)/R1  = 1+1/N
#define	 ISO_N   (double)(ISO_R1/ISO_R0)     		//R1/R0
#define  JY_RESERR		(50000u + 5000u)		    //绝缘检测故障阻值 误差百分之10

   INT8U err = BMS_STEP_DEFAULT;
	INT32U Vbus0 = 0,Viso0 = 0,Vbus1 = 0,Viso1 = 0,Vbus2 = 0,Viso2 = 0;
	INT32U Rp,Rn;			//绝缘检测电阻
	INT16U	modulevol = 0;
	static _BSP_MESSAGE send_message[GUN_MAX];
	
    _BMS_BHM_CONTEXT BHMContext;
	_PRESET_VALUE		PresetValue_Tmp;
	_BSP_MESSAGE * pmsg;
	INT8U gun;
	if(pdata == NULL)
	{
		SetStartFailType(GUN_B,ENDFAIL_INSOLUTIONERR);	//绝缘检测失败
		ChargeErrDeal(GUN_B,ERR_LF_METHOD_B);
		SetBMSInsulationCheck(GUN_B,FALSE);//设置绝缘检测完成，不再需要绝缘检测
		printf("INSOLUTIONERR parameter1 err\r\n");
		NB_WriterReason(GUN_B,"F6",2);
		return FALSE;
	}
	pmsg = pdata;
	gun = GUN_B;
	

	
    _SELF_CHECK_STATE *ptr = (_SELF_CHECK_STATE *)&SysState[gun].SelfCheckState.NoErr;
    

	if (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
	{//通过句1和句2，来对那些一上电就闭合了主开关的车，取消绝缘检测 Test
		ptr->ContactorVol = STATE_ERR;
		ptr->NoErr = STATE_ERR;	
		SetStartFailType(gun,ENDFAIL_BATVOLTERR2);
		ChargeErrDeal(gun,ERR_LF_METHOD_B);
		printf("K1/K2 Batter Vol Large than 30V\r\n");
		SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
		NB_WriterReason(gun,"F7",2);
		return FALSE;
	}
	
	if (GetRS_BatReverse(gun) == TRUE)
    {
       ptr->ContactorVol = STATE_ERR;
		ptr->NoErr = STATE_ERR;	
		SetStartFailType(gun,ENDFAIL_BATREVERSE);	//电池反接
		ChargeErrDeal(gun,ERR_LF_METHOD_B);
		SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
		printf("电池反接\r\n");
		NB_WriterReason(gun,"F8",2);
		return FALSE;
    }
   


    //此处一种是发消息给绝缘检测任务
    //一种是直接调用绝缘检测接口
    //(1)绝缘检测电路ON,闭合充电机的DC继电器K1、K2
//    BSP_RLCOpen(BSPRLC_INSULATION); //绝缘检测电路ON
	printf("Insulation Check Begin\r\n");
#if 1
        BMSDemandInfo[gun].Vol = VAL_ADJUST(200, 10, 1);//默认的话，200够了
        BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR; //
        send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_POWER_ENABLE;
		send_message[gun].GunNum = gun;  //枪号
		send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
  
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
        OSTimeDly(SYS_DELAY_100ms);
        PowerModuleInfo[gun].OutputInfo.Vol = 0;                   //进入故障处理会没清零，导致绝缘检测电压没起来
        //超时20s，20秒内必须要达到200V  ,    可能模块在切换，时间会比较久（之前为10s）
		modulevol = 200;
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_30s);
		while ((PowerModuleInfo[gun].OutputInfo.Vol<= (BMSDemandInfo[gun].Vol-BHMCMP_VOLT))\
        ||(PowerModuleInfo[gun].OutputInfo.Vol>= (BMSDemandInfo[gun].Vol+BHMCMP_VOLT))) 
        {//模块输出电压20s还没有升上来，就超时
            if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
                err = BMS_TIME_OUT;
                ptr->Insulation = STATE_ERR;   //绝缘检测故障
                ptr->NoErr = STATE_ERR;
                SetStartFailType(gun,ENDFAIL_CHARGEMODULEERR);	
				NB_WriterReason(gun,"F9",2);				
                printf("20S TIMEOUT\r\n");
                break;
            }
			modulevol++;
			if(modulevol > 205)
			{
				modulevol = 200;
			}
			BMSDemandInfo[gun].Vol = VAL_ADJUST(modulevol, 10, 1);//默认的话，200够了
			BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR; //
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			send_message[gun].DivNum = APP_POWER_ENABLE;
			send_message[gun].GunNum = gun;  //枪号
			send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
	  
			OSQPost(TaskPCU_pevent ,&send_message[gun]);
            OSTimeDly(SYS_DELAY_500ms);
        }
		if (err != BMS_TIME_OUT)
		{
			if (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1)) 
			{//继电器发生了黏连
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				SetStartFailType(gun,ENDFAIL_OUTSWERR2);
//				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 7, 1);
	//            BSP_RLCOpen(BSPRLC_ERR_LED);
				printf("DCOUT ERR1\r\n");
					//通知PCU任务停止输出
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
				send_message[gun].DivNum = APP_POWER_DISABLE;
				send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
				send_message[gun].GunNum = gun;  //枪号
				OSQPost(TaskPCU_pevent, &send_message[gun]);
				SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
				NB_WriterReason(gun,"F10",3);
				OSTimeDly(SYS_DELAY_50ms);
				return FALSE;
			}  
			else
			{
//				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 7, 0);
				//闭合充电机的DC继电器K1、K2
				BSP_OpenK1K2(gun);				
				SetPowerDCState(gun,POWER_DC_CLOSE);
				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
				printf("k1K2 Open\r\n");
			}
		}
#endif
    //(2)绝缘检测用电压输出
    //发送400V 5A的消息给PCU任务，开始输出电能
	 if (err != BMS_TIME_OUT)
	 {
		APP_GetBMSBHM(gun,&BHMContext);
		PresetValue_Tmp=APP_GetPresetValue();
		if (BHMContext.MaxChargeWholeVol <= DEF_OUTPUT_VOL)
		{
			BMSDemandInfo[gun].Vol = DEF_OUTPUT_VOL; //350V
		}
		else if (BHMContext.MaxChargeWholeVol <= PresetValue_Tmp.VolMaxOutput)
		{
			BMSDemandInfo[gun].Vol = BHMContext.MaxChargeWholeVol;
		}
		else
		{
			BMSDemandInfo[gun].Vol = VAL_ADJUST(500, 10, 1);//默认的话，500够了
		}
		BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR; //5A
		send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_POWER_ENABLE;
		send_message[gun].GunNum = gun;  //枪号
		send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
		OSTimeDly(SYS_DELAY_100ms);
		printf("BMSDemandInfo.Vol = %d,BMSDemandInfo.Cur = %d\r\n",BMSDemandInfo[gun].Vol,BMSDemandInfo[gun].Cur);
		PowerModuleInfo[gun].OutputInfo.Vol = 0;                   //进入故障处理会没清零，导致绝缘检测电压没起来
		//超时10s，10秒内必须要达到300V
		BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_20s);

		while ((PowerModuleInfo[gun].OutputInfo.Vol<= (BMSDemandInfo[gun].Vol-BHMCMP_VOLT))\
			||(PowerModuleInfo[gun].OutputInfo.Vol>= (BMSDemandInfo[gun].Vol+BHMCMP_VOLT))) 
		{//模块输出电压20s还没有升上来，就超时
			if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_20s) == BMS_TIME_OUT)
			{
				err = BMS_TIME_OUT;
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				SetStartFailType(gun,ENDFAIL_CHARGEMODULEERR);		
				printf("20S TIMEOUT\r\n");
				printf("PowerModuleInfo[gun].OutputInfo.Vol =%d,BMSDemandInfo[gun].Vol =%d\r\n,PowerModuleInfo[gun].OutputInfo.Vol,BMSDemandInfo[gun].Vol");
				NB_WriterReason(gun,"F11",3);
				break;
			}
			OSTimeDly(SYS_DELAY_500ms);
		}
		
		if (err != BMS_TIME_OUT)
		{//模块在20s内达到了需求电压
			//绝缘检测在此处进行  2020"
			//1、读取Vbus0,Viso0;
			Vbus0 = GetRM_DCOutVolt(gun)/10;
			Viso0 = GetJY_DCOutVolt(gun)/10;
			//2、闭合K300等待1s，读取Vbus1,Viso2;
			BSP_OpenPG1(gun);
			OSTimeDly(SYS_DELAY_1s);
			Vbus1 = GetRM_DCOutVolt(gun)/10;
			Viso1 = GetJY_DCOutVolt(gun)/10;
			//3、闭合K301等待1s，读取Vbus2,Viso2;
			BSP_OpenNG2(gun);
			OSTimeDly(SYS_DELAY_1s);
			Vbus2 = GetRM_DCOutVolt(gun)/10;
			Viso2 = GetJY_DCOutVolt(gun)/10;
			//4、断开K300、K301
		    BSP_ClosePG1(gun);
			BSP_CloseNG2(gun);
			printf("JY Vbus0 = %d,Viso0 = %d,JY Vbus1 = %d,Viso1 = %d,JY Vbus2 = %d,Viso2 = %d\r\n",Vbus0,Viso0,Vbus1,Viso1,Vbus2,Viso2);
			/******************判断是否异常***************************/
			if(((Vbus0*100) > (Viso0*2*110)) || ((Vbus0*100) < (Viso0*2*90)))
			{
				//Vbus0 与 Viso0 * 2 误差大于 10%时判断位绝缘检测回路异常，理论上时分压，刚好一半
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F12",3);
				SetStartFailType(gun,ENDFAIL_INSOLUTIONERR);	
                SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 1);				
				printf("绝缘检测回路故障\r\n");
			}else if((Viso1 < 100) || (Viso1 > (Vbus1 - 200))){
				//正负母线对地阻值很小时，比较电压后续根据场景可以变动，目前是 小于10v后大于输出电压-10v
				//小于10v后大于输出电压-20v  500V去测量，理论上可以测量出30k的电阻  （150K*5 + 2K||30K）/(150K*5 + 2K) 约等于 25倍数  
				ptr->Insulation = STATE_ERR;   //绝缘检测故障
				ptr->NoErr = STATE_ERR;
				NB_WriterReason(gun,"F13",3);
				SetStartFailType(gun,ENDFAIL_INSOLUTIONERR);	
                SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 1);				
				printf("正负母线对地阻值很小\r\n");
			}else{
			
				//计算结缘检测是否正常
				Rp = ISO_R0 *(((Vbus2 - Viso2)*Viso1 - (Vbus1 - Viso1)*Viso2) / \
				((Vbus1 - Viso1)*Viso2 + (ISO_M - 1) * Viso1 * Viso2- (Vbus2 - Viso2)*Viso1) );
				
				Rn = ISO_R0 *(((Vbus2 - Viso2)*Viso1 - (Vbus1 - Viso1)*Viso2) / \
				(ISO_M * (Vbus1 - Viso1)*Viso2 - (Vbus2 - Viso2)*Viso1) );
				
				printf("JY Rp = %d,Rn = %d\r\n",Rp,Rn);
				if((Rp < JY_RESERR) || (Rn < JY_RESERR) )
				{
					ptr->Insulation = STATE_ERR;   //绝缘检测故障
					ptr->NoErr = STATE_ERR;
					NB_WriterReason(gun,"F14",3);
					SetStartFailType(gun,ENDFAIL_INSOLUTIONERR);	
					SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 1);				
					printf("绝缘检测故障　\r\n");
				}
			}
			
		}
	}
    //(4)绝缘检测电压输出停止
    //通知PCU任务停止输出
   send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
	send_message[gun].DivNum = APP_POWER_DISABLE;
	send_message[gun].DataLen = 1;     //0表示不预留模块  1表示预留模块
	send_message[gun].GunNum = gun;  //枪号
	OSQPost(TaskPCU_pevent, &send_message[gun]);
    OSTimeDly(SYS_DELAY_50ms);
    //绝缘检测电路OFF
//    BSP_RLCClose(BSPRLC_INSULATION);
    if(APP_GetGBType(gun) == BMS_GB_2011)
    {//旧国标可能存在接触器直接闭合的情况，这样会导致把电池的电进行了泄放
     //(6)断开充电机的DC继电器K1、K2
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
    } //#if (BMS_CUR_GB == BMS_OLD_GB2011)  //旧国标 

    if(APP_GetGBType(gun) == BMS_GB_2015)  //新标
    {
        BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_5s);
        //判断K1\K2接触器外侧电压是否小于60v
        while (GetRM_DCOutVolt(gun) >= VAL_ADJUST(100, 100, 1))
        {
            //超时5秒
            if (BMS_TIME_OUT == BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_5s))
            {
                ptr->LeakVol = STATE_ERR;
                ptr->NoErr = STATE_ERR;
				SetStartFailType(gun,ENDFAIL_LEAKOUTTIMEOUT);
                ChargeErrDeal(gun,ERR_LF_METHOD_A);
				NB_WriterReason(gun,"F15",3);
				printf("1113\r\n");
                break;
            }
            OSTimeDly(SYS_DELAY_100ms);
        }
        //(6)断开充电机的DC继电器K1、K2
		BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
		SetPowerDCState(gun,POWER_DC_OPEN);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
    }
    else
    {
        OSTimeDly(SYS_DELAY_1s); 
    }

    SetBMSInsulationCheck(gun,FALSE);//设置绝缘检测完成，不再需要绝缘检测
    if (ptr->NoErr == STATE_ERR) //有错误，返回错误
    {//新国标对于泄放有时间要求，目前为5s
        return FALSE;
    }
    else
    {
        ptr->NoErr = STATE_NO_ERR;
    }
    return TRUE;
}

/*****************************************************************************
* Function      : APP_BCPFitCheck
* Description   : 检查BCP参数是否合适
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_BCPFitCheckGunA(void *pdata)
{
	 _BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_A;
    if (GetBMSBCPFitCheck(gun) == TRUE)
    {
//检查BCP参数是否合适,具体是检测什么???
        SysState[gun].BMSState.BCPFitState = STATE_NO_ERR;
        SetBMSBCPFitCheck(gun,FALSE);
    }
    return TRUE;
}


/*****************************************************************************
* Function      : APP_BCPFitCheck
* Description   : 检查BCP参数是否合适
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_BCPFitCheckGunB(void *pdata)
{
	 _BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_B;
    if (GetBMSBCPFitCheck(gun) == TRUE)
    {
//检查BCP参数是否合适,具体是检测什么???
        SysState[gun].BMSState.BCPFitState = STATE_NO_ERR;
        SetBMSBCPFitCheck(gun,FALSE);
    }
    return TRUE;
}
/*****************************************************************************
* Function      : APP_ChargeReadyCheck
* Description   : 充电桩就绪
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
  INT8U APP_ChargeReadyCheckGunA(void *pdata)
{
    INT32U vol;
    static _BSP_MESSAGE send_message[GUN_MAX];
	_PRESET_VALUE		PresetVal_Tmp;
	_BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	PresetVal_Tmp=APP_GetPresetValue();
    if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_A;
    if (GetChargeReady(gun) == READY_ING)
    {
        //"检查DC继电器外侧的电压正常?"
        //电压正常判断条件:
        //(1):与通信报文电池电压相差<=正负5%
        //(2):在充电机的最大最小输出电压之间
		printf("Charge Ready Check\r\n");
        vol = GetRM_DCOutVolt(gun) / 10;//GetRM_DCOutVolt为电池电压
		//电池电压不判断 2020"
		if (GetRS_BatReverse(gun) == TRUE)
		{
			SetStartFailType(gun,ENDFAIL_BATREVERSE);	//电池反接
			printf("电池反接\r\n");
			SetChargeReady(gun,READY_FAIL);
			return FALSE;
		}
		//2020预充电压未未过来是否需要给车子充电"
		if(vol>1000)
		{//这里的vol取决于车里面的一个开关是否闭合，如果没有闭合，这个vol会很小
			if (vol >= BMS_BCP_Context[gun].BatteryPresentVol)
			{
				if (vol * 100 > BMS_BCP_Context[gun].BatteryPresentVol * 115)
				{
					OSTimeDly(SYS_DELAY_1s);  //等待1s再次获取
					vol = GetRM_DCOutVolt(gun) / 10;//GetRM_DCOutVolt为电池电压
					if (vol * 100 > BMS_BCP_Context[gun].BatteryPresentVol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						SetChargeReady(gun,READY_FAIL);
						return FALSE;
					}
					if (BMS_BCP_Context[gun].BatteryPresentVol * 100 > vol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetChargeReady(gun,READY_FAIL);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						return FALSE;
					}
				}
			}
			else
			{
				if (BMS_BCP_Context[gun].BatteryPresentVol * 100 > vol * 115)
				{
					OSTimeDly(SYS_DELAY_1s);  //等待1s再次获取
					vol = GetRM_DCOutVolt(gun) / 10;//GetRM_DCOutVolt为电池电压
					if (BMS_BCP_Context[gun].BatteryPresentVol * 100 > vol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetChargeReady(gun,READY_FAIL);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						return FALSE;
					}
					if (vol * 100 > BMS_BCP_Context[gun].BatteryPresentVol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						SetChargeReady(gun,READY_FAIL);
						return FALSE;
					}
				}
			}
		}
        //判断电压是否比桩本身最大电压小
        if (vol > PresetVal_Tmp.VolMaxOutput)
        {
			printf("vol:%d\r\n",vol);
			SetChargeReady(gun,READY_FAIL);
			SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
            return FALSE;
        }  
		printf("Charge Ready Check Done\r\n");		
		OSTimeDly(SYS_DELAY_500ms); //可能存在开的关的都同时处理，20211116
        //调整充电机输出电压,使得输出电压比电池电压低(1V~10V)
        //发送消息给PCU任务，采用电池电压、5A进行预充
        BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR;
        BMSDemandInfo[gun].Vol = BMS_BCP_Context[gun].BatteryPresentVol - 50; //比电池电压高5V输出
        //发送需求给PCU，开启输出
		send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_POWER_ENABLE;
		send_message[gun].GunNum = gun;  //枪号
		send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
  
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
		
        OSTimeDly(SYS_DELAY_10ms); //20211116
#if 1
		//调试中存在开启模块电压是之前的绝缘检测电压（目前发现只存在2把枪同时启动的情况下），
		//临时处理方法是法相模块电压大于BCP电压，则再下发一次关机指令
        BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_30s); 
		while(1)
		{
			if(PowerModuleInfo[gun].OutputInfo.Vol < BMS_BCP_Context[gun].BatteryPresentVol - VAL_ADJUST(20, 10, 1))
			{
				 OSTimeDly(SYS_DELAY_250ms);
			}
			else if(PowerModuleInfo[gun].OutputInfo.Vol > BMS_BCP_Context[gun].BatteryPresentVol + VAL_ADJUST(20, 10, 1))
			{
				//调试中存在开启模块电压是之前的绝缘检测电压（目前发现只存在2把枪同时启动的情况下），
				//临时处理方法是法相模块电压大于BCP电压，则再下发一次关机指令
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		        send_message[gun].DivNum = APP_POWER_DISABLE;
				send_message[gun].GunNum = gun;	
				send_message[gun].DataLen = 1;     //0表示不预留模块  1表示预留模块				
		         OSQPost(TaskPCU_pevent ,&send_message[gun]);
		        OSTimeDly(SYS_DELAY_2s);
				
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
				send_message[gun].DivNum = APP_POWER_ENABLE;
				send_message[gun].GunNum = gun;  //枪号
				send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
		  
				OSQPost(TaskPCU_pevent ,&send_message[gun]);
				OSTimeDly(SYS_DELAY_10ms);

				
			}else{
				break;
			}
			if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
            	//发送需求给PCU，关闭输出
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
				send_message[gun].DivNum = APP_POWER_DISABLE;
				send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
				send_message[gun].GunNum = gun;			
				OSQPost(TaskPCU_pevent ,&send_message[gun]);
		         OSTimeDly(SYS_DELAY_1s);
                //输出电压出错
                SysState[gun].SelfCheckState.LowVol = STATE_ERR;
                SysState[gun].SelfCheckState.NoErr = STATE_ERR;
                ChargeErrDeal(gun,ERR_LF_METHOD_C);
				printf("1111  vol = %d\r\n",PowerModuleInfo[gun].OutputInfo.Vol);
				SetStartFailType(gun,ENDFAIL_CHARGEMODULEERR);
				SetChargeReady(gun,READY_FAIL);
                return FALSE;
            }
		}
#endif
        if(APP_GetGBType(gun) == BMS_GB_2015)  //新标
            BMS_BCP_Context[gun].MaxVoltage = BMS_BHM_Context[gun].MaxChargeWholeVol;//黄海车辆BRM与BCP交错
        //闭合DC继电器K1\K2
		BSP_CloseK1K2(gun);
		SetPowerDCState(gun,POWER_DC_CLOSE);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
        //充电机就绪
        SetChargeReady(gun,READY_SUCCESS);
		printf("Charger Ready Done\r\n");
    }

    return TRUE;
}


/*****************************************************************************
* Function      : APP_ChargeReadyCheck
* Description   : 充电桩就绪
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
  INT8U APP_ChargeReadyCheckGunB(void *pdata)
{
    INT32U vol;
    static _BSP_MESSAGE send_message[GUN_MAX];
	_PRESET_VALUE		PresetVal_Tmp;
	_BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	PresetVal_Tmp=APP_GetPresetValue();
    if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_B;
    if (GetChargeReady(gun) == READY_ING)
    {
        //"检查DC继电器外侧的电压正常?"
        //电压正常判断条件:
        //(1):与通信报文电池电压相差<=正负5%
        //(2):在充电机的最大最小输出电压之间
		printf("Charge Ready Check\r\n");
        vol = GetRM_DCOutVolt(gun) / 10;//GetRM_DCOutVolt为电池电压
		//电池电压不判断 2020"
		if (GetRS_BatReverse(gun) == TRUE)
		{
			SetStartFailType(gun,ENDFAIL_BATREVERSE);	//电池反接
			printf("电池反接\r\n");
			SetChargeReady(gun,READY_FAIL);
			return FALSE;
		}
		//2020预充电压未未过来是否需要给车子充电"
		if(vol>1000)
		{//这里的vol取决于车里面的一个开关是否闭合，如果没有闭合，这个vol会很小
			if (vol >= BMS_BCP_Context[gun].BatteryPresentVol)
			{
				if (vol * 100 > BMS_BCP_Context[gun].BatteryPresentVol * 115)
				{
					OSTimeDly(SYS_DELAY_1s);  //等待1s再次获取
					vol = GetRM_DCOutVolt(gun) / 10;//GetRM_DCOutVolt为电池电压
					if (vol * 100 > BMS_BCP_Context[gun].BatteryPresentVol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						SetChargeReady(gun,READY_FAIL);
						return FALSE;
					}
					if (BMS_BCP_Context[gun].BatteryPresentVol * 100 > vol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetChargeReady(gun,READY_FAIL);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						return FALSE;
					}
				}
			}
			else
			{
				if (BMS_BCP_Context[gun].BatteryPresentVol * 100 > vol * 115)
				{
					OSTimeDly(SYS_DELAY_1s);  //等待1s再次获取
					vol = GetRM_DCOutVolt(gun) / 10;//GetRM_DCOutVolt为电池电压
					if (BMS_BCP_Context[gun].BatteryPresentVol * 100 > vol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetChargeReady(gun,READY_FAIL);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						return FALSE;
					}
					if (vol * 100 > BMS_BCP_Context[gun].BatteryPresentVol * 115)
					{
						printf("vol:%d,BatteryPresentVol:%d\r\n",vol,BMS_BCP_Context[gun].BatteryPresentVol);
						SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
						SetChargeReady(gun,READY_FAIL);
						return FALSE;
					}
				}
			}
		}
        //判断电压是否比桩本身最大电压小
        if (vol > PresetVal_Tmp.VolMaxOutput)
        {
			printf("vol:%d\r\n",vol);
			SetChargeReady(gun,READY_FAIL);
			SetStartFailType(gun,ENDFAIL_BATVOLTERR3);
            return FALSE;
        }  
		printf("Charge Ready Check Done\r\n");		
		OSTimeDly(SYS_DELAY_500ms); //可能存在开的关的都同时处理，20211116
        //调整充电机输出电压,使得输出电压比电池电压低(1V~10V)
        //发送消息给PCU任务，采用电池电压、5A进行预充
        BMSDemandInfo[gun].Cur = DEF_OUTPUT_CUR;
        BMSDemandInfo[gun].Vol = BMS_BCP_Context[gun].BatteryPresentVol - 50; //比电池电压高5V输出
        //发送需求给PCU，开启输出
		send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		send_message[gun].DivNum = APP_POWER_ENABLE;
		send_message[gun].GunNum = gun;  //枪号
		send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
  
        OSQPost(TaskPCU_pevent ,&send_message[gun]);
		
        OSTimeDly(SYS_DELAY_10ms); //20211116
#if 1
		//调试中存在开启模块电压是之前的绝缘检测电压（目前发现只存在2把枪同时启动的情况下），
		//临时处理方法是法相模块电压大于BCP电压，则再下发一次关机指令
        BMS_TIMEOUT_ENTER(gun,BMS_OTH1_STEP, SYS_DELAY_30s); 
		while(1)
		{
			if(PowerModuleInfo[gun].OutputInfo.Vol < BMS_BCP_Context[gun].BatteryPresentVol - VAL_ADJUST(20, 10, 1))
			{
				 OSTimeDly(SYS_DELAY_250ms);
			}
			else if(PowerModuleInfo[gun].OutputInfo.Vol > BMS_BCP_Context[gun].BatteryPresentVol + VAL_ADJUST(20, 10, 1))
			{
				//调试中存在开启模块电压是之前的绝缘检测电压（目前发现只存在2把枪同时启动的情况下），
				//临时处理方法是法相模块电压大于BCP电压，则再下发一次关机指令
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
		        send_message[gun].DivNum = APP_POWER_DISABLE;
				send_message[gun].GunNum = gun;	
				send_message[gun].DataLen = 1;     //0表示不预留模块  1表示预留模块				
		         OSQPost(TaskPCU_pevent ,&send_message[gun]);
		        OSTimeDly(SYS_DELAY_2s);
				
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
				send_message[gun].DivNum = APP_POWER_ENABLE;
				send_message[gun].GunNum = gun;  //枪号
				send_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
		  
				OSQPost(TaskPCU_pevent ,&send_message[gun]);
				OSTimeDly(SYS_DELAY_10ms);

				
			}else{
				break;
			}
			if (BMS_TIMEOUT_CHECK(gun,BMS_OTH1_STEP, SYS_DELAY_30s) == BMS_TIME_OUT)
            {
            	//发送需求给PCU，关闭输出
				send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
				send_message[gun].DivNum = APP_POWER_DISABLE;
				send_message[gun].DataLen = 0;     //0表示不预留模块  1表示预留模块
				send_message[gun].GunNum = gun;			
				OSQPost(TaskPCU_pevent ,&send_message[gun]);
		         OSTimeDly(SYS_DELAY_1s);
                //输出电压出错
                SysState[gun].SelfCheckState.LowVol = STATE_ERR;
                SysState[gun].SelfCheckState.NoErr = STATE_ERR;
                ChargeErrDeal(gun,ERR_LF_METHOD_C);
				printf("1111  vol = %d\r\n",PowerModuleInfo[gun].OutputInfo.Vol);
				SetStartFailType(gun,ENDFAIL_CHARGEMODULEERR);
				SetChargeReady(gun,READY_FAIL);
                return FALSE;
            }
		}
#endif
        if(APP_GetGBType(gun) == BMS_GB_2015)  //新标
            BMS_BCP_Context[gun].MaxVoltage = BMS_BHM_Context[gun].MaxChargeWholeVol;//黄海车辆BRM与BCP交错
        //闭合DC继电器K1\K2
		BSP_CloseK1K2(gun);
		SetPowerDCState(gun,POWER_DC_CLOSE);
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
        //充电机就绪
        SetChargeReady(gun,READY_SUCCESS);
		printf("Charger Ready Done\r\n");
    }

    return TRUE;
}

extern INT16U curpowera;
extern INT16U curpowerb;
/*****************************************************************************
* Function      : APP_ChargeRunning
* Description   : 充电开始，需要让功率模块输出电能
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_ChargeRunningGunA(void *pdata)
{
    static _BSP_MESSAGE  pcusend_message[GUN_MAX];
    _CCU_DEMAND_INFO DemandInfo1[GUN_MAX];
	_PRESET_VALUE	PresetValueTmp;
	INT16U cur;
	INT8U gun;
	_BSP_MESSAGE * pdispose = pdata;
	if(pdata == NULL)
	{
		return FALSE;
	}
	static _BSP_MESSAGE send_message[GUN_MAX];
	OS_EVENT* pdisevent =  APP_GetDispEvent();
	gun = GUN_A;
 
	PresetValueTmp=APP_GetPresetValue();
    
    if (GetChargeMode(gun) == MODE_MAN) //手动模式
    {
        if (ManDemandInfo[gun].Vol > PresetValueTmp.VolMaxOutput)
        {
            //充电失败，其他错误
            SysState[gun].SelfCheckState.Other = STATE_ERR;
            SysState[gun].SelfCheckState.NoErr = STATE_ERR;
			SetStartFailType(gun,ENDFAIL_OTHERERR);
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			NB_WriterReason(gun,"F4",2);
			send_message[gun].DivNum = APP_CHARGE_FAIL; //启动失败
			send_message[gun].GunNum = gun;
			OSQPost(pdisevent, &send_message[gun]);
            OSTimeDly(SYS_DELAY_10ms);
            SetBMSStartCharge(gun,FALSE);
			printf("Demand Vol Too Large\r\n");
            return FALSE;
        }
        else
        {
            if (ManDemandInfo[gun].Cur > PresetValueTmp.CurMaxOutput)
            {
                ManDemandInfo[gun].Cur = PresetValueTmp.CurMaxOutput; 
            }
            //通知PCU任务输出电能
            pcusend_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
            pcusend_message[gun].DivNum = APP_POWER_ENABLE;
            pcusend_message[gun].GunNum = gun;  //枪号
            pcusend_message[gun].pData = (INT8U *)(&ManDemandInfo[gun].Vol);
            OSQPost(TaskPCU_pevent, &pcusend_message[gun]);
            OSTimeDly(SYS_DELAY_10ms);
			
			
            //已经在充电了，说明是更新了需求
            if (GetChargeRunningState(gun) == TRUE)
            {
                return TRUE;
            }
            //手动模式需要闭合DC继电器K1\K2
            BSP_OpenK1K2(gun);
			SetPowerDCState(gun,POWER_DC_CLOSE);
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
            memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
                (INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
            SetChargeRunningState(gun,TRUE);
            //启动成功
            send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
            send_message[gun].DivNum = APP_CHARGE_SUCCESS;
			send_message[gun].GunNum = gun;
            OSQPost(pdisevent, &send_message[gun]);
            OSTimeDly(SYS_DELAY_10ms);
        }
    }
    else
    {
        //根据BMS需求给PCU任务发送消息
        //需求值的有效性也在这边做掉
        DemandInfo1[gun].Cur = 4000 - BMS_BCL_Context[gun].DemandCur; 
		cur = 4000 - BMS_BCP_Context[gun].MaxCurrent;
		if(DemandInfo1[gun].Cur > cur)
		{
			DemandInfo1[gun].Cur = cur;
		}
        DemandInfo1[gun].Vol = BMS_BCL_Context[gun].DemandVol;


        BMSDemandInfo[gun].Vol = DemandInfo1[gun].Vol;
        BMSDemandInfo[gun].Cur = DemandInfo1[gun].Cur;
        if (BMSDemandInfo[gun].Cur > PresetValueTmp.CurMaxOutput)
        {
            BMSDemandInfo[gun].Cur = PresetValueTmp.CurMaxOutput; 
        }
		if(SYSSet.NetState == DISP_NET)
		{
			//网络模式下功率调节
			if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
			{
				 if(PowerModuleInfo[GUN_A].OutputInfo.Vol < 2000)
				 {
					if (BMSDemandInfo[GUN_A].Cur > (curpowera * 10000/2000))
					{
						BMSDemandInfo[GUN_A].Cur = (curpowera * 10000/2000); 
					}
				 }
				else
				{
					if (BMSDemandInfo[GUN_A].Cur > (curpowera * 10000/ PowerModuleInfo[GUN_A].OutputInfo.Vol))
					{
						BMSDemandInfo[GUN_A].Cur = (curpowera* 10000/ PowerModuleInfo[GUN_A].OutputInfo.Vol); 
					}
				}
			}
		}
        if (BMSDemandInfo[gun].Vol > PresetValueTmp.VolMaxOutput)
        {
            BMSDemandInfo[gun].Vol = PresetValueTmp.VolMaxOutput;
        }
		else if(BMSDemandInfo[gun].Vol<PresetValueTmp.VolMinOutput)
		{
			printf("AAAMINMIN,%x\r\n",BMSDemandInfo[gun].Vol);

            if(BMSDemandInfo[gun].Vol<=0)
			{
				if(APP_GetGBType(gun) == BMS_GB_2015)
				{
					BMSDemandInfo[gun].Vol=PresetValueTmp.VolMinOutput;
				}
				else
				{
					BMSDemandInfo[gun].Vol=2000;
				}
			}
			else
			{
				BMSDemandInfo[gun].Vol=PresetValueTmp.VolMinOutput;
                printf("AAAMINMIN2,%x\r\n",BMSDemandInfo[gun].Vol);
			}
		}
		
        //发送需求给PCU任务  
        if (GetChargeRunningState(gun) == TRUE)
        {
			
            //已经在充电了，直接一次性到位
             pcusend_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
             pcusend_message[gun].DivNum = APP_POWER_ENABLE;
             pcusend_message[gun].GunNum = gun;    //枪号
             pcusend_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
            OSQPost(TaskPCU_pevent ,&pcusend_message[gun]);
            OSTimeDly(SYS_DELAY_20ms);
			//三次重连有bug，放在上面没有打开 20210627
			BSP_OpenK1K2(gun);
            return TRUE;
        }
        else 
        {//还未开始充电，采用阶梯递增的方式增加电流
            APP_BatChargeAdjust(gun,BMSDemandInfo[gun].Vol, BMSDemandInfo[gun].Cur, 3);
	
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
            memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
                (INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
            SetChargeRunningState(gun,TRUE);
            //启动成功
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
            send_message[gun].DivNum = APP_CHARGE_SUCCESS;
			send_message[gun].GunNum = gun;
            OSQPost(pdisevent, &send_message[gun]);
			SetBMSConnectStep(gun,BMS_CONNECT_RUNSUCCESS);
            OSTimeDly(SYS_DELAY_10ms);
			printf("Charge Begin\r\n");
        }
    }
	//三次重连有bug，放在上面没有打开 20210627
	BSP_OpenK1K2(gun);
	SetPowerDCState(gun,POWER_DC_CLOSE);
    //点亮充电灯
    BSP_OpenRunLed(gun); 
    //获取开始心跳
    SysState[gun].ChargeInfo.StartTick = OSTimeGet();
    //获取充电时间
    BSP_RTCGetTime((_BSPRTC_TIME *)&SysState[gun].ChargeInfo.StartTime.Second);
    SetBMSStopCharge(gun,FALSE);  //开始充电
    return TRUE;
}


/*****************************************************************************
* Function      : APP_ChargeRunning
* Description   : 充电开始，需要让功率模块输出电能
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月8日  叶喜雨
*****************************************************************************/
INT8U APP_ChargeRunningGunB(void *pdata)
{
    static _BSP_MESSAGE  pcusend_message[GUN_MAX];
    _CCU_DEMAND_INFO DemandInfo1[GUN_MAX];
	_PRESET_VALUE	PresetValueTmp;
	INT16U cur;
	INT8U gun;
	_BSP_MESSAGE * pdispose = pdata;
	if(pdata == NULL)
	{
		return FALSE;
	}

	static _BSP_MESSAGE send_message[GUN_MAX];
	OS_EVENT* pdisevent =  APP_GetDispEvent();
	gun = GUN_B;
 
	PresetValueTmp=APP_GetPresetValue();
    
    if (GetChargeMode(gun) == MODE_MAN) //手动模式
    {
        if (ManDemandInfo[gun].Vol > PresetValueTmp.VolMaxOutput)
        {
            //充电失败，其他错误
            SysState[gun].SelfCheckState.Other = STATE_ERR;
            SysState[gun].SelfCheckState.NoErr = STATE_ERR;
			SetStartFailType(gun,ENDFAIL_OTHERERR);
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			send_message[gun].DivNum = APP_CHARGE_FAIL; //启动失败
			NB_WriterReason(gun,"F30",3);
			send_message[gun].GunNum = gun;
			OSQPost(pdisevent, &send_message[gun]);
            OSTimeDly(SYS_DELAY_10ms);
            SetBMSStartCharge(gun,FALSE);
			printf("Demand Vol Too Large\r\n");
            return FALSE;
        }
        else
        {
            if (ManDemandInfo[gun].Cur > PresetValueTmp.CurMaxOutput)
            {
                ManDemandInfo[gun].Cur = PresetValueTmp.CurMaxOutput; 
            }
            //通知PCU任务输出电能
            pcusend_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
            pcusend_message[gun].DivNum = APP_POWER_ENABLE;
            pcusend_message[gun].GunNum = gun;  //枪号
            pcusend_message[gun].pData = (INT8U *)(&ManDemandInfo[gun].Vol);
            OSQPost(TaskPCU_pevent, &pcusend_message[gun]);
            OSTimeDly(SYS_DELAY_10ms);
			
			
            //已经在充电了，说明是更新了需求
            if (GetChargeRunningState(gun) == TRUE)
            {
                return TRUE;
            }
            //手动模式需要闭合DC继电器K1\K2
            BSP_OpenK1K2(gun);
			SetPowerDCState(gun,POWER_DC_CLOSE);
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
            memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
                (INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
            SetChargeRunningState(gun,TRUE);
            //启动成功
            send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
            send_message[gun].DivNum = APP_CHARGE_SUCCESS;
			send_message[gun].GunNum = gun;
            OSQPost(pdisevent, &send_message[gun]);
            OSTimeDly(SYS_DELAY_10ms);
        }
    }
    else
    {
        //根据BMS需求给PCU任务发送消息
        //需求值的有效性也在这边做掉
        DemandInfo1[gun].Cur = 4000 - BMS_BCL_Context[gun].DemandCur; 
		cur = 4000 - BMS_BCP_Context[gun].MaxCurrent;
		if(DemandInfo1[gun].Cur > cur)
		{
			DemandInfo1[gun].Cur = cur;
		}
        DemandInfo1[gun].Vol = BMS_BCL_Context[gun].DemandVol;


        BMSDemandInfo[gun].Vol = DemandInfo1[gun].Vol;
        BMSDemandInfo[gun].Cur = DemandInfo1[gun].Cur;
        if (BMSDemandInfo[gun].Cur > PresetValueTmp.CurMaxOutput)
        {
            BMSDemandInfo[gun].Cur = PresetValueTmp.CurMaxOutput; 
        }
		if(SYSSet.NetState == DISP_NET)
		{
			//网络模式下功率调节
			if((SYSSet.NetYXSelct == XY_HY)||  (SYSSet.NetYXSelct == XY_YL2)||  (SYSSet.NetYXSelct == XY_JG))
			{
				 if(PowerModuleInfo[GUN_B].OutputInfo.Vol < 2000)
				 {
					if (BMSDemandInfo[GUN_B].Cur > (curpowerb * 10000/2000))
					{
						BMSDemandInfo[GUN_B].Cur = (curpowerb * 10000/2000); 
					}
				 }
				else
				{
					if (BMSDemandInfo[GUN_B].Cur > (curpowerb * 10000/ PowerModuleInfo[GUN_B].OutputInfo.Vol))
					{
						BMSDemandInfo[GUN_B].Cur = (curpowerb * 10000/ PowerModuleInfo[GUN_B].OutputInfo.Vol); 
					}
				}
			}
		}
        if (BMSDemandInfo[gun].Vol > PresetValueTmp.VolMaxOutput)
        {
            BMSDemandInfo[gun].Vol = PresetValueTmp.VolMaxOutput;
        }
		else if(BMSDemandInfo[gun].Vol<PresetValueTmp.VolMinOutput)
		{
			printf("AAAMINMIN,%x\r\n",BMSDemandInfo[gun].Vol);

            if(BMSDemandInfo[gun].Vol<=0)
			{
				if(APP_GetGBType(gun) == BMS_GB_2015)
				{
					BMSDemandInfo[gun].Vol=PresetValueTmp.VolMinOutput;
				}
				else
				{
					BMSDemandInfo[gun].Vol=2000;
				}
			}
			else
			{
				BMSDemandInfo[gun].Vol=PresetValueTmp.VolMinOutput;
                printf("AAAMINMIN2,%x\r\n",BMSDemandInfo[gun].Vol);
			}
		}
		
        //发送需求给PCU任务  
        if (GetChargeRunningState(gun) == TRUE)
        {
			
            //已经在充电了，直接一次性到位
             pcusend_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
             pcusend_message[gun].DivNum = APP_POWER_ENABLE;
             pcusend_message[gun].GunNum = gun;    //枪号
             pcusend_message[gun].pData = (INT8U *)&BMSDemandInfo[gun].Vol;
            OSQPost(TaskPCU_pevent ,&pcusend_message[gun]);
            OSTimeDly(SYS_DELAY_20ms);
			//三次重连有bug，放在上面没有打开 20210627
			BSP_OpenK1K2(gun);
            return TRUE;
        }
        else 
        {//还未开始充电，采用阶梯递增的方式增加电流
            APP_BatChargeAdjust(gun,BMSDemandInfo[gun].Vol, BMSDemandInfo[gun].Cur, 3);
	
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
            memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
                (INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
            SetChargeRunningState(gun,TRUE);
            //启动成功
			send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
            send_message[gun].DivNum = APP_CHARGE_SUCCESS;
			send_message[gun].GunNum = gun;
            OSQPost(pdisevent, &send_message[gun]);
			SetBMSConnectStep(gun,BMS_CONNECT_RUNSUCCESS);
            OSTimeDly(SYS_DELAY_10ms);
			printf("Charge Begin\r\n");
        }
    }
	//三次重连有bug，放在上面没有打开 20210627
	BSP_OpenK1K2(gun);
	SetPowerDCState(gun,POWER_DC_CLOSE);
    //点亮充电灯
    BSP_OpenRunLed(gun); 
    //获取开始心跳
    SysState[gun].ChargeInfo.StartTick = OSTimeGet();
    //获取充电时间
    BSP_RTCGetTime((_BSPRTC_TIME *)&SysState[gun].ChargeInfo.StartTime.Second);
    SetBMSStopCharge(gun,FALSE);  //开始充电
    return TRUE;
}
/*****************************************************************************
* Function      : APP_ChargePause
* Description   : 收到BMS任务的BMS帧，需要暂停或重新开启充电
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_ChargePauseGunA(void *pdata)
{
	static _BSP_MESSAGE send_message[GUN_MAX];
	_BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_A;
	//需要暂停输出
	if (BMS_BSM_Context[gun].State1.OneByte.ChargeAllowBits == 0x00)
	{//还未暂停输出
		if (GetChargePauseState(gun) == FALSE)
		{//开关模块输出
            SetChargePauseState(gun,TRUE);
			 send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			send_message[gun].DivNum = APP_POWER_DISABLE;
			send_message[gun].GunNum = gun;  //枪号
			send_message[gun].DataLen = 1;   //1表示预留模块
			OSQPost(TaskPCU_pevent, &send_message[gun]);
		    OSTimeDly(SYS_DELAY_50ms);
			printf("Power Disable Message Send\r\n");
		}
	}
	return TRUE;
}


/*****************************************************************************
* Function      : APP_ChargePause
* Description   : 收到BMS任务的BMS帧，需要暂停或重新开启充电
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_ChargePauseGunB(void *pdata)
{
	static _BSP_MESSAGE send_message[GUN_MAX];
	_BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_B;
	//需要暂停输出
	if (BMS_BSM_Context[gun].State1.OneByte.ChargeAllowBits == 0x00)
	{//还未暂停输出
		if (GetChargePauseState(gun) == FALSE)
		{//开关模块输出
            SetChargePauseState(gun,TRUE);
			 send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_CONTROL;
			send_message[gun].DivNum = APP_POWER_DISABLE;
			send_message[gun].GunNum = gun;  //枪号
			send_message[gun].DataLen = 1;   //1表示预留模块
			OSQPost(TaskPCU_pevent, &send_message[gun]);
		    OSTimeDly(SYS_DELAY_50ms);
			printf("Power Disable Message Send\r\n");
		}
	}
	return TRUE;
}

/*****************************************************************************
* Function      : APP_ChargeBSD
* Description   : 收到BMS任务的充电信息报文
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_ChargeBSDGunA(void *pdata)
{
	_BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_A;
    SetChargeInfo(gun,6, pdata, sizeof(_BMS_BSD_CONTEXT));
    return TRUE;
}

/*****************************************************************************
* Function      : APP_ChargeBSD
* Description   : 收到BMS任务的充电信息报文
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月14日  叶喜雨
*****************************************************************************/
INT8U APP_ChargeBSDGunB(void *pdata)
{
	_BSP_MESSAGE * pdisposedata = pdata;
	INT8U gun;
	if (pdata == NULL)
	{
		return FALSE;
	}
	gun = GUN_B;
    SetChargeInfo(gun,6, pdata, sizeof(_BMS_BSD_CONTEXT));
    return TRUE;
}

/*****************************************************************************
* Function      : APP_AnalyzeRxData
* Description   : 控制任务处理收到的函数
* Input         : INT8U divnum  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
static INT8U APP_AnalyzeRxDataA(_BSP_MESSAGE *pMsg)
{
    INT8U i;
    const _CONTROL_CMD *p = ControlCmdTableA;

    for (i = 0; i < CMD_TABLE_NUM; i++)
    {
        if (pMsg->DivNum == p[i].DivNum)
        {
            if (p[i].Fun != NULL)
            {
                return (p[i].Fun((void *)pMsg) );
            }
        }
    }
    return FALSE;
}


/*****************************************************************************
* Function      : APP_AnalyzeRxData
* Description   : 控制任务处理收到的函数
* Input         : INT8U divnum  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
static INT8U APP_AnalyzeRxDataB(_BSP_MESSAGE *pMsg)
{
    INT8U i;
    const _CONTROL_CMD *p = ControlCmdTableB;

    for (i = 0; i < CMD_TABLE_NUM; i++)
    {
        if (pMsg->DivNum == p[i].DivNum)
        {
            if (p[i].Fun != NULL)
            {
                return (p[i].Fun((void *)pMsg) );
            }
        }
    }
    return FALSE;
}

/*****************************************************************************
* Function      : GetWorkState
* Description   : 获取工作状态
* Input         : INT8U state  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月14日  叶喜雨
*****************************************************************************/
_WORK_STATE GetWorkState(_GUN_NUM gun,INT8U state)
{
    if (ERR_CHECK(state)) //故障检查
    {
        return WORK_STATE_ERR;
    }
    else if (GetChargeRunningState(gun) == TRUE)
    {
        //已经输出电能算正在工作
        return WORK_STATE_RUN;
    }
    else //其他都算待机
    {
        return WORK_STATE_IDLE;
    }
}

/*****************************************************************************
* Function      : CheckBSTErr
* Description   : 检测BMS返回的BST是否有错
* Input         : INT8U *perr
* Output        : None
* Note(s)       :
* Contributor   : 2016年7月6日  叶喜雨
*****************************************************************************/
INT8U CheckBSTErr(INT8U gun,INT8U *perr)
{

	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	APP_Set_ERR_Branch(gun,STOP_BSMNORMAL); //初始化为正常停止
   
                    //需求SOC目标值，此处并没有告警
	if ( (BMS_BST_Context[gun].State0.OneByte.SocTargetBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State0.OneByte.TolVolValBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State0.OneByte.VolSetValBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State0.OneByte.ChargeStopBits == STATE_TROUBLE) )
	{
		SysState[gun].StopReason.BMSStop.State0.EndChargeReason \
			= BMS_BST_Context[gun].State0.EndChargeReason;
		//BMS中止错误原因
		SysState[gun].TotalErrState	|= ERR_BMSSTOP_TYPE;
		APP_Set_ERR_Branch(gun,STOP_BSMNORMAL);
		//达到条件
		*perr |= STOP_CONDITION;
		printf("11146\r\n");
	}

	
	if ( (BMS_BST_Context[gun].State1.OneByte.InsulationErrBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.ConnectorOverTempBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.ElementOverTempBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.ConnectorErrBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.BatOverTempBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.HighPresRelayBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.TPTwoVolErrBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State1.OneByte.OtherErrBits == STATE_TROUBLE) )
	{
		if(BMS_BST_Context[gun].State1.OneByte.InsulationErrBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_BSTINSULATIONERR);
		}
		else if (BMS_BST_Context[gun].State1.OneByte.ConnectorOverTempBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_BSTSWOVERT);
		}
		else if(BMS_BST_Context[gun].State1.OneByte.ElementOverTempBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_BSTELOVERT);
		}
		else if(BMS_BST_Context[gun].State1.OneByte.ConnectorErrBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_CERR);
		}
		else if(BMS_BST_Context[gun].State1.OneByte.BatOverTempBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_BSTBATOVERT);
		}
		else if(BMS_BST_Context[gun].State1.OneByte.TPTwoVolErrBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_BSTTPTWO);
		}
		else if(BMS_BST_Context[gun].State1.OneByte.HighPresRelayBits == STATE_TROUBLE)
		{
			APP_Set_ERR_Branch(gun,STOP_BSTHIGHRLCERR);
		}
		else
		{
			APP_Set_ERR_Branch(gun,STOP_BSTOTHERERR);
		}
		SysState[gun].StopReason.BMSStop.State1.EndChargeTroubleReason \
			= BMS_BST_Context[gun].State1.EndChargeTroubleReason;
	
		//BMS中止错误原因
		SysState[gun].TotalErrState	|= ERR_BMSSTOP_TYPE;
	//	APP_Set_ERR_Branch(STOP_BSMERR);
		//错误中止
		*perr |= STOP_ERR; 
		printf("11145\r\n");
	}

	if ( (BMS_BST_Context[gun].State2.OneByte.OverCurErrBits == STATE_TROUBLE) \
	   || (BMS_BST_Context[gun].State2.OneByte.OverVolErrBits == STATE_TROUBLE) )
	{
		SysState[gun].StopReason.BMSStop.State2.EndChargeErroReason \
			= BMS_BST_Context[gun].State2.EndChargeErroReason;
		//BMS电流过大
		if (BMS_BST_Context[gun].State2.OneByte.OverCurErrBits)
		{
			//直流母线过流告警1
			APP_Set_ERR_Branch(gun,STOP_OUTCURROVER);
			SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 7, 1);
		}
		else
		{
			//直流母线不过流告警0
			SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 7, 0);
		}
		//BMS电压过大
		if (BMS_BST_Context[gun].State2.OneByte.OverVolErrBits)
		{
			//直流母线输出过压告警1
         	APP_Set_ERR_Branch(gun,STOP_OUTVOLTVORE);  //电压异常
			SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 5, 1);
		}
		else
		{
			//直流母线输出不过压告警0
			SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 5, 0);
		}

		//BMS中止错误原因
		SysState[gun].TotalErrState	|= ERR_BMSSTOP_TYPE;
//		APP_Set_ERR_Branch(STOP_BSMNORMAL);
		//错误中止
		*perr |= STOP_ERR; 
		printf("11144\r\n");
	}

    return TRUE;
}

/*****************************************************************************
* Function      : APP_CheckPowerModule
* Description   : 判断模块是否有故障
* Input         : _BMS_CST_CONTEXT* const ptr  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月12日  叶喜雨
*****************************************************************************/
INT8U APP_CheckPowerModule(INT8U gun,_BMS_CST_CONTEXT* const ptr, INT8U *perr)
{
    INT8U i;
    static INT8U oldState[3] =  {0};
    static INT8U oldState3 = 0;
 

	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
    _APP_CHARGE_STATE2 *p2 = &PowerModuleInfo[gun].TotalState;
	_MODULE_STATE_CCU *pstate = &PowerModuleInfo[gun].TotalState.ModuleState;
	INT8U *p 				= &PowerModuleInfo[gun].TotalState.ModuleState.State1.allbits;
	if ( (ptr == NULL) || (perr == NULL)  )
    {
        return FALSE;
    }
    
  
    for (i = 0; i < sizeof(_MODULE_STATE_CCU); i++)
    {//判断模块的3个字节状态是否置1
        if ( oldState[i] != *(p + i) || (*(p + i)))
        {
            oldState[i] = *(p + i); 
            switch (i)
            {
                case 0:
//                    if ( *(p + i) & 0x40) 
//                    {//模块输出短路
//                        ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;		//CST中设置为电能不能传输
//                        SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;			//停止原因为充电机中止错误
//						APP_Set_ERR_Branch(gun,STOP_SHORTCIRCUIT);
//                        *perr |= STOP_ERR; 										//错误原因为故障停止
//						printf("11119\r\n");
//                        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 5, 1);	//功率模块直流输出短路故障1
//                    }
//                    else
//                    {//功率模块直流输出短路无故障0
//                        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 5, 0);
//                    }
                    if ( *(p + i) & 0xBE) //BE-10111110
                    {//功率模块故障
                        if (pstate->State1.onebits.Mod_Alarm)
                        {//模块故障告警状态
#if 0 //模块总状态不做处理
                            //CST中设置为电能不能传输
                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
							printf("11111119\r\n");
                            //错误原因为故障停止
                            *perr |= STOP_ERR; 
#endif
                        }
                        
                        if (pstate->State1.onebits.Protected)
                        {//模块保护过压告警状态
#if 0
                            //CST中设置为电能不能传输
                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
							printf("11111110\r\n");
                            //错误原因为故障停止
                            *perr |= STOP_ERR; 
#endif
                        }
                        
                        if (pstate->State1.onebits.Tem_Over)
                        {//功率模块过温告警,阀值为100度
#if (MODULE_WARNING_SHUTDOWN > 0u)
                            //CST中设置为充电机内部过温故障
                            ptr->State1.OneByte.ChargeOverTempInBits = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
							printf("1111111910\r\n");
                            //错误原因为故障停止
                            *perr |= STOP_ERR; 
#else
                            //CST中设置为充电机内部过温故障
                            ptr->State1.OneByte.ChargeOverTempInBits = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            //SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
                            //警告状态，但不停止充电
                            *perr |= STOP_WARN; 
							printf("11117\r\n");
#endif
                            //充电模块过温告警1
                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 1, 1);
                        }
                        else
                        {//充电模块无过温告警0
                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 1, 0);
                        }
                    
                        if (pstate->State1.onebits.Volt_Over)
                        { //功率模块输出过压告警状态
#if (MODULE_WARNING_SHUTDOWN > 0u)
                            //CST中设置充电电压过压
                            ptr->State2.OneByte.OverVolErrBits = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
							printf("111111199\r\n");
                            //错误原因为故障停止
                            *perr |= STOP_ERR; 
#else
                            //警告状态，但不停止充电
                            *perr |= STOP_WARN; 
#endif
                            //功率模块输出过压1
                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 1);
                        }
                        else
                        {//功率模块无输出过压0
                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 0);
                        }
                        
//                        if (pstate->State1.onebits.Commu_Aarm)
//                        {//功率模块通讯中断
//                            //CST中设置为电能不能传输
//                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
//                            //停止原因为充电机中止错误
//                            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;
//							APP_Set_ERR_Branch(gun,STOP_CHARGEMODULEERR);  //充电机模块故障
//                            //错误原因为故障停止
//                            *perr |= STOP_ERR; 
//							printf("11116\r\n");							
//                            //充电模块通信告警1
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 2, 1);
//                        }
//                        else
//                        {//充电模块无通信告警0
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 2, 0);
//                        }
                    }
                    break;
                case 1:
                    if ( *(p + i) & 0x7F)//7F-01111111
                    {
                        if (pstate->State2.onebits.Power_Limit)
                        {//模块处于功率限制状态
#if (MODULE_WARNING_SHUTDOWN > 0u)
                            //CST中设置为电能不能传输
                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
							printf("11111118\r\n");
                            //错误原因为故障停止
                            *perr |= STOP_ERR; 
#else
//                            //警告状态，但不停止充电
//                            *perr |= STOP_WARN; 
#endif
                        }
                        
//                        if (pstate->State2.onebits.ID_Repeat)
//                        {//模块ID重复
//                            //CST中设置为电能不能传输
//                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
//                            //停止原因为充电机中止错误
//                            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;
//							APP_Set_ERR_Branch(gun,STOP_CHARGEMODULEERR);
//                            //错误原因为故障停止
//                            *perr |= STOP_ERR; 
//							printf("11115\r\n");
//                            //充电模块故障1
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 0, 1);
//                        }
//                        else
//                        {//充电模块无故障0
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 0, 0);
//                        }
                        
                        if (pstate->State2.onebits.Current_Unbalance)
                        {//模块严重不均流
#if (MODULE_WARNING_SHUTDOWN > 0u)
                            //CST中设置为电能不能传输
                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
                            //停止原因为充电机中止错误
                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
							printf("111111197\r\n");
                            //错误原因为故障停止
                            *perr |= STOP_ERR; 
#else
//                            //警告状态，但不停止充电
//                            *perr |= STOP_WARN; 
#endif
                        }
                        
//                        if (pstate->State2.onebits.AC_UVP)
//                        {//模块交流输入欠压告警，输入欠压点为255V
//#if (MODULE_WARNING_SHUTDOWN > 0u)
//                            //CST中设置为电能不能传输
//                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
//                            //停止原因为充电机中止错误
//                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
//							printf("111111196\r\n");
//                            //错误原因为故障停止
//                            *perr |= STOP_ERR; 
//#else
//                            //CST中设置为电能不能传输
////                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
//                            //停止原因为充电机中止错误
////                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
//                            //警告状态，但不停止充电
//                            *perr |= STOP_WARN; 
//#endif
//                            //模块交流输入欠压告警1
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 3, 1);
//                        }
//                        else
//                        {
//                            //模块交流无输入欠压告警0
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 3, 0);
//                        }
						
//                        if (pstate->State2.onebits.AC_Over)
//                        {//模块交流输入过压告警,过压点为535V
//#if (MODULE_WARNING_SHUTDOWN > 0u)
//                            //CST中设置为电能不能传输
//                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
//                            //停止原因为充电机中止错误
//                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
//							printf("111111195\r\n");
//                            //错误原因为故障停止
//                            *perr |= STOP_ERR; 
//#else
//                            //CST中设置为电能不能传输
////                            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
////                            //停止原因为充电机中止错误
////                            SysState.TotalErrState |= ERR_CHARGESTOP_TYPE;
//                            //警告状态，但不停止充电
//                            *perr |= STOP_WARN; 
//#endif
//                            //模块交流输入过压告警1
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 2, 1);
//                        }
//                        else
//                        {
//                            //模块无交流输入过压告警0
//                            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 2, 0);
//                        }
                    }
                    break;
            }
        }
    }
	
	#if (MODULE_WARNING_SHUTDOWN > 0u)   //20220812    怕模块误报
    if ((p2->State3.AllBits != oldState3) || (p2->State3.AllBits))
    {//有故障
        oldState3 = p2->State3.AllBits;
        if (p2->State3.OneByte.InACOverVolBit)
        {//模块输入过压异常告警
            //CST所需电能不能传输故障
            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
            //停止原因为充电机中止错误
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;
			APP_Set_ERR_Branch(gun,STOP_ACINERR);
            //错误原因为故障停止
            *perr |= STOP_ERR; 
			printf("11114\r\n");
//			*perr |= STOP_WARN; 
            //交流输入过压告警
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 3, 1);
        }
        //模块本身返回跟按照国标计算出来的输入电压都没过压才能去掉警告
        else
        {//交流输入过压告警
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 3, 0);
        }
        
        if (p2->State3.OneByte.InACUnderVolBit)
        {//模块输入欠压告警(自定义阀值)
            //CST所需电能不能传输故障
            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
            //停止原因为充电机中止错误
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;
			APP_Set_ERR_Branch(gun,STOP_ACINERR);
            //错误原因为故障停止
            *perr |= STOP_ERR;  
			printf("11113\r\n");
//			*perr |= STOP_WARN; 			
            //交流输入欠压告警
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 4, 1);
        }
        //模块本身返回跟按照国标计算出来的输入电压都没欠压才能去掉警告
        else
        {
            //交流输入欠压告警
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 4, 0);
        }
//        if(p2->State3.OneByte.RevBits)
//        {
//            ptr->State1.OneByte.PowerUnreach = STATE_TROUBLE;
//            //停止原因为充电机中止错误
//            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;
//			APP_Set_ERR_Branch(gun,STOP_CHARGEMODULEERR);
//            //错误原因为故障停止
//            *perr |= STOP_ERR;  
//			printf("11113\r\n");
//        }
    }
	#endif
    
    return TRUE;
}


/*****************************************************************************
* Function      : APP_MonitorChargingState
* Description   : 监控充电过程中的状态
* Input         : void  
* Output        : None
* Note(s)       : static
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
static void APP_MonitorChargingState(INT8U gun)
{    
    INT8U 	err;
	INT32U	Vol_Judge=0;
	INT16U ACVol;
    _STOP_TYPE stoptype = STOP_UNDEF;
	_PRESET_VALUE	PresetVal_Temp;
 	static INT32U curtime[GUN_MAX] = {0},lastcurtime[GUN_MAX] = {0},voltime[GUN_MAX] = {0},lastvoltime[GUN_MAX] = {0};
    static _BSP_MESSAGE send_message[GUN_MAX]; 
	static INT32U	LastTotalPower[GUN_MAX] = {0},powertime[GUN_MAX] = {0},powerlasttime[GUN_MAX] = {0};
	if(gun >= GUN_MAX)
	{
		return;
	}
    //ptr指向充电机停止原因CST
    _BMS_CST_CONTEXT* const ptr = (_BMS_CST_CONTEXT *)(&SysState[gun].StopReason.ChargeStop.State0.EndChargeReason);
    
    if (GetGunState(gun) == GUN_DISCONNECTED)
    {//(1)枪拔出
        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {
           if((GetChargeMode(gun) == MODE_AUTO) || (GetChargeMode(gun) == MODE_VIN))
            {
                BSP_CloseK1K2(gun);					//断开输出接触器K1\K2
				SetPowerDCState(gun,POWER_DC_OPEN);
                SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
               BSP_CloseBMSPower(gun);
                ptr->State1.OneByte.OtherErrBits = STATE_TROUBLE; //CST中没有枪拔出的字段，采用其他故障字段
                SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;    //停止原因为充电机中止错误
					NB_WriterReason(gun,"E18",3);
				APP_Set_ERR_Branch(gun,STOP_HANDERR);
                stoptype |= STOP_ERR;							  //错误原因为故障停止
				printf("51\r\n");
            }
        }
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 5, 1);   //枪未连接
    }
    else
    {//枪已连接
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 5, 0);
    }
    #if ENBLE_JG
	if(MSPIo.QDC || MSPIo.SWDC_A || MSPIo.SWDC_B|| (BSP_MPLSState(BSP_DI_W) == 0) || BSP_MPLSState(BSP_DI_QX))
	{
		if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//急停按下
            ptr->State1.OneByte.UrgentStopBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE; 		//停止原因为充电机中止错误
			NB_WriterReason(gun,"E99",3);
			if(MSPIo.QDC)
			{
//				STOP_XQERR,                    //倾斜
//	STOP_SJERR,                    //水景
//	STOP_SWAERR,                    //直流接触器A
//	STOP_SWBERR,                    //直流接触器B
//	STOP_QJERR,                    //桥接
				APP_Set_ERR_Branch(gun,STOP_QJERR);
			}
			else if(MSPIo.SWDC_A)
			{
				APP_Set_ERR_Branch(gun,STOP_SWAERR);
			}
					else if(MSPIo.SWDC_B)
			{
				APP_Set_ERR_Branch(gun,STOP_SWBERR);
			}
			else if(BSP_MPLSState(BSP_DI_W) == 0)
			{
				APP_Set_ERR_Branch(gun,STOP_SJERR);
			}
			else
			{
				APP_Set_ERR_Branch(gun,STOP_XQERR);
			}
            stoptype |= STOP_ERR;								//错误原因为故障停止
			printf("其他故障52\r\n");
        }
	}
	#endif
	
	//门禁  20220719
	if (BSP_MPLSState(BSP_DI_DOOR))
    {
        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//急停按下
            ptr->State1.OneByte.UrgentStopBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE; 		//停止原因为充电机中止错误
			NB_WriterReason(gun,"E8",2);
			SetStartFailType(gun,ENDFAIL_OTHERERR);
			APP_Set_ERR_Branch(gun,STOP_DOORERR);
            stoptype |= STOP_ERR;								//错误原因为故障停止
			printf("52\r\n");
        }
    }
	
	
    if (GetEmergencyState(gun) == EMERGENCY_PRESSED)
    {//(2)急停按钮是否按下
        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//急停按下
            ptr->State1.OneByte.UrgentStopBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE; 		//停止原因为充电机中止错误
			NB_WriterReason(gun,"E17",3);
			SetStartFailType(gun,ENDFAIL_EMERGENCY);
			APP_Set_ERR_Branch(gun,STOP_EMERGENCY);
            stoptype |= STOP_ERR;								//错误原因为故障停止
			printf("52\r\n");
        }
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 4, 1);	//急停按下0
    }
    else
    {//急停未按下1
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 4, 0);
    }
	if(PcuControl.ModuleNormal == 0)   //没有可用模块
	{
		 if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//
            ptr->State1.OneByte.UrgentStopBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE; 		//停止原因为充电机中止错误
			NB_WriterReason(gun,"E66",3);
			SetStartFailType(gun,ENDFAIL_MODULEERR);
			APP_Set_ERR_Branch(gun,STOP_OTHERERR);
            stoptype |= STOP_ERR;								//错误原因为故障停止
			printf("52\r\n");
        }
	}

    //(3)电子锁没锁住，导引测试时，电磁锁实际上不会锁，这里就不判断电磁锁，默认电磁锁锁上
    if (GetElecLockState(gun) == ELEC_LOCK_UNLOCKED)
    {
		//再充电中而且不在停止中
        if (GetChargeRunningState(gun) == TRUE && (GetBMSStopCharge(gun) == FALSE))
        {
        //仅在自动模式下判断电磁锁状态
           if((GetChargeMode(gun) == MODE_AUTO) || (GetChargeMode(gun) == MODE_VIN))
            {//电子锁没锁住，CST报文中的其他故障
                ptr->State1.OneByte.OtherErrBits = STATE_TROUBLE;
                SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
					NB_WriterReason(gun,"E16",3);
				SetStartFailType(gun,ENDFAIL_ELECLOCKERR);
				APP_Set_ERR_Branch(gun,STOP_ELECLOCKERR);
                stoptype |= STOP_ERR; 								//错误原因为故障停止
				printf("53\r\n");
				//电子锁故障
				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 1, 1);
            }
        }
        //充电接口电子锁状态1
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 6, 0);
    }
    else
    {//电子锁正常锁上
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 1, 0);
        //充电接口电子锁状态1
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 6, 1);
    }
	if (GetGunState((_GUN_NUM)gun) == GUN_DISCONNECTED)
   {//枪未连接清状态	
		SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 4, 0);//流程化的错误在用户确认后清除
		SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_2, 5, 0);//流程化的错误在用户确认后清除
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 2, 0);//流程化的错误在用户确认后清除
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 2, 0);//模块输入欠压清除
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 5, 0);//模块输出短路清除
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 6, 0);//模块输出过流
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 0);//模块输出过压
		SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 7, 0);//过流
	   	SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 5, 0);//电压异常
//		SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_1, 7, 0);   //继电器粘连拔枪清除   20211118
    }
	PresetVal_Temp=APP_GetPresetValue();
	Vol_Judge = PowerModuleInfo[gun].OutputInfo.Vol;    //现场运行出现直流电压过压现象   20210909 叶

	if(Vol_Judge < 7600)   //必须小于760V，大于760V可能存在校准有问题
	{
		if (Vol_Judge > (PresetVal_Temp.Therold_Value.OverOutVolTherold + 200))
		{//(5)判断模块返回的输出电压是否正常，1：模块输出与最高阀值；2：继电器后级电压与电池电压
			if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
			{//输出电压过高
				ptr->State2.OneByte.OverVolErrBits = STATE_TROUBLE;
				SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
				NB_WriterReason(gun,"E15",3);
				APP_Set_ERR_Branch(gun,STOP_OUTVOLTVORE);
				SetStartFailType(gun,ENDFAIL_OUTVOLTVORE);
				stoptype |= STOP_ERR; 								//错误原因为故障停止
				printf("55,%x\r\n",Vol_Judge);
			}
			SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 1);	//直流母线输出过压告警1
		}
	}
	
   if ( (Vol_Judge > (BMS_BCP_Context[gun].MaxVoltage + 300)) \
        && (BMS_BCP_Context[gun].MaxVoltage >= 3000) )       //采样误差有点的以bcp的最大电压输出的时候会有点问题，故范围加大
    {
        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//输出电压过高
            ptr->State2.OneByte.OverVolErrBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
			NB_WriterReason(gun,"E14",3);
			APP_Set_ERR_Branch(gun,STOP_OUTVOLTVORE);
			SetStartFailType(gun,ENDFAIL_OUTVOLTVORE);
            stoptype |= STOP_ERR; 								//错误原因为故障停止
			printf("56,%x,%x\r\n",Vol_Judge,BMS_BCP_Context[gun].MaxVoltage);
        }
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 1);	//直流母线输出过压告警1
    }
    else 
    {
        if (Vol_Judge <= PresetVal_Temp.Therold_Value.OverOutVolTherold)
        {//直流母线输出不过压告警0
            SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 0);
        }
    }
	
	
	//国网测试的时候才判断输入过欠压
	//三项输入电压判断
	ACVol = GetModuleReturnACVol();
	if ((ACVol > (PresetVal_Temp.Therold_Value.OverACVolTherold)) && (ACVol < 6000) )  //怕AC误报 20211021
    {//(6)判断模块返回的输出电流是否正常
        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//输入电压过压
            ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
				NB_WriterReason(gun,"E13",3);
			APP_Set_ERR_Branch(gun,STOP_ACINERR);
			SetStartFailType(gun,ENDFAIL_ACINERR);
            stoptype |= STOP_ERR; 								//错误原因为故障停止
			printf("ac57\r\n");
        }
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 3, 1);	//输入过压
    }
    else
    {//直流母线输出不过流告警0
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 3, 0);
    }
	
	#if(ENBLE_JG == 0)     //精工断交流接触器不能报欠压
	if ((ACVol < (PresetVal_Temp.Therold_Value.UnderACVolTherold)) && (ACVol < 6000) )   //怕AC误报 20211021
    {//(6)判断模块返回的输出电流是否正常
        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
        {//输入电压欠压
            ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;
            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
				NB_WriterReason(gun,"E12",3);
			APP_Set_ERR_Branch(gun,STOP_ACINERR);
			SetStartFailType(gun,ENDFAIL_ACINERR);
            stoptype |= STOP_ERR; 								//错误原因为故障停止
			printf("ac57\r\n");
        }
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 4, 1);	//输入欠压
    }
    else
	#endif
    {//直流母线输出不过流告警0
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_3, 4, 0);
    }

	if(BCURemoteSignal[gun].state1.State.DCContactor_Err)   //直流接触器粘连
	{
		 if((GetChargeMode(gun) == MODE_AUTO) || (GetChargeMode(gun) == MODE_VIN))
		 {
			//只要有一个粘连，2把枪都要停止充电
			 if (!( (GetBMSStartCharge(GUN_A) == FALSE) && (GetBMSStopCharge(GUN_A) == FALSE) ) )
			{//直流接触器粘连
				ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;
				SysState[GUN_A].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
				NB_WriterReason(GUN_A,"E63",3);
				APP_Set_ERR_Branch(GUN_A,STOP_OUTSWERR2);
				SetStartFailType(GUN_A,ENDFAIL_OUTSWERR2);
				stoptype |= STOP_ERR; 								//错误原因为故障停止
				printf("dc57\r\n");
			}
			if (!( (GetBMSStartCharge(GUN_B) == FALSE) && (GetBMSStopCharge(GUN_B) == FALSE) ) )
			{//直流接触器粘连
				ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;
				SysState[GUN_B].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
				NB_WriterReason(GUN_B,"E64",3);
				APP_Set_ERR_Branch(GUN_B,STOP_OUTSWERR2);
				SetStartFailType(GUN_B,ENDFAIL_OUTSWERR2);
				stoptype |= STOP_ERR; 								//错误原因为故障停止
				printf("dc57\r\n");
			}
		}
	}
	
	//	static INT32U	LastTotalPower[GUN_MAX] = {0},powertime[GUN_MAX] = {0},powerlasttime[GUN_MAX] = {0};
	//充电的时候持续2分钟电能未发生变化，则停止充电    20230511
	powertime[gun] = OSTimeGet();
	if((GetChargeMode(gun) == MODE_AUTO) || (GetChargeMode(gun) == MODE_VIN) )
	{
		 if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
		 {
			 if(LastTotalPower[gun] == GunBillInfo[gun].TotalPower4)
			 {
				 if((powertime[gun] >= powerlasttime[gun]) ? ((powertime[gun] - powerlasttime[gun]) >= SYS_DELAY_2M) : \
				((powertime[gun] + (INT32U_MAX_NUM - powerlasttime[gun])) >= SYS_DELAY_2M))
				 {
					 powerlasttime[gun] = powertime[gun]; 
					LastTotalPower[gun] =  GunBillInfo[gun].TotalPower4;
					 ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;
					SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
					NB_WriterReason(gun,"E65",3);
					APP_Set_ERR_Branch(gun,STOP_MATERFAIL);
					SetStartFailType(gun,ENDFAIL_OTHERERR);
					stoptype |= STOP_ERR; 								//错误原因为故障停止
					printf("power57\r\n");
				 }
			 }
			 else
			 {
				powerlasttime[gun] = powertime[gun]; 
				LastTotalPower[gun] =  GunBillInfo[gun].TotalPower4;
			 }
		 }
		 else
		 {
			powerlasttime[gun] = powertime[gun]; 
			LastTotalPower[gun] =  GunBillInfo[gun].TotalPower4;
		 }
	}
	else
	{
		powerlasttime[gun] = powertime[gun]; 
		LastTotalPower[gun] =  GunBillInfo[gun].TotalPower4;
	}

	
	
	
	
//	if (PowerModuleInfo[gun].OutputInfo.Cur > (PresetVal_Temp.Therold_Value.OverOutCurTherold + MAX_DEVIATION) )
//    {//(6)判断模块返回的输出电流是否正常
//        if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
//        {//输出电流过高
//            ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;

//            SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
//			APP_Set_ERR_Branch(gun,STOP_OUTCURROVER);
//			SetStartFailType(gun,ENDFAIL_OUTCURROVER);
//            stoptype |= STOP_ERR; 								//错误原因为故障停止
//	
//			printf("57\r\n");
//        }
//        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 6, 1);	//直流母线输出过流告警1
//    }
   
    err = STOP_UNDEF;
    if (APP_CheckPowerModule(gun,ptr, &err) == TRUE)
    {//(5)判断模块返回的状态是否正常
        if (err == STOP_ERR)
        {
			NB_WriterReason(gun,"E11",3);
			printf("58\r\n");
        }
        stoptype |= err;
    }

	
	
	//充电过程中，连续5s，电压，电流超过需求电压或者需求电流，则判断为异常
	if((GetChargeRunningState(gun) == TRUE) &&  ((GetChargeMode(gun) == MODE_AUTO) || ((GetChargeMode(gun) == MODE_VIN))))
	{
		//电流判断
		curtime[gun] = OSTimeGet();
		//存在汽车电流频繁得下降 20210610
		if (PowerModuleInfo[gun].OutputInfo.Cur  > ((4000-BMS_BCL_Context[gun].DemandCur) + MAX_DEVIATION) )
		{
			if((curtime[gun] >= lastcurtime[gun]) ? ((curtime[gun] - lastcurtime[gun]) >= SYS_DELAY_15s) : \
				((curtime[gun] + (INT32U_MAX_NUM - lastcurtime[gun])) >= SYS_DELAY_15s))
			{
				//(6)判断模块返回的输出电流是否正常
				if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
				{
			
					//输出电流过高
					ptr->State2.OneByte.OverCurErrBits = STATE_TROUBLE;

					SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
					NB_WriterReason(gun,"E10",3);
					APP_Set_ERR_Branch(gun,STOP_OUTCURROVER);
					SetStartFailType(gun,ENDFAIL_OUTCURROVER);
					stoptype |= STOP_ERR; 								//错误原因为故障停止
					SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 6, 1);	//直流母线输出过流告警1
					printf("577\r\n ");
				}
	
			}
		}
		else
		{
			lastcurtime[gun] = curtime[gun];
		}
		
		//电压判断
		voltime[gun] = OSTimeGet();
		
		if (PowerModuleInfo[gun].OutputInfo.Vol > (BMS_BCL_Context[gun].DemandVol + BHMCMP_VOLT) )
		{
			if((voltime[gun] >= lastvoltime[gun]) ? ((voltime[gun] - lastvoltime[gun]) >= SYS_DELAY_10s) : \
				((voltime[gun] + (INT32U_MAX_NUM - lastvoltime[gun])) >= SYS_DELAY_10s))
			{
				if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
				{//输出电压过高
					ptr->State2.OneByte.OverVolErrBits = STATE_TROUBLE;
					SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE;		//停止原因为充电机中止错误
					NB_WriterReason(gun,"E9",2);
					APP_Set_ERR_Branch(gun,STOP_OUTVOLTVORE);
					SetStartFailType(gun,ENDFAIL_OUTVOLTVORE);
					stoptype |= STOP_ERR; 								//错误原因为故障停止
					printf("55,Vol = %d,BCL = %d\r\n",Vol_Judge,BMS_BCL_Context[gun].DemandVol);
				}
				SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_4, 7, 1);	//直流母线输出过压告警1
			}
		}
		else
		{
			lastvoltime[gun] = voltime[gun];
		}
		
	}
	else
	{
		curtime[gun] = OSTimeGet();
		lastcurtime[gun] = curtime[gun];
		voltime[gun] = OSTimeGet();
		lastvoltime[gun] = voltime[gun];
	}
	
    if (GetPowerDCState(gun) == POWER_DC_OPEN)
    {//直流接触器断开
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_OPEN);
    }
    else 
    {//闭合
        SetBcuRemoteSignalState(gun,(INT8U)REMOTE_SIGNALE_5, 7, POWER_DC_CLOSE);
    }

	
	
	if(SYSSet.GunTemp == 0)
	{
		//枪温度，超过120°就强制关机
		 if ((BSP_GetTemp1(gun) > ADCJustInfo[gun].T1_120) || (BSP_GetTemp2(gun) > ADCJustInfo[gun].T2_120) ) 
		{
			ptr->State1.OneByte.UrgentStopBits = STATE_TROUBLE;
			SysState[gun].TotalErrState	|= ERR_CHARGESTOP_TYPE; 		//停止原因为充电机中止错误
			NB_WriterReason(gun,"E7",2);
			SetStartFailType(gun,ENDFAIL_GUNUPTEMPERATURE);
			APP_Set_ERR_Branch(gun,STOP_GUNUPTEMPERATURE);
			stoptype |= STOP_ERR;								//错误原因为故障停止
			//printf("62\r\n");
		}
	}
	
	
    if (!( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) ) )
    {//有错误发生
        if (SysState[gun].TotalErrState != ERR_NORMAL_TYPE)
        {//通知控制任务停止充电
			printf("GUN = %d,TotalErr:%x\r\n",gun,SysState[gun].TotalErrState);
            send_message[gun].MsgID = (_BSP_MSGID)BSP_MSGID_PERIOD;
            send_message[gun].DivNum = APP_CHARGE_END;
            send_message[gun].DataLen = 1;
            send_message[gun].pData = (INT8U *)&stoptype;
			send_message[gun].GunNum = gun;
			if(gun == GUN_A)
			{
				OSQPost(Control_PeventA, &send_message[gun]);	//给控制任务发送消息
			}
			else
			{
				OSQPost(Control_PeventB, &send_message[gun]);	//给控制任务发送消息
			}
            OSTimeDly(SYS_DELAY_1s);
        }
    }
}

/*****************************************************************************
* Function      : ControlParaInit
* Description   : 控制参数初始化
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月15日  叶喜雨
*****************************************************************************/
void ControlParaInit(INT8U gun)
{
		if(gun >= GUN_MAX)
		{
			return;
		}


		    //需要重新自检
		SetBMSSelfCheck(gun,TRUE);
		//需要检测BCP参数
		SetBMSBCPFitCheck(gun,TRUE);
		//需要绝缘检测
		SetBMSInsulationCheck(gun,TRUE);
		//充电未开始
		SetBMSStartCharge(gun,FALSE);
		printf("555659654655\r\n");
		//充电允许
		SetChargePauseState(gun,FALSE);
		//充电未结束
		SetBMSStopCharge(gun,FALSE);
		//可以进行结束充电
		SetBMSStopOnce(gun,FALSE);
		//充电机未准备好
		SetChargeReady(gun,READY_ING);
		//未输出电能
		SetChargeRunningState(gun,FALSE);
		//充电模式设置为未定义
		SetChargeMode(gun,MODE_UNDEF);
		//未处理故障
		SetDealFaultOverState(gun,FALSE);
		memset( (INT8U *)&SysState[gun].TotalErrState, 0, \
			(INT8U *)&SysState[gun].RemoteMeterState.GunState - (INT8U *)&SysState[gun].TotalErrState);
			    //手动充电模式清0
		memset( (INT8U *)&ManDemandInfo[gun].Vol, 0, sizeof(_CCU_DEMAND_INFO) );
		memset( (INT8U *)&BMSDemandInfo[gun].Vol, 0, sizeof(_CCU_DEMAND_INFO) );
		//功率模块相关信息清0

		memset( (INT8U *)&PowerModuleInfo[gun].OutputInfo.Vol, 0, sizeof(_POWER_MODULE_INFO));

		//枪未连接
		SetGunState(gun,GUN_DISCONNECTED);
		//充电灯关闭
		BSP_CloseRunLed(gun);
		//故障灯关闭
		BSP_CloseFailLed(gun);
		//断开K1 K2
		BSP_CloseK1K2(gun);	
		//断开辅助电源
		BSP_CloseBMSPower(gun);
	
}

void APP_ControlHardwareInit(void)
{
	INT8U gun;
	for(gun = 0;gun < GUN_MAX;gun++)
	{
		BSP_CloseBMSPower(gun);
		BSP_CloseLOCK(gun);
		BSP_CloseK1K2(gun);
	}

}

/*****************************************************************************
* Function      : APP_ControlInit
* Description   : 控制任务变量初始化
* Input         : void  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月13日  叶喜雨
*****************************************************************************/
INT8U APP_ControlInit(void)
{  
    //创建消息队列
    Control_PeventA = OSQCreate(ControlOSQA, CONTROL_Q_NUM);
    if (Control_PeventA == NULL)
    {
        while(1);
    }
	
	Control_PeventB = OSQCreate(ControlOSQB, CONTROL_Q_NUM);
    if (Control_PeventB == NULL)
    {
        while(1);
    }
    //参数初始化
    ControlParaInit(GUN_A);
	ControlParaInit(GUN_B);
    return TRUE;
}

/*****************************************************************************
* Function      : APP_MonitorGunState
* Description   : 监控枪状态，枪的状态由BCU任务定时读取枪状态，不采用消息方式
* Input         : void  
* Output        : None
* Note(s)       : static
* Contributor   : 2016年6月15日  叶喜雨
*****************************************************************************/
static void APP_MonitorGunState(INT8U gun)
{
	if(gun >= GUN_MAX)
	{
		return;
	}
    if (GetRM_HandVolt(gun) == INTERFACE_CONNECTED)  
    {//枪已经连接好
        if (GetGunState(gun) == GUN_DISCONNECTED)
        {//原来枪未连接好
        	SetGunState(gun,GUN_CONNECTED);
        }
    }
    else 
    {//枪未连接好
        if (GetGunState((_GUN_NUM)gun) == GUN_CONNECTED)
        {//原来枪连接好
			SetGunState(gun,GUN_DISCONNECTED);
			//空闲状态，全部初始化，其他状态就不做任何处理
			if ( (GetBMSStartCharge(gun) == FALSE) && (GetBMSStopCharge(gun) == FALSE) )
			{//需要重新初始化参数
	            ControlParaInit(gun);
                
                BMS_CONNECT_StepSet((_GUN_NUM)gun,BMS_SEND_DEFAULT);	//停止发送任何帧
                BMS_CONNECT_ControlInit(gun);  //重新初始化
			}
			else 
			{//枪从连接到断开，必须要重新做绝缘监测
			    SetBMSInsulationCheck(gun,TRUE);//需要绝缘检测
			}
        }
    }
}
/*****************************************************************************
* Function      : APP_MonitorRemoteSignalState
* Description   : 监控遥信状态
* Input         : void  
* Output        : None
* Note(s)       : static
* Contributor   : 2016年7月6日  叶喜雨
*****************************************************************************/
static void APP_MonitorRemoteSignalState(INT8U gun)
{
    INT8U state;

	state = BSP_MPLSState(BSP_DI_JT);//急停状态
	if ( (_EMERGENCY_STATE)state != GetEmergencyState(gun) )
	{
		if(EMERGENCY_PRESSED == (_EMERGENCY_STATE)state )
		{
			//电子锁存在解不开，拍下急停，解电子锁动作一次  20210702
			BSP_OpenLOCK(gun);
			OSTimeDly(SYS_DELAY_500ms);
			BSP_CloseLOCK(gun);
		}
		SetEmergencyState( gun,(_EMERGENCY_STATE)state);
	}
   
	state = BSP_GetLockState(gun);//枪电磁锁状态
  //  if ( (_ELEC_LOCK_STATE)state != GetElecLockState() )
	{
		SetElecLockState(gun,(_ELEC_LOCK_STATE)state);
	}

}

/***********************************************************************************************
* Function      : Get_BCUYXPara
* Description   : 判断整机遥信故障状态，以BCU上传数据为准
* Input         :
* Output        : 任意一个故障即可报黄灯
* Note(s)       :
* Contributor   : 2020-9-16 叶喜雨
***********************************************************************************************/
ERR_LED_STATE Get_ErrLedState(INT8U gun)
{
	static INT8U state[GUN_MAX] = {0};
	if(gun >= GUN_MAX)
	{
		return  ERR_LEDOPENED;
	}
	#if(ENBLE_JG == 0)
	{
		if(((BCURemoteSignal[gun].state1.byte&0xB0)!=0)||((BCURemoteSignal[gun].state2.byte&0x3C)!=0)||\
			((BCURemoteSignal[gun].state3.byte&0xFC)!=0)||((BCURemoteSignal[gun].state4.byte&0xE4)!=0) || BSP_MPLSState(BSP_DI_DOOR))
		{
			if(state[gun])
			{
				state[gun] = 0;
				printf("state1 =%x,state2 =%x,state3 =%x,state4 =%x,state5 =%x,state6 =%x,state7 =%x",BCURemoteSignal[gun].state1.byte,\
				BCURemoteSignal[gun].state2.byte,BCURemoteSignal[gun].state3.byte,BCURemoteSignal[gun].state4.byte,BCURemoteSignal[gun].state5.byte,BCURemoteSignal[gun].state6.byte,BCURemoteSignal[gun].state7.byte);
			}
			return ERR_LEDOPENED;
		}
	}
	#else
	{
		//精工
		if(((BCURemoteSignal[gun].state1.byte&0xB0)!=0)||((BCURemoteSignal[gun].state2.byte&0x3C)!=0)||\
			((BCURemoteSignal[gun].state3.byte&0xFC)!=0)||((BCURemoteSignal[gun].state4.byte&0xE4)!=0) || BSP_MPLSState(BSP_DI_DOOR) 
			|| MSPIo.QDC || MSPIo.SWDC_A || MSPIo.SWDC_B|| BSP_MPLSState(BSP_DI_W) == 0 || BSP_MPLSState(BSP_DI_QX))
		{
			if(state[gun])
			{
				state[gun] = 0;
				printf("state1 =%x,state2 =%x,state3 =%x,state4 =%x,state5 =%x,state6 =%x,state7 =%x",BCURemoteSignal[gun].state1.byte,\
				BCURemoteSignal[gun].state2.byte,BCURemoteSignal[gun].state3.byte,BCURemoteSignal[gun].state4.byte,BCURemoteSignal[gun].state5.byte,BCURemoteSignal[gun].state6.byte,BCURemoteSignal[gun].state7.byte);
			}
			return ERR_LEDOPENED;
		}
		
	}
	#endif
	state[gun] = 1;
    return ERR_LEDECLOSED;
}

/***********************************************************************************************
* Function      : APP_DCSWDispose
* Description   :对应枪没启动，接触器后继有电压，需要报故障PcuControl.OpenCloseState[gunnum] = GUN_CLOSE;	
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2023-5-10 叶喜雨
***********************************************************************************************/
void APP_DCSWDispose(INT8U gun)
{
	static INT32U curtime[GUN_MAX] = {0},lasttime[GUN_MAX] = {0};
	
	curtime[gun] = OSTimeGet();
	
	if(gun >= GUN_MAX)
	{
		return;
	}
	
	//持续10s处于关机状态,而且电压都大于200V 说明有粘连，需要断电重启，故障才能消失
	if((PcuControl.OpenCloseState[gun] ==  GUN_CLOSE)  && (GetRM_DCOutVolt(gun) > 20000) )
	{
		if((curtime[gun] >= lasttime[gun]) ? ((curtime[gun] - lasttime[gun]) >= SYS_DELAY_15s ) : \
		((curtime[gun] + (INT32U_MAX_NUM - lasttime[gun])) >= SYS_DELAY_15s ) )
		{
			//A B枪都报故障，防止A B枪继续充电
			SetBcuRemoteSignalState(GUN_A,(INT8U)REMOTE_SIGNALE_1, 7, 1);  //直流接触器故障，必须断电重启
			SetBcuRemoteSignalState(GUN_B,(INT8U)REMOTE_SIGNALE_1, 7, 1);  //直流接触器故障，必须断电重启
			lasttime[gun] = curtime[gun];
		}
	}
	else
	{
		lasttime[gun] = curtime[gun];
	}
}
#if ENBLE_JG
/***********************************************************************************************
* Function      : APP_DCSWStateDispose
* Description   :直流接触去状态处理	
* Input         :
* Output        :
* Note(s)       :
* Contributor   :2023-5-10 叶喜雨
***********************************************************************************************/
void APP_DCSWStateDispose(void)
{
	static  INT32U curtimea = 0,curtimeb = 0,curtimeq = 0,nowSysTime;
	
	nowSysTime = OSTimeGet();
	#if (USER_GUN	!= USER_SINGLE_GUN)
		if((MSPIo.QDC_State  &&  (BSP_MPLSState(BSP_DI_QSW) == 0)) || ((MSPIo.QDC_State == 0 ) &&  BSP_MPLSState(BSP_DI_QSW)) )
		{
			curtimeq = nowSysTime;
			MSPIo.QDC = 0;
		}
		else
		{
			if((nowSysTime >= curtimeq) ? ((nowSysTime - curtimeq) >= SYS_DELAY_2s) : \
			((nowSysTime + (0xffffffff - curtimeq)) >= SYS_DELAY_2s))
			{
				curtimeq = nowSysTime;
				MSPIo.QDC = 0;   //桥接临时屏蔽
			}
		}
		#endif
	
	if((MSPIo.SWDC_A_State  &&  (BSP_MPLSState(BSP_DI_DCSWA)  == 0)) || ((MSPIo.SWDC_A_State == 0)  &&  BSP_MPLSState(BSP_DI_DCSWA) == 1) )
	{
		curtimea = nowSysTime;
		MSPIo.SWDC_A = 0;
	}
	else
	{
		if((nowSysTime >= curtimea) ? ((nowSysTime - curtimea) >= SYS_DELAY_2s) : \
		((nowSysTime + (0xffffffff - curtimea)) >= SYS_DELAY_2s))
		{
			curtimea = nowSysTime;
			MSPIo.SWDC_A = 1;
		}
	}
	
	#if (USER_GUN	!= USER_SINGLE_GUN)
	if((MSPIo.SWDC_B_State  &&  (BSP_MPLSState(BSP_DI_DCSWB) == 0) ) || ((MSPIo.SWDC_B_State == 0)  &&  BSP_MPLSState(BSP_DI_DCSWB)) )
	{
		curtimeb = nowSysTime;
		MSPIo.SWDC_B = 0;
	}
	else
	{
		if((nowSysTime >= curtimeb) ? ((nowSysTime - curtimeb) >= SYS_DELAY_2s) : \
		((nowSysTime + (0xffffffff - curtimeb)) >= SYS_DELAY_2s))
		{
			curtimeb = nowSysTime;
			MSPIo.SWDC_B = 1;
		}
	}
	#endif
	
}
#endif


/*****************************************************************************
* Function      : TaskPeriod
* Description   : 周期性任务
* Input         : void *data  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月5日  叶喜雨
*****************************************************************************/
void TaskPeriod(void *data)
{
	INT8U gun;
    OSTimeDly(SYS_DELAY_1s);
    while (1)
    {
		#if ENBLE_JG
		APP_DCSWStateDispose();
		#endif
		OSTimeDly(SYS_DELAY_5ms);
		for(gun = 0;gun < GUN_MAX;gun++)
		{
			//判断是否未故障
			if(Get_ErrLedState(gun) == ERR_LEDOPENED)
			{
				OSSchedLock();
				SysState[gun].WorkState = GetWorkState((_GUN_NUM)gun,STOP_ERR);//计算工作状态
				BCURemoteSignal[gun].state1.byte &= 0xf0;
				BCURemoteSignal[gun].state1.byte |= SysState[gun].WorkState & 0x0f;
				OSSchedUnlock();
				BSP_OpenFailLed(gun);
			}else
			{
				OSSchedLock();
				SysState[gun].WorkState = GetWorkState((_GUN_NUM)gun,STOP_UNDEF);//计算工作状态
				BCURemoteSignal[gun].state1.byte &= 0xf0;
				BCURemoteSignal[gun].state1.byte |= SysState[gun].WorkState & 0x0f;
				OSSchedUnlock();
				BSP_CloseFailLed(gun);
			}
			//跟新模块数据（跟新  PowerModuleInfo）
			Updata_ModuleInfo();
			//实时获取枪状态
			APP_MonitorGunState(gun);
			//实时获取遥信状态
			APP_MonitorRemoteSignalState(gun);
			//开始充电后，由此任务实时监控输出电压跟电流
			APP_MonitorChargingState(gun);
			

			//对应枪没启动，接触器后继有电压，需要报故障PcuControl.OpenCloseState[gunnum] = GUN_CLOSE;	
		//	#if(USER_GUN != USER_SINGLE_GUN)
			#if 0
			APP_DCSWDispose(gun);
			#endif
		}
    }
}
/*****************************************************************************
* Function      : TaskControl
* Description   : 控制任务，此任务控制整个充电的逻辑跟BMS通讯相关硬件操作。
                  此任务跟BMS通讯任务并非采用消息的方式同步，跟其他任务采用消息
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月08日  叶喜雨
*****************************************************************************/
void TaskControlGunA(void *pdata)
{   
    INT8U err;
    _BSP_MESSAGE *pMsg;   // 定义消息指针
    
    pdata = pdata;
    
    OSTimeDly(SYS_DELAY_1s);  

    APP_ControlHardwareInit();
    OSTimeDly(SYS_DELAY_1s);  
    APP_ControlInit();
    OSTimeDly(SYS_DELAY_2s);
//	BSP_OpenLOCK(GUN_A);
//	BSP_OpenLOCK(GUN_B);
//	
//		BSP_RLCOpen(BSPRLC_GUN_LOCK);
////	
//	BSP_RLCClose(BSPRLC_GUN_LOCK);

    while(1)
    {	
        pMsg = OSQPend(Control_PeventA, SYS_DELAY_20ms, &err); 
        if (err == OS_ERR_NONE)
        {
            switch((_BSP_MSGID)pMsg->MsgID)
            {
                case BSP_MSGID_BMS:         //来自BMS任务
                case BSP_MSGID_BCU: 
                case BSP_MSGID_PCU:
                case BSP_MSGID_INSULATION:
                case BSP_MSGID_PERIOD:
                case BSP_MSGID_GUIYUE:
                    APP_AnalyzeRxDataA(pMsg);
                    break;
                default:
                    break;
            }
        }
    }
}
 
/*****************************************************************************
* Function      : TaskControl
* Description   : 控制任务，此任务控制整个充电的逻辑跟BMS通讯相关硬件操作。
                  此任务跟BMS通讯任务并非采用消息的方式同步，跟其他任务采用消息
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年6月08日  叶喜雨
*****************************************************************************/
void TaskControlGunB(void *pdata)
{   
    INT8U err;
    _BSP_MESSAGE *pMsg;   // 定义消息指针
    
    pdata = pdata;
    
    OSTimeDly(SYS_DELAY_5s);    //A控制任务先起来
//	BSP_OpenLOCK(GUN_A);
//	BSP_OpenLOCK(GUN_B);
//	
//		BSP_RLCOpen(BSPRLC_GUN_LOCK);
////	
//	BSP_RLCClose(BSPRLC_GUN_LOCK);

    while(1)
    {	
        pMsg = OSQPend(Control_PeventB, SYS_DELAY_20ms, &err); 
        if (err == OS_ERR_NONE)
        {
            switch((_BSP_MSGID)pMsg->MsgID)
            {
                case BSP_MSGID_BMS:         //来自BMS任务
                case BSP_MSGID_BCU: 
                case BSP_MSGID_PCU:
                case BSP_MSGID_INSULATION:
                case BSP_MSGID_PERIOD:
                case BSP_MSGID_GUIYUE:
                    APP_AnalyzeRxDataB(pMsg);
                    break;
                default:
                    break;
            }
        }
    }
}
/************************(C)COPYRIGHT 2010 汇誉科技****END OF FILE****************************/
