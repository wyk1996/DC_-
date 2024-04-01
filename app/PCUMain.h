/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName          : pcu.h
* Author              :
* Date First Issued : 2016-07-09
* Version             : V1.0
* Description       :
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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

//ģ��ĸ澯״̬
__packed typedef struct
{
    __packed union
    {
        INT8U allbits;
        __packed struct
        {
            INT8U DC_Off: 1;    //ģ��DC�ദ�ڹػ�״̬
            INT8U Mod_Alarm: 1; //ģ����ϸ澯״̬
            INT8U Protected: 1; //ģ���Ƿ��ڱ�����ѹ�澯״̬
            INT8U Fan_Fault: 1; //ģ���Ƿ��ڷ��ȹ��ϸ澯״̬
            INT8U Tem_Over: 1;  //ģ���Ƿ����¶ȹ��ȸ澯״̬,��ֵΪ100��
            INT8U Volt_Over: 1; //ģ���Ƿ��ڹ�ѹ�澯״̬
            INT8U OutShorted: 1;   //�����·
            INT8U Commu_Aarm: 1;//ģ���Ƿ���ͨ���жϸ澯״̬
        } onebits;
    } State1;
    
    __packed union
    {
        INT8U allbits;
        __packed struct
        {
            INT8U Power_Limit: 1;       //ģ���Ƿ��ڹ�������״̬
            INT8U ID_Repeat: 1;         //ģ��ID�ظ�
            INT8U Current_Unbalance: 1; //ģ�鲻����
            INT8U AC_Phaseloss: 1;      //����ȱ��澯
            INT8U AC_Unbalance: 1;      //������ƽ��
            INT8U AC_UVP: 1;            //����Ƿѹ
            INT8U AC_Over: 1;           //������ѹ
            INT8U PFC_Off: 1;           //ģ��PFC���ڹػ�״̬
        } onebits;
    } State2;
} _MODULE_STATE_CCU; 

__packed typedef struct
{
    INT16U Vol;   //����ģ���ѹ��0.1V/λ
    INT16U Cur;   //����ģ�������0.1A/λ
}_POWER_INFO;

__packed typedef struct
{
    INT8U ModuleMaxTep;    //���ģ������¶�-50��
    INT8U ShutDownNum;     //ģ��ػ�ģ��ţ��쳣Ϊ�����ģ��� ����Ϊ0
    INT8U ComErrNum ;      //ͨ���쳣ģ��ţ��쳣Ϊ�����ģ��� ����Ϊ0
    INT8U ModuleErrnum ;   //�����쳣ģ��ţ��쳣Ϊ�����ģ��� ����Ϊ0
    _MODULE_STATE_CCU ModuleState;
    __packed union
    {
        INT8U AllBits;
        __packed struct
        {
            INT8U InACOverVolBit:1;           //ģ�������ѹ�쳣�澯
            INT8U InACUnderVolBit:1;          //ģ������Ƿѹ�澯
            INT8U RevBits:6;                  //Ԥ����д0
        }OneByte;
    }State3;
}_APP_CHARGE_STATE2;


//ģ��������Ϣ��������Ҫ��ȫ
typedef struct
{
    _POWER_INFO OutputInfo;      				//ģ�������Ϣ
    _APP_CHARGE_STATE2 TotalState; 				//ģ����״̬
}_POWER_MODULE_INFO;

extern _POWER_MODULE_INFO PowerModuleInfo[GUN_MAX]; //ģ��������Ϣ

/*****************************************************************************
* Function      : GetModuleReturnACVol
* Description   : ��ȡģ�鷵�ص������ѹ
* Input         : void *pdata  
* Output        : None
* Note(s)       : 
* Contributor   : 2016��7��8��  Ҷϲ��
*****************************************************************************/
INT16U GetModuleReturnACVol(void);

/***********************************************************************************************
* Function      : Updata_ModuleInfo
* Description   : ����ģ�����ݣ�����  PowerModuleInfo��
* Input         :
* Output        :
* Note(s)       :
* Contributor   : 2020��6��5��
***********************************************************************************************/
INT8U Updata_ModuleInfo(void);
#endif

