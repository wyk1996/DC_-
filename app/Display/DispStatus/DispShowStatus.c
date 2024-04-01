/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: DispStatus.c
* Author			:
* Date First Issued	:
* Version			:
* Description		: 提供系统状态信息，提供显示屏显示中文使用，此文本格式为ANSI
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include <stdlib.h>
#include "DataChangeInterface.h" 
#include "DwinProtocol.h"
#include "DispShowStatus.h"
#include "DisplayMain.h"
#include "DispkeyFunction.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
//鏄剧ず鐘舵�佸鐞?
typedef struct
{
	_SHOW_NUM ShowNum;   //鏄剧ず鐘舵�佺紪鍙?
	const char* pStr;      //鏄剧ず鍦板潃 
	 INT16U ShowLen;      //鏄剧ず鍦板潃 
}_SHOW_STATUS;
/* Private variables--------------------------------------------------------------------------*/
_SHOW_STATUS ShowStatus[SHOW_MAX] = 
{
	{SHOW_CODE_ERR     ,  "密码输入错误！"    	,14}, 
	
	{SHOW_CFG_FAIL     ,  "设置失败！"    		,10}, 
	
	{SHOW_CFG_SUCCESS  ,  "设置成功！"    		,10}, 
	
	{SHOW_CFG_INPURERR ,  "数据输入有误！"    	,14}, 
	
	{SHOW_CHARGE_AUTO,    "自动分配"			,8 },			  		  
	
	{SHOW_CHARGE_JUN,	  "均充"	            ,4 },
	
	{SHOW_CHARGE_LUN,	  "轮充"	            ,4 },
		
	{SHOW_LOCK_SUCCESS,	  "解锁成功"	   		,8 },
	
	{SHOW_LOCK_FAIL,	  "解锁失败"	        ,8 },

	{SHOW_BATTERY_TYPE1,  "铅酸电池"			,8 },
	
	{SHOW_BATTERY_TYPE2,  "镍氢电池"			,8 },
	
	{SHOW_BATTERY_TYPE3,  "磷酸铁锂电池"		,12},
	
	{SHOW_BATTERY_TYPE4,  "锰酸锂电池"			,10},
	
	{SHOW_BATTERY_TYPE5,  "钴酸锂电池"			,10},
	
	{SHOW_BATTERY_TYPE6,  "三元材料电池"		,12},
	
	{SHOW_BATTERY_TYPE7,  "聚合物锂离子电池"	,14},
	
	{SHOW_BATTERY_TYPE8,  "钛酸锂电池"			,10},
	
	{SHOW_BATTERY_OTHER,  "其它电池"			,8 },
	
	{SHOW_GUNA_ERR,		  "A枪故障" 			,8 },
		
	{SHOW_GUNB_ERR,       "B枪故障" 			,8 },
	
	{SHOW_GUNA_WARNING,	  "A枪告警"				,8 },
	
	{SHOW_GUNB_WARNING,	  "B枪告警"				,8 },
	
	{SHOW_GUN_NOTE,       "        "            ,8 },	

	{SHOW_HAND_EXIT,      "请退出手动模式！"    ,16},	
	
	{SHOW_DIV_ERR,        "故障"				,4 },	
	
	{SHOW_DIV_WARNING,    "告警"				,4 },
	
	{SHOW_DIV_NORMAL,     "正常"				,4 },	

	{SHOW_GUN_HOMING,	  "归位"				,4 },
		
	{SHOW_GUN_UNHOMING,	  "未归位"			    ,6 },
	
	{SHOW_STATE_CONNECT,  "连接"				,4 },	
	
	{SHOW_STATE_UNCONNECT,"未连接"				,6 },
	
	{SHOW_NULL,"                              "				,30 },
	
	
	//启动完成帧原因，与启动完成帧对应起来
	{SHOW_STARTEND_SUCCESS,				"启动成功",						8},	
	{SHOW_ENDFAIL_HANDERR,				"导引故障",						8},	
	{SHOW_ENDFAIL_RECVTIMEOUT,			"通信超时",						8},	
	{SHOW_ENDFAIL_EMERGENCY	,			"急停故障",						8},	
	{SHOW_ENDFAIL_DOORERR,				"门禁故障",						8},	
	{SHOW_ENDFAIL_SURGEARRESTER,		"避雷器故障",					10},	
	{SHOW_ENDFAIL_SMOKEERR,				"断路器故障",					10},	
	{SHOW_ENDFAIL_ACSWERR1,				"交流接触器故障",				14},	
	{SHOW_ENDFAIL_ACSWERR2,				"交流接触器粘连",				14},	
	{SHOW_ENDFAIL_ACINERR,				"交流输入故障",					12},	
	
	{SHOW_ENDFAIL_CUPTEMPERATURE,		"充电桩过温",					10},	
	{SHOW_ENDFAIL_GUNUPTEMPERATURE,		"充电接口过温",					12},	
	{SHOW_ENDFAIL_ELECLOCKERR,			"电磁锁故障",					10},	
	{SHOW_ENDFAIL_INSOLUTIONERR,		"绝缘检测故障",					12},	
	{SHOW_ENDFAIL_BATREVERSE,			"电池反接",						8},	
	{SHOW_ENDFAIL_OUTSWERR1,			"直流接触器故障",				14},	
	{SHOW_ENDFAIL_OUTSWERR2,			"直流接触器粘连",				14},	
	{SHOW_ENDFAIL_OUTSWERR3,			"输出熔断器故障",				14},	
	{SHOW_ENDFAIL_SAMELEVELSW1,			"并联接触器故障",				14},	
	{SHOW_ENDFAIL_SAMELEVELSW2,			"并联接触器粘连",				14},
	
	{SHOW_ENDFAIL_LEAKOUTTIMEOUT,		"泄放回路故障",					12},
	{SHOW_ENDFAIL_CHARGEMODULEERR,		"模块故障",						8},	
	{SHOW_ENDFAIL_MODULEERR,			"模块故障",						8},	
	{SHOW_ENDFAIL_OUTVOLTVORE,			"输出电压过压",					12},	
	{SHOW_ENDFAIL_OUTVOLTUNDER,			"输出电压欠压",					12},	
	{SHOW_ENDFAIL_OUTCURROVER,			"输出电流过流",					12},	
	{SHOW_ENDFAIL_SHORTCIRCUIT,			"输出短路",						8},	
	{SHOW_ENDFAIL_BATVOLTERR1, 			"最高允许电压异常",				16},	                    
	{SHOW_ENDFAIL_BATVOLTERR2,  		"绝缘监测前电池大于10V"  ,		21},	
	{SHOW_ENDFAIL_BATVOLTERR3,			"预充报文与采样电压大于5%",		24},
	
	{SHOW_ENDFAIL_ADVOLTERR1, 			"后级电压小于最小输出",			20},
	{SHOW_ENDFAIL_ADVOLTERR2,       	"后级电压大于最大输出",			20},
	{SHOW_ENDFAIL_ADVOLTERR3,       	"后级电压大于BMS允许最高电压",	27},
	{SHOW_ENDFAIL_BRMTIMEOUT,        	"BRM超时",						7},
	{SHOW_ENDFAIL_BRMDATAERR,      		"BRM数据异常",					11},
	{SHOW_ENDFAIL_BCPTIMEOUT,       	"BCP超时",						7},
	{SHOW_ENDFAIL_BCPDATAERR,      		"BCP数据异常",					11},
	{SHOW_ENDFAIL_BROWAITTIMEOUT,   	"BRO00超时",					9},
	{SHOW_ENDFAIL_BRORUNTIMEOUT,     	"BROAA超时",					9},
	{SHOW_ENDFAIL_VIN,     				"VIN启动失败",					11},
	{SHOW_ENDFAIL_OTHERERR ,            "其他故障",					    8},
	{SHOW_ENDFAIL_RECVCCUTIMEOUT,       "接收CCU启动帧超时",			 17},					
	//停止原因，与停止完成帧对应起来
				
	{SHOW_STOP_ERR_NONE,				"正常停止",						8},
	{SHOW_STOP_TCUNORMAL,              	"计费单元正常停止",				16},
	{SHOW_STOP_TCUERR,               	"计费单元自身故障",           	16},                                                                                                                                            
	{SHOW_STOP_CCUERR,					"计费单元判断充电控制器故障",	26},
	{SHOW_STOP_STARTEDTOUT ,       		"启动完成状态确认帧超时",		22},
	{SHOW_STOP_HANDERR,            		"充电枪故障",					10},
	{SHOW_STOP_HEARTTOUT,         		"心跳超时",						8},
	{SHOW_STOP_WAITTOIT ,         		"充电中暂停超时",				14},
	{SHOW_STOP_EMERGENCY,        		"急停",							4},
	{SHOW_STOP_DOORERR,            		"门禁",							4},
	{SHOW_STOP_XQERR,                    "倾斜",							4},
	{SHOW_STOP_SJERR,                    "水浸",							4},
	{SHOW_SHOW_STOP_SWAERR,              "直流接触器故障",							14},
	{SHOW_STOP_SWBERR,                    "直流接触器故障",							14},
	{SHOW_STOP_QJERR,                    "桥接接触器故障",							14},
	{SHOW_STOP_SURGEARRESTER,      		"避雷器",						6},

	{SHOW_STOP_SMOKEERR,              	"烟感",							4},
	{SHOW_STOP_ACSWERR1,          		"交流输入断路器故障",			18},
	{SHOW_STOP_ACSWERR2,           		"交流输入接触器误动",			18},
	{SHOW_STOP_ACSWERR3,             	"交流输入接触器粘连",			18},
	{SHOW_STOP_ACINERR ,               	"交流输入故障",					12},
	{SHOW_STOP_CUPTEMPERATURE,         	"充电桩过温",					10},
	{SHOW_STOP_GUNUPTEMPERATURE,       	"充电桩接口过温",				14},
	{SHOW_STOP_ELECLOCKERR,            	"充电桩电磁锁故障",				16},
	{SHOW_STOP_OUTSWERR1,           	"输出接触器误动故障",			18},
	{SHOW_STOP_OUTSWERR2,        		"输出接触器粘连",				14},

	{SHOW_STOP_OUTSWERR3,              	"输出熔断器故障",				14},
	{SHOW_STOP_SAMELEVELSW1,         	"并联接触器误动故障",			18},
	{SHOW_STOP_SAMELEVELSW2,         	"并联接触器故障",				14},
	{SHOW_STOP_LEAKOUTTIMEOUT,      	"泄放回路",						8},
	{SHOW_STOP_BMSPOWERERR ,         	"辅助电源故障",					12},
	{SHOW_STOP_CHARGEMODULEERR ,     	"充电机模块故障",				14},
	{SHOW_STOP_OUTVOLTVORE ,       		"输出电压异常",					12},
	{SHOW_STOP_OUTVOLTUNDER ,         	"输出欠压",						8},
	{SHOW_STOP_OUTCURROVER ,           	"输出电流过流",					12},
	{SHOW_STOP_SHORTCIRCUIT,          	"输出短路",						8},

	{SHOW_STOP_BCLTIMTOUT,				"BCL超时",						7},
	{SHOW_STOP_BCSTIMTOUT ,				"BCS超时",						7},
	{SHOW_STOP_BSMTIMTOUT ,				"BSM超时",						7},
	{SHOW_STOP_BSMBATVOLTHIGH ,			"BSM单体电压过高",				15},
	{SHOW_STOP_BSMBATVOLTLOW ,        	"BSM单体电压过低",				15},
	{SHOW_STOP_BSMSOCHIGH ,           	"BSMSOC过高",					10},
	{SHOW_STOP_BSMSOCLOW ,             	"BSMSOC过低",					10},
	{SHOW_STOP_BSMCURRUP ,          	"BSM过流",						7},
	{SHOW_STOP_BSMTEMPUP ,          	"BSM过温",						7},
	{SHOW_STOP_BSMINSOLUTION ,        	"BSM绝缘",						7},

	{SHOW_STOP_BSMSWERR,            	"BSM连接器故障",				13},
	{SHOW_STOP_BSMNORMAL ,            	"BMS正常停止",					11},
	{SHOW_STOP_BSMERR ,              	"车端异常停止",					12},
	{SHOW_STOP_CCUBSMERR ,       		"充电桩判断BMS故障",			17},
	{SHOW_STOP_MATERFAIL ,       		"电表故障",						 8},
	{SHOW_STOP_OTHERERR ,             	"充电桩其他故障",				14},
	
	{SHOW_STOP_BSTINSULATIONERR ,        	"BST绝缘故障",				11},
	{SHOW_STOP_BSTSWOVERT ,             	"BST输出连接器过温",		17},
	{SHOW_STOP_BSTELOVERT ,             	"BST元件过温",				11},
	{SHOW_STOP_CERR ,             				"BST输出连接器",			11},
	{SHOW_STOP_BSTBATOVERT ,             	"BST电池组过温故障",		17},
	{SHOW_STOP_BSTHIGHRLCERR ,             	"BST高压继电器故障",		17},
	{SHOW_STOP_BSTTPTWO ,             		"BST检测点二故障",			15},
	{SHOW_STOP_BSTOTHERERR ,             	"BST其他故障",				11},

	{SHOW_STOP_BALANCE ,             	"余额不足",				8},
	{SHOW_STOP_SOC ,             	"设定SOC达到",				11},
	
	//卡状态
	{SHOW_CARD_LOCK ,       		"未结算",							6},
	{SHOW_CARD_UNLOCK ,          	"已结算",							6},
	{SHOW_CARD_PERLLOCK ,          	"补充结算",							8},
	{SHOW_START_FAIL ,          	"启动失败",							8},
	{SHOW_GUN_A ,       			"A枪",							3},
	{SHOW_GUN_B ,          			"B枪",							3},
	{SHOW_START_APP ,          		"后台启动",							8},
	{SHOW_START_CARD ,          	"刷卡启动",							8},
	//时间格式显示
	{SHOW_TIME ,          	"--++",							4},
//	SHOW_SYS_MJ,
//	SHOW_SYS_SJ,
//	SHOW_SYS_QX,
//	SHOW_SYS_ADC,
//	SHOW_SYS_BDC,
//	SHOW_SYS_QJ,
	//系统故障，目前只写这么几个，后续细分需要添加
	{SHOW_SYS_EMERGENCY ,          	"急停故障",							8},
	{SHOW_SYS_MJ ,          	"门禁故障",							8},
	{SHOW_SYS_SJ ,          	"水浸故障",							8},
	{SHOW_SYS_QX ,          	"倾斜故障",							8},
	{SHOW_SYS_ADC ,          	"SW_A故障",							8},
	{SHOW_SYS_BDC ,          	"SW_B故障",							8},
	{SHOW_SYS_QJ ,          	"SW_桥接故障",					11},
	{SHOW_SYS_PCUTIMEOUT ,          "PCU故障",					7},
	{SHOW_SYS_OTHERFAIL ,          "系统故障",					8},
	{SHOW_SYS_AGAINGUN ,          	"重新插枪",					8},
	{SHOW_SYS_CHARGE,				"正在充电",					8},		
	{SHOW_SYS_CARD,					"刷卡结算",					8},	
	{SHOW_SYS_NET,				"扫码充电",					8},	
	{SHOW_SYS_GUN,				"请插电枪",					8},	
	{SHOW_SYS_START,				"正在启动",					8},	
	{SHOW_SYS_FAIL,					"启动失败",					8},	
	{SHOW_SYS_STOP,					"正在停止",					8},	
	{SHOW_SYS_NULL,					"                 "			,20 },
	{SHOW_NET_DJ,					"单机"						,4 },
	{SHOW_NET_WL,					"网络"						,4 },
	{SHOW_LOCK_CLOSE,				"常闭"						,4 },
	{SHOW_LOCK_OPEN,				"常开"						,4 },
	{SHOW_MODULE_YL_20KWH,			"20kw永联恒功率"			,14},
	{SHOW_MODULE_YL_20KW,			"20kw永联非恒功率"			,16 },
	{SHOW_MODULE_SH,				"15kw盛弘"					,8 },
	{SHOW_MODULE_YL_15KWH,			"15kw永联恒功率"			,14 },
	{SHOW_MODULE_YL_15KW,			"20kw盛弘"			,8 },
	{SHOW_MODULE_TH_30KW,			"30kw通合"			,8 },
	{SHOW_MODULE_TH_40KW,			"40kw通合"			,8 },
	{SHOW_MODULE_YL_30KW,			"30kw永联"			,8 },
	{SHOW_MODULE_YL_20KW_1000V,		"20kw永联1kv"		,12},
	
	
	{SHOW_XY_HY,					"汇誉"					,4 },
	{SHOW_XY_YKC,					"云快充"				,6 },
	{SHOW_XY_AP,					"安培"					,4 },
	{SHOW_XY_XJ,					"小桔"					,4 },
	{SHOW_XY_YL1,					"达克云"				,5 },
	{SHOW_XY_YL2,					"安充"					,4 },
	{SHOW_XY_YL3,					"星星"					,4 },
	{SHOW_XY_YL4,					"塑云"					,5 },
	{SHOW_XY_66,					"66"					,2 },
	{SHOW_XY_JG,					"精工"					,4 },
	{SHOW_XY_TT,					"铁塔"					,4 },
	{SHOW_XY_ZSH,					"中石化"				,6 },
};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Dis_ShowStartErr
* Description	: 显示启动失败原因
				 code 故障码 0位正常，不显示          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowStartErr(INT16U add_show ,INT8U code)
{	
	char * buf[30];
	ST_Menu* CurMenu = GetCurMenu();
	if(CurMenu == NULL) 
	{
		return FALSE;
	}
	memset(buf,0,sizeof(buf));
	if(code > 0)
	{
		snprintf((char *)buf, sizeof(buf), "启动失败，故障码为E%02x",code);
	}

	PrintStr(add_show,(INT8U *)buf,30);   //显示状态
	SetVariColor(0x0030,RED);
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowTime
* Description	: 显示时间         				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowTime(INT16U add_show ,_BSPRTC_TIME gRTC)
{	
	char * buf[30];
	ST_Menu* CurMenu = GetCurMenu();
	if(CurMenu == NULL) 
	{
		return FALSE;
	}
	memset(buf,0,sizeof(buf));
	
	snprintf((char *)buf, sizeof(buf), "20%02x-%02x-%02x %02x:%02x:%02x",(gRTC.Year),(gRTC.Month),(gRTC.Day),\
		(gRTC.Hour),(gRTC.Minute),(gRTC.Second));


	PrintStr(add_show,(INT8U *)buf,30);   //显示状态
	SetVariColor((add_show & 0xFFC0 ) + 0x30,WHITE);
	return TRUE;
}
/***********************************************************************************************
* Function		: Dis_ShowStopReason
* Description	: 显示停止原因
				 code 故障码 0位正常，不显示          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowStopReason(INT16U add_show ,INT8U code)
{
	char * buf[10];
	ST_Menu* CurMenu = GetCurMenu();
	if(CurMenu == NULL) 
	{
		return FALSE;
	}
	memset(buf,0,sizeof(buf));
	if(code > 0)
	{
		snprintf((char *)buf, sizeof(buf), "E%02x",code);
	}

	PrintStr(add_show,(INT8U *)buf,10);   //显示状态
	SetVariColor((add_show & 0xFFC0 ) + 0x30,RED);
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowGunErr
* Description	: 主界面显示枪错误状态 应调用Dis_ShowStatus函数存在bug，故单独写一个函数          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowGunErr(_GUN_NUM gun,_SHOW_NUM show_num,INT16U add_show,_SHOW_COLOR show_color)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	PrintStr(add_show,(INT8U *)ShowStatus[show_num].pStr,ShowStatus[show_num].ShowLen);   //显示状态
	if(gun == GUN_A)
	{
		SetVariColor(0x03b0,show_color);	
	}
	else
	{
		SetVariColor(0x03C0,show_color);
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowStatus
* Description	: 鐣岄潰鏄剧ず鐘舵�?
				 add_show          	鏄剧ず鍦板潃
				 show_num     		鏄剧ず缂栧彿
				 show_color   		鏄剧ず棰滆壊
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowStatus(INT16U add_show,_SHOW_NUM show_num,_SHOW_COLOR show_color)
{

	INT8U i = 0;
	ST_Menu* CurMenu = GetCurMenu();
	if((show_num >= SHOW_MAX) || (CurMenu == NULL) )
	{
		return FALSE;
	}

	for(i = 0;i < SHOW_MAX;i++)
	{
		if(ShowStatus[i].ShowNum == show_num)
		{
			PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //显示状态
			SetVariColor(DIS_SHOW(CurMenu->FrameID),show_color);	 						   		//显示颜色						  
			break;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowStatus
* Description	: 鐣岄潰鏄剧ず鐘舵�?
				 add_show          	鏄剧ず鍦板潃
				 show_num     		鏄剧ず缂栧彿
				 show_color   		鏄剧ず棰滆壊
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_Show(INT16U add_show,_SHOW_NUM show_num)
{

	INT8U i = 0;
	ST_Menu* CurMenu = GetCurMenu();
	if((show_num >= SHOW_MAX) || (CurMenu == NULL) )
	{
		return FALSE;
	}

	for(i = 0;i < SHOW_MAX;i++)
	{
		if(ShowStatus[i].ShowNum == show_num)
		{
			PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //显示状态				  
			break;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowErr
* Description	: 鐣岄潰鏄剧ず鐘舵�?
				 add_show          	鏄剧ず鍦板潃
				 show_num     		鏄剧ず缂栧彿
				 show_color   		鏄剧ず棰滆壊
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowErr(INT16U add_show,_SHOW_NUM show_num,_SHOW_COLOR show_color)
{

	INT8U i = 0;
	ST_Menu* CurMenu = GetCurMenu();
	if((show_num >= SHOW_MAX) || (CurMenu == NULL) )
	{
		return FALSE;
	}

	for(i = 0;i < SHOW_MAX;i++)
	{
		if(ShowStatus[i].ShowNum == show_num)
		{
			//单机和网络地址区分开来
			if(SYSSet.NetState == DISP_NET)
			{
				PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //显示状态
				SetVariColor(add_show  + 0x6000,show_color);	 						   		//显示颜色		
			}
			else 
			{
				PrintStr(add_show +0x400,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //显示状态
				SetVariColor(add_show +0x400  + 0x6000,show_color);	 						   		//显示颜色	
			}
			break;
		}
	}
	return TRUE;
}


/***********************************************************************************************
* Function		: Dis_ShowCopy
* Description	: 鐣岄潰鏄剧ず鐘舵�?
				 add_show          	鏄剧ず鍦板潃
				 show_num     		鏄剧ず缂栧彿
				 show_color   		鏄剧ず棰滆壊
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowCopy(INT8U* pdata,_SHOW_NUM show_num)
{

	INT8U i = 0;
	ST_Menu* CurMenu = GetCurMenu();
	if((show_num >= SHOW_MAX) || (CurMenu == NULL) )
	{
		return FALSE;
	}

	for(i = 0;i < SHOW_MAX;i++)
	{
		if(ShowStatus[i].ShowNum == show_num)
		{
			memcpy(pdata,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);					  
			break;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowDivStatus
* Description	:显示设备状态
				 add_show          	鏄剧ず鍦板潃
				 show_num     		鏄剧ず缂栧彿
				 show_color   		鏄剧ず棰滆壊
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018年7月27日
***********************************************************************************************/
INT8U Dis_ShowDivStatus(INT16U add_show,_SHOW_NUM show_num,_SHOW_COLOR show_color)
{
	
	INT8U i = 0;
	ST_Menu* CurMenu = GetCurMenu();
	if((show_num >= SHOW_MAX) || (CurMenu == NULL) )
	{
		return FALSE;
	}

	for(i = 0;i < SHOW_MAX;i++)
	{
		if(ShowStatus[i].ShowNum == show_num)
		{
			PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //显示状态
			SetVariColor(add_show + 0x1000,show_color);	 						   		//显示颜色						  
			break;
		}
	}
	return TRUE;	
}
/************************(C)COPYRIGHT 2018 汇誉科技*****END OF FILE****************************/
