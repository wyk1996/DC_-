/*****************************************Copyright(C)******************************************
*******************************************杭州汇誉*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: DispStatus.h
* Author			:
* Date First Issued	: 
* Version			: 
* Description		: 
*----------------------------------------历史版本信息-------------------------------------------
* History			:	        
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __DISPSTATUS_H_
#define __DISPSTATUS_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
//#include "DwinProtocol.h"
/* Private define-----------------------------------------------------------------------------*/
//显示状态编�?
typedef enum
{
	SHOW_CODE_ERR = 0,        // 显示密码输入错误状�?
	SHOW_CFG_FAIL,			  //显示配置信息设置失败
	SHOW_CFG_SUCCESS,		  //显示配置信息设置成功
	SHOW_CFG_INPURERR,		  //显示配置信息输入有误
	SHOW_CHARGE_AUTO,  		  //自动分配
	SHOW_CHARGE_JUN,		  //均充
	SHOW_CHARGE_LUN,		  //轮充
	SHOW_LOCK_SUCCESS,		  //解锁成功
	SHOW_LOCK_FAIL,			  //解锁失败
	SHOW_BATTERY_TYPE1,		  //电池类型1
	SHOW_BATTERY_TYPE2,		  
	SHOW_BATTERY_TYPE3,
	SHOW_BATTERY_TYPE4,
	SHOW_BATTERY_TYPE5,
	SHOW_BATTERY_TYPE6,
	SHOW_BATTERY_TYPE7,
	SHOW_BATTERY_TYPE8,
	SHOW_BATTERY_OTHER,		  //其它电池类型
	SHOW_GUNA_ERR,			//A枪故�?
	SHOW_GUNB_ERR,			//B枪故�?
	SHOW_GUNA_WARNING,		//A枪告�?
	SHOW_GUNB_WARNING,		//B枪告�?
	SHOW_GUN_NOTE,			//无故�?
	SHOW_HAND_EXIT,			//手动模式退出提�?
	SHOW_DIV_ERR,			//设备信息中显示故�?
	SHOW_DIV_WARNING,		//设备信息中显示告�?
	SHOW_DIV_NORMAL,		//设备信息中显示正�?
	SHOW_GUN_HOMING,		//枪归�?
	SHOW_GUN_UNHOMING,		//枪未归位�?
	SHOW_STATE_CONNECT,		//连接
	SHOW_STATE_UNCONNECT,	//未连�?
	
	
	//启动完成帧原�?
	SHOW_STARTEND_SUCCESS,                   
	SHOW_ENDFAIL_HANDERR ,                      
	SHOW_ENDFAIL_RECVTIMEOUT  ,               
	SHOW_ENDFAIL_EMERGENCY,                  
	SHOW_ENDFAIL_DOORERR ,                      
	SHOW_ENDFAIL_SURGEARRESTER,                 
	SHOW_ENDFAIL_SMOKEERR,                       
	SHOW_ENDFAIL_ACSWERR ,                    
	SHOW_ENDFAIL_ACSWERR1,                       
	SHOW_ENDFAIL_ACSWERR2,                    
	SHOW_ENDFAIL_ACINERR,                     

	SHOW_ENDFAIL_CUPTEMPERATURE,               
	SHOW_ENDFAIL_GUNUPTEMPERATURE,               
	SHOW_ENDFAIL_ELECLOCKERR,                    
	SHOW_ENDFAIL_INSOLUTIONERR,                  
	SHOW_ENDFAIL_BATREVERSE,                    
	SHOW_ENDFAIL_OUTSWERR1,                   
	SHOW_ENDFAIL_OUTSWERR2,                 
	SHOW_ENDFAIL_OUTSWERR3,                   
	SHOW_ENDFAIL_SAMELEVELSW1,               
	SHOW_ENDFAIL_SAMELEVELSW2,                  


	SHOW_ENDFAIL_LEAKOUTTIMEOUT,                 
	SHOW_ENDFAIL_MODULEERR ,                   	
	SHOW_ENDFAIL_CHARGEMODULEERR ,              
	SHOW_ENDFAIL_OUTVOLTVORE ,                  
	SHOW_ENDFAIL_OUTVOLTUNDER ,                  
	SHOW_ENDFAIL_OUTCURROVER ,                   
	SHOW_ENDFAIL_SHORTCIRCUIT ,                  
	SHOW_ENDFAIL_BATVOLTERR1 ,                     
	SHOW_ENDFAIL_BATVOLTERR2 ,                     
	SHOW_ENDFAIL_BATVOLTERR3 ,                     
  
	SHOW_ENDFAIL_ADVOLTERR1,              
	SHOW_ENDFAIL_ADVOLTERR2 ,                  
	SHOW_ENDFAIL_ADVOLTERR3 ,                      
	SHOW_ENDFAIL_BRMTIMEOUT ,                    
	SHOW_ENDFAIL_BRMDATAERR ,                    
	SHOW_ENDFAIL_BCPTIMEOUT ,                     
	SHOW_ENDFAIL_BCPDATAERR ,                     
	SHOW_ENDFAIL_BROWAITTIMEOUT ,                 
	SHOW_ENDFAIL_BRORUNTIMEOUT ,
	SHOW_ENDFAIL_VIN,	
	SHOW_ENDFAIL_OTHERERR , 
	SHOW_ENDFAIL_RECVCCUTIMEOUT,	//接收CCU帧超�?
	SHOW_NULL,				//清空显示显示�?
//停止完成帧原�?
   SHOW_STOP_ERR_NONE,
   SHOW_STOP_TCUNORMAL,             
   SHOW_STOP_TCUERR  ,                 
   SHOW_STOP_CCUERR,						
   SHOW_STOP_STARTEDTOUT ,              
   SHOW_STOP_HANDERR,                   
   SHOW_STOP_HEARTTOUT,                 
   SHOW_STOP_WAITTOIT ,                 
   SHOW_STOP_EMERGENCY,                  
   SHOW_STOP_DOORERR,   
	SHOW_STOP_XQERR,                    //��б
	SHOW_STOP_SJERR,                    //ˮ��
	SHOW_SHOW_STOP_SWAERR,                    //ֱ���Ӵ���A
	SHOW_STOP_SWBERR,                    //ֱ���Ӵ���B
	SHOW_STOP_QJERR,                    //�Ž�   
   SHOW_STOP_SURGEARRESTER,            

   SHOW_STOP_SMOKEERR,              
   SHOW_STOP_ACSWERR1,                 
   SHOW_STOP_ACSWERR2,                   
   SHOW_STOP_ACSWERR3,                   
   SHOW_STOP_ACINERR ,                   
   SHOW_STOP_CUPTEMPERATURE,           
   SHOW_STOP_GUNUPTEMPERATURE,         
   SHOW_STOP_ELECLOCKERR,               
   SHOW_STOP_OUTSWERR1,                  
   SHOW_STOP_OUTSWERR2,                  
   
   SHOW_STOP_OUTSWERR3,              
   SHOW_STOP_SAMELEVELSW1,               
   SHOW_STOP_SAMELEVELSW2,               
   SHOW_STOP_LEAKOUTTIMEOUT,                 
   SHOW_STOP_BMSPOWERERR ,                  
   
   SHOW_STOP_CHARGEMODULEERR ,               
   SHOW_STOP_OUTVOLTVORE ,                
   SHOW_STOP_OUTVOLTUNDER ,               
   SHOW_STOP_OUTCURROVER ,                  
   SHOW_STOP_SHORTCIRCUIT,              
   
   SHOW_STOP_BCLTIMTOUT,					
   SHOW_STOP_BCSTIMTOUT ,					
   SHOW_STOP_BSMTIMTOUT ,						
   SHOW_STOP_BSMBATVOLTHIGH ,				
   SHOW_STOP_BSMBATVOLTLOW ,                  
   SHOW_STOP_BSMSOCHIGH ,                 
   SHOW_STOP_BSMSOCLOW ,                    
   SHOW_STOP_BSMCURRUP ,                     
   SHOW_STOP_BSMTEMPUP ,                     
   SHOW_STOP_BSMINSOLUTION ,                 
   
	SHOW_STOP_BSMSWERR,                   
	SHOW_STOP_BSMNORMAL ,                   
	SHOW_STOP_BSMERR ,                      
	SHOW_STOP_CCUBSMERR ,     
	SHOW_STOP_MATERFAIL,
	SHOW_STOP_OTHERERR ,

	SHOW_STOP_BSTINSULATIONERR,		 //��Ե����
	SHOW_STOP_BSTSWOVERT,				//������������¹���	
	SHOW_STOP_BSTELOVERT,				//BMSԪ�����¹���
	SHOW_STOP_CERR,     				//�������������
	SHOW_STOP_BSTBATOVERT, //�������¹���
	SHOW_STOP_BSTHIGHRLCERR, //��ѹ�̵�������
	SHOW_STOP_BSTTPTWO,//����2��ѹ������
	SHOW_STOP_BSTOTHERERR,//BST��������
	SHOW_STOP_BALANCE,
	SHOW_STOP_SOC,

	SHOW_CARD_LOCK,		//卡位结算
	SHOW_CARD_UNLOCK,	//卡已经结�?
	SHOW_CARD_PERLLOCK,	//后续解锁
	SHOW_START_FAIL,
    SHOW_GUN_A,
	SHOW_GUN_B, 
	SHOW_START_APP ,  
	SHOW_START_CARD,
	SHOW_TIME,
	SHOW_SYS_EMERGENCY,
	SHOW_SYS_MJ,
	SHOW_SYS_SJ,
	SHOW_SYS_QX,
	SHOW_SYS_ADC,
	SHOW_SYS_BDC,
	SHOW_SYS_QJ,
	SHOW_SYS_PCUTIMEOUT,
	SHOW_SYS_OTHERFAIL,
	SHOW_SYS_AGAINGUN,
	SHOW_SYS_CHARGE,
	SHOW_SYS_CARD,	
	SHOW_SYS_NET,
	SHOW_SYS_GUN,	
	SHOW_SYS_START,			
	SHOW_SYS_FAIL,
	SHOW_SYS_STOP,
	SHOW_SYS_NULL,
	SHOW_NET_DJ,
	SHOW_NET_WL,
	SHOW_LOCK_CLOSE,
	SHOW_LOCK_OPEN,
	SHOW_MODULE_YL_20KWH,
	SHOW_MODULE_YL_20KW,
	SHOW_MODULE_SH,
	SHOW_MODULE_YL_15KWH,
	SHOW_MODULE_YL_15KW,
	SHOW_MODULE_TH_30KW,
	SHOW_MODULE_TH_40KW,
	SHOW_MODULE_YL_30KW,
	SHOW_MODULE_YL_20KW_1000V,  //����20KW-1000V
	
	SHOW_XY_HY,			//����
	SHOW_XY_YKC,		//�ƿ��
	SHOW_XY_AP,			//����
	SHOW_XY_XJ,			//С��
	SHOW_XY_YL1,		//Ԥ��1 --�����
	SHOW_XY_YL2,		//Ԥ��2 --����
	SHOW_XY_YL3,		//Ԥ��3 --����
	SHOW_XY_YL4,		//Ԥ��4 --����
	SHOW_XY_66,			//Ԥ��5
	SHOW_XY_JG,			//Ԥ��4
	SHOW_XY_TT,			//Ԥ��5
	SHOW_XY_ZSH,		//Ԥ��5
	SHOW_MAX,
}_SHOW_NUM;

/* Private variables--------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Dis_ShowErr
* Description	: ç•Œé¢æ˜¾ç¤ºçŠ¶æ€?
				 add_show          	æ˜¾ç¤ºåœ°å�?
				 show_num     		æ˜¾ç¤ºç¼–å�?
				 show_color   		æ˜¾ç¤ºé¢œè‰�?
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018Äê7ÔÂ27ÈÕ
***********************************************************************************************/
INT8U Dis_ShowErr(INT16U add_show,_SHOW_NUM show_num,_SHOW_COLOR show_color);

/***********************************************************************************************
* Function		: Dis_ShowTime
* Description	:          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:
***********************************************************************************************/
INT8U Dis_ShowTime(INT16U add_show ,_BSPRTC_TIME gRTC);
/***********************************************************************************************
* Function		: Dis_ShowStatus
* Description	: 界面显示状�?
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018�?�?7�?
***********************************************************************************************/
INT8U Dis_ShowStatus(INT16U add_show,_SHOW_NUM show_num,_SHOW_COLOR show_color);

/***********************************************************************************************
* Function		: Dis_ShowStatus
* Description	: 界面显示状�?
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	: 2018�?�?7�?
***********************************************************************************************/
INT8U Dis_Show(INT16U add_show,_SHOW_NUM show_num);
/***********************************************************************************************
* Function		: Dis_ShowStartErr
* Description	: ÏÔÊ¾Æô¶¯Ê§°ÜÔ­Òò
				 code ¹ÊÕÏÂë 0Î»Õý³££¬²»ÏÔÊ¾          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018Äê7ÔÂ27ÈÕ
***********************************************************************************************/
INT8U Dis_ShowStartErr(INT16U add_show ,INT8U code);

/***********************************************************************************************
* Function		: Dis_ShowGunErr
* Description	: Ö÷½çÃæÏÔÊ¾Ç¹´íÎó×´Ì¬ Ó¦µ÷ÓÃDis_ShowStatusº¯Êý´æÔÚbug£¬¹Êµ¥¶ÀÐ´Ò»¸öº¯Êý          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018Äê7ÔÂ27ÈÕ
***********************************************************************************************/
INT8U Dis_ShowGunErr(_GUN_NUM gun,_SHOW_NUM show_num,INT16U add_show,_SHOW_COLOR show_color);

/***********************************************************************************************
* Function		: Dis_ShowStopReason
* Description	: ÏÔÊ¾Í£Ö¹Ô­Òò
				 code ¹ÊÕÏÂë 0Î»Õý³££¬²»ÏÔÊ¾          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018Äê7ÔÂ27ÈÕ
***********************************************************************************************/
INT8U Dis_ShowStopReason(INT16U add_show ,INT8U code);

/***********************************************************************************************
* Function		: Dis_ShowDivStatus
* Description	:ÏÔÊ¾Éè±¸×´Ì¬
				 add_show          	æ˜¾ç¤ºåœ°å�?
				 show_num     		æ˜¾ç¤ºç¼–å�?
				 show_color   		æ˜¾ç¤ºé¢œè‰�?
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018Äê7ÔÂ27ÈÕ
***********************************************************************************************/
INT8U Dis_ShowDivStatus(INT16U add_show,_SHOW_NUM show_num,_SHOW_COLOR show_color);

/***********************************************************************************************
* Function		: Dis_ShowCopy
* Description	: ç•Œé¢æ˜¾ç¤ºçŠ¶æ€?
				 add_show          	æ˜¾ç¤ºåœ°å�?
				 show_num     		æ˜¾ç¤ºç¼–å�?
				 show_color   		æ˜¾ç¤ºé¢œè‰�?
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018Äê7ÔÂ27ÈÕ
***********************************************************************************************/
INT8U Dis_ShowCopy(INT8U* pdata,_SHOW_NUM show_num);
#endif
/************************(C)COPYRIGHT 2018 杭州汇誉*****END OF FILE****************************/
