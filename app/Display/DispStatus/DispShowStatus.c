/*****************************************Copyright(C)******************************************
*******************************************�����Ƽ�*********************************************
*------------------------------------------�ļ���Ϣ---------------------------------------------
* FileName			: DispStatus.c
* Author			:
* Date First Issued	:
* Version			:
* Description		: �ṩϵͳ״̬��Ϣ���ṩ��ʾ����ʾ����ʹ�ã����ı���ʽΪANSI
*----------------------------------------��ʷ�汾��Ϣ-------------------------------------------
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
//显示状态处�?
typedef struct
{
	_SHOW_NUM ShowNum;   //显示状态编�?
	const char* pStr;      //显示地址 
	 INT16U ShowLen;      //显示地址 
}_SHOW_STATUS;
/* Private variables--------------------------------------------------------------------------*/
_SHOW_STATUS ShowStatus[SHOW_MAX] = 
{
	{SHOW_CODE_ERR     ,  "�����������"    	,14}, 
	
	{SHOW_CFG_FAIL     ,  "����ʧ�ܣ�"    		,10}, 
	
	{SHOW_CFG_SUCCESS  ,  "���óɹ���"    		,10}, 
	
	{SHOW_CFG_INPURERR ,  "������������"    	,14}, 
	
	{SHOW_CHARGE_AUTO,    "�Զ�����"			,8 },			  		  
	
	{SHOW_CHARGE_JUN,	  "����"	            ,4 },
	
	{SHOW_CHARGE_LUN,	  "�ֳ�"	            ,4 },
		
	{SHOW_LOCK_SUCCESS,	  "�����ɹ�"	   		,8 },
	
	{SHOW_LOCK_FAIL,	  "����ʧ��"	        ,8 },

	{SHOW_BATTERY_TYPE1,  "Ǧ����"			,8 },
	
	{SHOW_BATTERY_TYPE2,  "������"			,8 },
	
	{SHOW_BATTERY_TYPE3,  "������﮵��"		,12},
	
	{SHOW_BATTERY_TYPE4,  "����﮵��"			,10},
	
	{SHOW_BATTERY_TYPE5,  "����﮵��"			,10},
	
	{SHOW_BATTERY_TYPE6,  "��Ԫ���ϵ��"		,12},
	
	{SHOW_BATTERY_TYPE7,  "�ۺ�������ӵ��"	,14},
	
	{SHOW_BATTERY_TYPE8,  "����﮵��"			,10},
	
	{SHOW_BATTERY_OTHER,  "�������"			,8 },
	
	{SHOW_GUNA_ERR,		  "Aǹ����" 			,8 },
		
	{SHOW_GUNB_ERR,       "Bǹ����" 			,8 },
	
	{SHOW_GUNA_WARNING,	  "Aǹ�澯"				,8 },
	
	{SHOW_GUNB_WARNING,	  "Bǹ�澯"				,8 },
	
	{SHOW_GUN_NOTE,       "        "            ,8 },	

	{SHOW_HAND_EXIT,      "���˳��ֶ�ģʽ��"    ,16},	
	
	{SHOW_DIV_ERR,        "����"				,4 },	
	
	{SHOW_DIV_WARNING,    "�澯"				,4 },
	
	{SHOW_DIV_NORMAL,     "����"				,4 },	

	{SHOW_GUN_HOMING,	  "��λ"				,4 },
		
	{SHOW_GUN_UNHOMING,	  "δ��λ"			    ,6 },
	
	{SHOW_STATE_CONNECT,  "����"				,4 },	
	
	{SHOW_STATE_UNCONNECT,"δ����"				,6 },
	
	{SHOW_NULL,"                              "				,30 },
	
	
	//�������֡ԭ�����������֡��Ӧ����
	{SHOW_STARTEND_SUCCESS,				"�����ɹ�",						8},	
	{SHOW_ENDFAIL_HANDERR,				"��������",						8},	
	{SHOW_ENDFAIL_RECVTIMEOUT,			"ͨ�ų�ʱ",						8},	
	{SHOW_ENDFAIL_EMERGENCY	,			"��ͣ����",						8},	
	{SHOW_ENDFAIL_DOORERR,				"�Ž�����",						8},	
	{SHOW_ENDFAIL_SURGEARRESTER,		"����������",					10},	
	{SHOW_ENDFAIL_SMOKEERR,				"��·������",					10},	
	{SHOW_ENDFAIL_ACSWERR1,				"�����Ӵ�������",				14},	
	{SHOW_ENDFAIL_ACSWERR2,				"�����Ӵ���ճ��",				14},	
	{SHOW_ENDFAIL_ACINERR,				"�����������",					12},	
	
	{SHOW_ENDFAIL_CUPTEMPERATURE,		"���׮����",					10},	
	{SHOW_ENDFAIL_GUNUPTEMPERATURE,		"���ӿڹ���",					12},	
	{SHOW_ENDFAIL_ELECLOCKERR,			"���������",					10},	
	{SHOW_ENDFAIL_INSOLUTIONERR,		"��Ե������",					12},	
	{SHOW_ENDFAIL_BATREVERSE,			"��ط���",						8},	
	{SHOW_ENDFAIL_OUTSWERR1,			"ֱ���Ӵ�������",				14},	
	{SHOW_ENDFAIL_OUTSWERR2,			"ֱ���Ӵ���ճ��",				14},	
	{SHOW_ENDFAIL_OUTSWERR3,			"����۶�������",				14},	
	{SHOW_ENDFAIL_SAMELEVELSW1,			"�����Ӵ�������",				14},	
	{SHOW_ENDFAIL_SAMELEVELSW2,			"�����Ӵ���ճ��",				14},
	
	{SHOW_ENDFAIL_LEAKOUTTIMEOUT,		"й�Ż�·����",					12},
	{SHOW_ENDFAIL_CHARGEMODULEERR,		"ģ�����",						8},	
	{SHOW_ENDFAIL_MODULEERR,			"ģ�����",						8},	
	{SHOW_ENDFAIL_OUTVOLTVORE,			"�����ѹ��ѹ",					12},	
	{SHOW_ENDFAIL_OUTVOLTUNDER,			"�����ѹǷѹ",					12},	
	{SHOW_ENDFAIL_OUTCURROVER,			"�����������",					12},	
	{SHOW_ENDFAIL_SHORTCIRCUIT,			"�����·",						8},	
	{SHOW_ENDFAIL_BATVOLTERR1, 			"��������ѹ�쳣",				16},	                    
	{SHOW_ENDFAIL_BATVOLTERR2,  		"��Ե���ǰ��ش���10V"  ,		21},	
	{SHOW_ENDFAIL_BATVOLTERR3,			"Ԥ�䱨���������ѹ����5%",		24},
	
	{SHOW_ENDFAIL_ADVOLTERR1, 			"�󼶵�ѹС����С���",			20},
	{SHOW_ENDFAIL_ADVOLTERR2,       	"�󼶵�ѹ����������",			20},
	{SHOW_ENDFAIL_ADVOLTERR3,       	"�󼶵�ѹ����BMS������ߵ�ѹ",	27},
	{SHOW_ENDFAIL_BRMTIMEOUT,        	"BRM��ʱ",						7},
	{SHOW_ENDFAIL_BRMDATAERR,      		"BRM�����쳣",					11},
	{SHOW_ENDFAIL_BCPTIMEOUT,       	"BCP��ʱ",						7},
	{SHOW_ENDFAIL_BCPDATAERR,      		"BCP�����쳣",					11},
	{SHOW_ENDFAIL_BROWAITTIMEOUT,   	"BRO00��ʱ",					9},
	{SHOW_ENDFAIL_BRORUNTIMEOUT,     	"BROAA��ʱ",					9},
	{SHOW_ENDFAIL_VIN,     				"VIN����ʧ��",					11},
	{SHOW_ENDFAIL_OTHERERR ,            "��������",					    8},
	{SHOW_ENDFAIL_RECVCCUTIMEOUT,       "����CCU����֡��ʱ",			 17},					
	//ֹͣԭ����ֹͣ���֡��Ӧ����
				
	{SHOW_STOP_ERR_NONE,				"����ֹͣ",						8},
	{SHOW_STOP_TCUNORMAL,              	"�Ʒѵ�Ԫ����ֹͣ",				16},
	{SHOW_STOP_TCUERR,               	"�Ʒѵ�Ԫ�������",           	16},                                                                                                                                            
	{SHOW_STOP_CCUERR,					"�Ʒѵ�Ԫ�жϳ�����������",	26},
	{SHOW_STOP_STARTEDTOUT ,       		"�������״̬ȷ��֡��ʱ",		22},
	{SHOW_STOP_HANDERR,            		"���ǹ����",					10},
	{SHOW_STOP_HEARTTOUT,         		"������ʱ",						8},
	{SHOW_STOP_WAITTOIT ,         		"�������ͣ��ʱ",				14},
	{SHOW_STOP_EMERGENCY,        		"��ͣ",							4},
	{SHOW_STOP_DOORERR,            		"�Ž�",							4},
	{SHOW_STOP_XQERR,                    "��б",							4},
	{SHOW_STOP_SJERR,                    "ˮ��",							4},
	{SHOW_SHOW_STOP_SWAERR,              "ֱ���Ӵ�������",							14},
	{SHOW_STOP_SWBERR,                    "ֱ���Ӵ�������",							14},
	{SHOW_STOP_QJERR,                    "�ŽӽӴ�������",							14},
	{SHOW_STOP_SURGEARRESTER,      		"������",						6},

	{SHOW_STOP_SMOKEERR,              	"�̸�",							4},
	{SHOW_STOP_ACSWERR1,          		"���������·������",			18},
	{SHOW_STOP_ACSWERR2,           		"��������Ӵ�����",			18},
	{SHOW_STOP_ACSWERR3,             	"��������Ӵ���ճ��",			18},
	{SHOW_STOP_ACINERR ,               	"�����������",					12},
	{SHOW_STOP_CUPTEMPERATURE,         	"���׮����",					10},
	{SHOW_STOP_GUNUPTEMPERATURE,       	"���׮�ӿڹ���",				14},
	{SHOW_STOP_ELECLOCKERR,            	"���׮���������",				16},
	{SHOW_STOP_OUTSWERR1,           	"����Ӵ����󶯹���",			18},
	{SHOW_STOP_OUTSWERR2,        		"����Ӵ���ճ��",				14},

	{SHOW_STOP_OUTSWERR3,              	"����۶�������",				14},
	{SHOW_STOP_SAMELEVELSW1,         	"�����Ӵ����󶯹���",			18},
	{SHOW_STOP_SAMELEVELSW2,         	"�����Ӵ�������",				14},
	{SHOW_STOP_LEAKOUTTIMEOUT,      	"й�Ż�·",						8},
	{SHOW_STOP_BMSPOWERERR ,         	"������Դ����",					12},
	{SHOW_STOP_CHARGEMODULEERR ,     	"����ģ�����",				14},
	{SHOW_STOP_OUTVOLTVORE ,       		"�����ѹ�쳣",					12},
	{SHOW_STOP_OUTVOLTUNDER ,         	"���Ƿѹ",						8},
	{SHOW_STOP_OUTCURROVER ,           	"�����������",					12},
	{SHOW_STOP_SHORTCIRCUIT,          	"�����·",						8},

	{SHOW_STOP_BCLTIMTOUT,				"BCL��ʱ",						7},
	{SHOW_STOP_BCSTIMTOUT ,				"BCS��ʱ",						7},
	{SHOW_STOP_BSMTIMTOUT ,				"BSM��ʱ",						7},
	{SHOW_STOP_BSMBATVOLTHIGH ,			"BSM�����ѹ����",				15},
	{SHOW_STOP_BSMBATVOLTLOW ,        	"BSM�����ѹ����",				15},
	{SHOW_STOP_BSMSOCHIGH ,           	"BSMSOC����",					10},
	{SHOW_STOP_BSMSOCLOW ,             	"BSMSOC����",					10},
	{SHOW_STOP_BSMCURRUP ,          	"BSM����",						7},
	{SHOW_STOP_BSMTEMPUP ,          	"BSM����",						7},
	{SHOW_STOP_BSMINSOLUTION ,        	"BSM��Ե",						7},

	{SHOW_STOP_BSMSWERR,            	"BSM����������",				13},
	{SHOW_STOP_BSMNORMAL ,            	"BMS����ֹͣ",					11},
	{SHOW_STOP_BSMERR ,              	"�����쳣ֹͣ",					12},
	{SHOW_STOP_CCUBSMERR ,       		"���׮�ж�BMS����",			17},
	{SHOW_STOP_MATERFAIL ,       		"������",						 8},
	{SHOW_STOP_OTHERERR ,             	"���׮��������",				14},
	
	{SHOW_STOP_BSTINSULATIONERR ,        	"BST��Ե����",				11},
	{SHOW_STOP_BSTSWOVERT ,             	"BST�������������",		17},
	{SHOW_STOP_BSTELOVERT ,             	"BSTԪ������",				11},
	{SHOW_STOP_CERR ,             				"BST���������",			11},
	{SHOW_STOP_BSTBATOVERT ,             	"BST�������¹���",		17},
	{SHOW_STOP_BSTHIGHRLCERR ,             	"BST��ѹ�̵�������",		17},
	{SHOW_STOP_BSTTPTWO ,             		"BST���������",			15},
	{SHOW_STOP_BSTOTHERERR ,             	"BST��������",				11},

	{SHOW_STOP_BALANCE ,             	"����",				8},
	{SHOW_STOP_SOC ,             	"�趨SOC�ﵽ",				11},
	
	//��״̬
	{SHOW_CARD_LOCK ,       		"δ����",							6},
	{SHOW_CARD_UNLOCK ,          	"�ѽ���",							6},
	{SHOW_CARD_PERLLOCK ,          	"�������",							8},
	{SHOW_START_FAIL ,          	"����ʧ��",							8},
	{SHOW_GUN_A ,       			"Aǹ",							3},
	{SHOW_GUN_B ,          			"Bǹ",							3},
	{SHOW_START_APP ,          		"��̨����",							8},
	{SHOW_START_CARD ,          	"ˢ������",							8},
	//ʱ���ʽ��ʾ
	{SHOW_TIME ,          	"--++",							4},
//	SHOW_SYS_MJ,
//	SHOW_SYS_SJ,
//	SHOW_SYS_QX,
//	SHOW_SYS_ADC,
//	SHOW_SYS_BDC,
//	SHOW_SYS_QJ,
	//ϵͳ���ϣ�Ŀǰֻд��ô����������ϸ����Ҫ���
	{SHOW_SYS_EMERGENCY ,          	"��ͣ����",							8},
	{SHOW_SYS_MJ ,          	"�Ž�����",							8},
	{SHOW_SYS_SJ ,          	"ˮ������",							8},
	{SHOW_SYS_QX ,          	"��б����",							8},
	{SHOW_SYS_ADC ,          	"SW_A����",							8},
	{SHOW_SYS_BDC ,          	"SW_B����",							8},
	{SHOW_SYS_QJ ,          	"SW_�Žӹ���",					11},
	{SHOW_SYS_PCUTIMEOUT ,          "PCU����",					7},
	{SHOW_SYS_OTHERFAIL ,          "ϵͳ����",					8},
	{SHOW_SYS_AGAINGUN ,          	"���²�ǹ",					8},
	{SHOW_SYS_CHARGE,				"���ڳ��",					8},		
	{SHOW_SYS_CARD,					"ˢ������",					8},	
	{SHOW_SYS_NET,				"ɨ����",					8},	
	{SHOW_SYS_GUN,				"����ǹ",					8},	
	{SHOW_SYS_START,				"��������",					8},	
	{SHOW_SYS_FAIL,					"����ʧ��",					8},	
	{SHOW_SYS_STOP,					"����ֹͣ",					8},	
	{SHOW_SYS_NULL,					"                 "			,20 },
	{SHOW_NET_DJ,					"����"						,4 },
	{SHOW_NET_WL,					"����"						,4 },
	{SHOW_LOCK_CLOSE,				"����"						,4 },
	{SHOW_LOCK_OPEN,				"����"						,4 },
	{SHOW_MODULE_YL_20KWH,			"20kw�����㹦��"			,14},
	{SHOW_MODULE_YL_20KW,			"20kw�����Ǻ㹦��"			,16 },
	{SHOW_MODULE_SH,				"15kwʢ��"					,8 },
	{SHOW_MODULE_YL_15KWH,			"15kw�����㹦��"			,14 },
	{SHOW_MODULE_YL_15KW,			"20kwʢ��"			,8 },
	{SHOW_MODULE_TH_30KW,			"30kwͨ��"			,8 },
	{SHOW_MODULE_TH_40KW,			"40kwͨ��"			,8 },
	{SHOW_MODULE_YL_30KW,			"30kw����"			,8 },
	{SHOW_MODULE_YL_20KW_1000V,		"20kw����1kv"		,12},
	
	
	{SHOW_XY_HY,					"����"					,4 },
	{SHOW_XY_YKC,					"�ƿ��"				,6 },
	{SHOW_XY_AP,					"����"					,4 },
	{SHOW_XY_XJ,					"С��"					,4 },
	{SHOW_XY_YL1,					"�����"				,5 },
	{SHOW_XY_YL2,					"����"					,4 },
	{SHOW_XY_YL3,					"����"					,4 },
	{SHOW_XY_YL4,					"����"					,5 },
	{SHOW_XY_66,					"66"					,2 },
	{SHOW_XY_JG,					"����"					,4 },
	{SHOW_XY_TT,					"����"					,4 },
	{SHOW_XY_ZSH,					"��ʯ��"				,6 },
};
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Dis_ShowStartErr
* Description	: ��ʾ����ʧ��ԭ��
				 code ������ 0λ����������ʾ          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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
		snprintf((char *)buf, sizeof(buf), "����ʧ�ܣ�������ΪE%02x",code);
	}

	PrintStr(add_show,(INT8U *)buf,30);   //��ʾ״̬
	SetVariColor(0x0030,RED);
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowTime
* Description	: ��ʾʱ��         				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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


	PrintStr(add_show,(INT8U *)buf,30);   //��ʾ״̬
	SetVariColor((add_show & 0xFFC0 ) + 0x30,WHITE);
	return TRUE;
}
/***********************************************************************************************
* Function		: Dis_ShowStopReason
* Description	: ��ʾֹͣԭ��
				 code ������ 0λ����������ʾ          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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

	PrintStr(add_show,(INT8U *)buf,10);   //��ʾ״̬
	SetVariColor((add_show & 0xFFC0 ) + 0x30,RED);
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowGunErr
* Description	: ��������ʾǹ����״̬ Ӧ����Dis_ShowStatus��������bug���ʵ���дһ������          				
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
***********************************************************************************************/
INT8U Dis_ShowGunErr(_GUN_NUM gun,_SHOW_NUM show_num,INT16U add_show,_SHOW_COLOR show_color)
{
	if(gun >= GUN_MAX)
	{
		return FALSE;
	}
	PrintStr(add_show,(INT8U *)ShowStatus[show_num].pStr,ShowStatus[show_num].ShowLen);   //��ʾ״̬
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
* Description	: 界面显示状�?
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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
			PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //��ʾ״̬
			SetVariColor(DIS_SHOW(CurMenu->FrameID),show_color);	 						   		//��ʾ��ɫ						  
			break;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowStatus
* Description	: 界面显示状�?
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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
			PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //��ʾ״̬				  
			break;
		}
	}
	return TRUE;
}

/***********************************************************************************************
* Function		: Dis_ShowErr
* Description	: 界面显示状�?
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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
			//�����������ַ���ֿ���
			if(SYSSet.NetState == DISP_NET)
			{
				PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //��ʾ״̬
				SetVariColor(add_show  + 0x6000,show_color);	 						   		//��ʾ��ɫ		
			}
			else 
			{
				PrintStr(add_show +0x400,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //��ʾ״̬
				SetVariColor(add_show +0x400  + 0x6000,show_color);	 						   		//��ʾ��ɫ	
			}
			break;
		}
	}
	return TRUE;
}


/***********************************************************************************************
* Function		: Dis_ShowCopy
* Description	: 界面显示状�?
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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
* Description	:��ʾ�豸״̬
				 add_show          	显示地址
				 show_num     		显示编号
				 show_color   		显示颜色
* Input			:
* Output		:
* Note(s)		:
* Contributor	:2018��7��27��
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
			PrintStr(add_show,(INT8U *)ShowStatus[i].pStr,ShowStatus[i].ShowLen);   //��ʾ״̬
			SetVariColor(add_show + 0x1000,show_color);	 						   		//��ʾ��ɫ						  
			break;
		}
	}
	return TRUE;	
}
/************************(C)COPYRIGHT 2018 �����Ƽ�*****END OF FILE****************************/
