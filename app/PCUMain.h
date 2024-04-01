/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : pcu.h
* Author              :
* Date First Issued : 2016-07-09
* Version             : V1.0
* Description       :
*----------------------------------------历史版本信息-------------------------------------------
* History             :no
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __PCUMAIN_H_
#define __PCUMAIN_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "OS_CPU.H"
#include "sysconfig.h"
//#include "controlmain.h"
/* Private define-----------------------------------------------------------------------------*/
extern OS_EVENT *TaskPCU_pevent;

//模块的告警状态
__packed typedef struct
{
    __packed union
    {
        INT8U allbits;
        __packed struct
        {
            INT8U DC_Off: 1;    //模块DC侧处于关机状态
            INT8U Mod_Alarm: 1; //模块故障告警状态
            INT8U Protected: 1; //模块是否处于保护过压告警状态
            INT8U Fan_Fault: 1; //模块是否处于风扇故障告警状态
            INT8U Tem_Over: 1;  //模块是否处于温度过热告警状态,阀值为100度
            INT8U Volt_Over: 1; //模块是否处于过压告警状态
            INT8U OutShorted: 1;   //输出短路
            INT8U Commu_Aarm: 1;//模块是否处于通信中断告警状态
        } onebits;
    } State1;
    
    __packed union
    {
        INT8U allbits;
        __packed struct
        {
            INT8U Power_Limit: 1;       //模块是否处于功率限制状态
            INT8U ID_Repeat: 1;         //模块ID重复
            INT8U Current_Unbalance: 1; //模块不均流
            INT8U AC_Phaseloss: 1;      //交流缺相告警
            INT8U AC_Unbalance: 1;      //交流不平衡
            INT8U AC_UVP: 1;            //交流欠压
            INT8U AC_Over: 1;           //交流过压
            INT8U PFC_Off: 1;           //模块PFC处于关机状态
        } onebits;
    } State2;
} _MODULE_STATE_CCU; 

__packed typedef struct
{
    INT16U Vol;   //功率模块电压，0.1V/位
    INT16U Cur;   //功率模块电流，0.1A/位
}_POWER_INFO;

__packed typedef struct
{
    INT8U ModuleMaxTep;    //充电模块最高温度-50°
    INT8U ShutDownNum;     //模块关机模块号，异常为具体的模块号 正常为0
    INT8U ComErrNum ;      //通信异常模块号，异常为具体的模块号 正常为0
    INT8U ModuleErrnum ;   //故障异常模块号，异常为具体的模块号 正常为0
    _MODULE_STATE_CCU ModuleState;
    __packed union
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U InACOverVolBit:1;           //模块输入过压异常告警
            INT8U InACUnderVolBit:1;          //模块输入欠压告警
            INT8U RevBits:6;                  //预留，写0
        }OneByte;
    }State3;
}_APP_CHARGE_STATE2;


//模块整体信息，后续需要补全
typedef struct
{
    _POWER_INFO OutputInfo;      				//模块输出信息
    _APP_CHARGE_STATE2 TotalState; 				//模块总状态
}_POWER_MODULE_INFO;

extern _POWER_MODULE_INFO PowerModuleInfo[GUN_MAX]; //模块整体信息

/*****************************************************************************
* Function      : GetModuleReturnACVol
* Description   : 获取模块返回得三项电压
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016年7月8日  叶喜雨
*****************************************************************************/
INT16U GetModuleReturnACVol(void);

/***********************************************************************************************
* Function      : Updata_ModuleInfo
* Description   : 跟新模块数据（跟新  PowerModuleInfo）
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020年6月5日
***********************************************************************************************/
INT8U Updata_ModuleInfo(void);
#endif

