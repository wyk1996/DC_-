/*****************************************Copyright(C)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: connectbmsinterface.c
* Author			: 叶喜雨
* Date First Issued	: 23/05/2016
* Version			: V0.1
* Description		: BMS接收帧处理，以及接口文件
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp.h"
#include "gbstandard.h"
#include "connectbms.h"
#include "connectbmsframe.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
//以下的内容是BMS发送过来的数据
//BHM报文内容
_BMS_BHM_CONTEXT BMS_BHM_Context[GUN_MAX];
//BRM报文内容
_BMS_BRM_CONTEXT BMS_BRM_Context[GUN_MAX];
//BCP报文内容
_BMS_BCP_CONTEXT BMS_BCP_Context[GUN_MAX];
//BRO报文内容
_BMS_BRO_CONTEXT BMS_BRO_Context[GUN_MAX];
//BCL报文内容
_BMS_BCL_CONTEXT BMS_BCL_Context[GUN_MAX];
//BCS报文内容
_BMS_BCS_CONTEXT BMS_BCS_Context[GUN_MAX];
//BSM报文内容
_BMS_BSM_CONTEXT BMS_BSM_Context[GUN_MAX];
//BST报文内容
_BMS_BST_CONTEXT BMS_BST_Context[GUN_MAX];
//充电结束阶段
//BSD报文内容
_BMS_BSD_CONTEXT BMS_BSD_Context[GUN_MAX];
//错误报文
//BEM报文内容,元素用报文简称填写，用下划线区分，这样有点不符合编程规范，不够统一，但识别性好些
_BMS_BEM_CONTEXT BMS_BEM_Context[GUN_MAX];

//以下的内容是充电机主动要发送的数据
//CML报文内容
_BMS_CML_CONTEXT BMS_CML_Context[GUN_MAX];
//充电机充电状态报文
_BMS_CCS_CONTEXT BMS_CCS_Context[GUN_MAX];
//充电机中止充电报文
_BMS_CST_CONTEXT BMS_CST_Context[GUN_MAX];
//充电机统计报文内容
_BMS_CSD_CONTEXT BMS_CSD_Context[GUN_MAX];
//充电机错误原因
_BMS_CEM_CONTEXT BMS_CEM_Context[GUN_MAX];
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: CanProtocolOperationRTS
* Description	: RTS帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationRTSA(CanInterfaceStruct *controlstrcut)
{
	INT16U	PGN_Temp=0;
    _BMS_RTS_FRAME_INF *pStruct;
    
    if((controlstrcut->DataID) == (GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236))) //RTS帧ID
    {
        if(controlstrcut->len > 8)
            controlstrcut->len = 8;//长度做保护
        //有效值拷贝到管理结构体
        pStruct = (_BMS_RTS_FRAME_INF *)&controlstrcut->pdata[0];
		//if (pStruct->messagepagenum > 8) //多包大于8包，最多应该是7包
		if (pStruct->messagepagenum > 255) //奇瑞EQ PGN15 多包有0X18包
		{
			return FALSE;
		}
        BMS_RTS_FrameInf[GUN_A] = *pStruct; //把接收到的RTS保存到BMS_RTS_FrameInf
        Reply_BMSFrame_CTSA();//发送BMS过来的RTS帧的回复帧，然后进入多页接收过程
		
		PGN_Temp=((INT16U)((INT16U)(BMS_RTS_FrameInf[GUN_A].paranumber[1]<<8)) | (BMS_RTS_FrameInf[GUN_A].paranumber[0]));
		
		if((PGN_Temp == GB_BMV_PGN_5376)||(PGN_Temp == GB_BMT_PGN_5632)||(PGN_Temp == GB_BSP_PGN_5888))
		{//特殊处理BMV\BMT\BSP
			Special_PGN[GUN_A][BMS_RTS_FrameInf[GUN_A].paranumber[1]-0x15].Get_Flag = 0x01;
			Special_PGN[GUN_A][BMS_RTS_FrameInf[GUN_A].paranumber[1]-0x15].Count_Delay = 0x00;
		}
//        BMS_CONNECT_StateSet(BMS_RTS_SUCCESS);//设置状态为接收RTS成功
        return TRUE;
    }
    return FALSE;
}

/***********************************************************************************************
* Function		: CanProtocolOperationRTS
* Description	: RTS帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationRTSB(CanInterfaceStruct *controlstrcut)
{
	INT16U	PGN_Temp=0;
    _BMS_RTS_FRAME_INF *pStruct;
    
    if((controlstrcut->DataID) == (GB_CONFIGRECEIVE_ID(GB_PRIO_7,GB_RTS_PGN_236))) //RTS帧ID
    {
        if(controlstrcut->len > 8)
            controlstrcut->len = 8;//长度做保护
        //有效值拷贝到管理结构体
        pStruct = (_BMS_RTS_FRAME_INF *)&controlstrcut->pdata[0];
		//if (pStruct->messagepagenum > 8) //多包大于8包，最多应该是7包
		if (pStruct->messagepagenum > 255) //奇瑞EQ PGN15 多包有0X18包
		{
			return FALSE;
		}
        BMS_RTS_FrameInf[GUN_B] = *pStruct; //把接收到的RTS保存到BMS_RTS_FrameInf
        Reply_BMSFrame_CTSB();//发送BMS过来的RTS帧的回复帧，然后进入多页接收过程
		
		PGN_Temp=((INT16U)((INT16U)(BMS_RTS_FrameInf[GUN_B].paranumber[1]<<8)) | (BMS_RTS_FrameInf[GUN_B].paranumber[0]));
		
		if((PGN_Temp == GB_BMV_PGN_5376)||(PGN_Temp == GB_BMT_PGN_5632)||(PGN_Temp == GB_BSP_PGN_5888))
		{//特殊处理BMV\BMT\BSP
			Special_PGN[GUN_B][BMS_RTS_FrameInf[GUN_B].paranumber[1]-0x15].Get_Flag = 0x01;
			Special_PGN[GUN_B][BMS_RTS_FrameInf[GUN_B].paranumber[1]-0x15].Count_Delay = 0x00;
		}
//        BMS_CONNECT_StateSet(BMS_RTS_SUCCESS);//设置状态为接收RTS成功
        return TRUE;
    }
    return FALSE;
}

/***********************************************************************************************
* Function		: CanProtocolOperationBHM
* Description	: BHM帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBHMA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BHM_CONTEXT *pStruct;
    
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BHM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BHM_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BHM_SUCCESS);//设置状态为接收BHM成功
    return TRUE;
 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBHM
* Description	: BHM帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBHMB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BHM_CONTEXT *pStruct;
    
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BHM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BHM_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BHM_SUCCESS);//设置状态为接收BHM成功
    return TRUE;
 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBRM
* Description	: 帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBRMA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(_BMS_BRM_CONTEXT)) 
        controlstrcut->len = sizeof(_BMS_BRM_CONTEXT);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BRM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRM_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BRM_SUCCESS);//设置状态为成功
    return TRUE;
}

/***********************************************************************************************
* Function		: CanProtocolOperationBRM
* Description	: 帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBRMB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(_BMS_BRM_CONTEXT)) 
        controlstrcut->len = sizeof(_BMS_BRM_CONTEXT);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BRM_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRM_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BRM_SUCCESS);//设置状态为成功
    return TRUE;
  
}
/***********************************************************************************************
* Function		: CanProtocolOperationBCP
* Description	: 帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBCPA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCP_CONTEXT *pStruct;
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BCP_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCP_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BCP_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCP
* Description	: 帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBCPB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCP_CONTEXT *pStruct;
    
    if(controlstrcut->len > 8)
        controlstrcut->len = 8;//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BCP_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCP_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BCP_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBRO
* Description	: 帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBROA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRO_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BRO_Context))
        controlstrcut->len = sizeof(BMS_BRO_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BRO_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRO_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BRO_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBRO
* Description	: 帧接收处理
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBROB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BRO_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BRO_Context))
        controlstrcut->len = sizeof(BMS_BRO_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BRO_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BRO_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BRO_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCL
* Description	: 电池充电需求报文BCL
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBCLA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCL_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BCL_Context))
        controlstrcut->len = sizeof(BMS_BCL_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BCL_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCL_Context[GUN_A] = *pStruct;
	
	
			//20211118
//	if(BMS_BCL_Context[GUN_A].DemandVol != 5500)
//	{
//		printf("GUN_A BCL  ERR,%d",BMS_BCL_Context[GUN_A].DemandVol);
//	}
	
	
    BMS_CONNECT_StateSet(GUN_A,BMS_BCL_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCL
* Description	: 电池充电需求报文BCL
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBCLB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BCL_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BCL_Context))
        controlstrcut->len = sizeof(BMS_BCL_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BCL_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BCL_Context[GUN_B] = *pStruct;
	
	
	
	
		//20211118
//	if(BMS_BCL_Context[GUN_B].DemandVol != 3000)
//	{
//		printf("GUN_B BCL  ERR,%d",BMS_BCL_Context[GUN_B].DemandVol);
//	}
//	
	
	
    BMS_CONNECT_StateSet(GUN_B,BMS_BCL_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBCS
* Description	: 电池充电总状态BCS
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBCSA(CanInterfaceStruct *controlstrcut)
{
	_BMS_BCS_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BCS_Context))
		controlstrcut->len = sizeof(BMS_BCS_Context);//长度做保护
	//有效值拷贝到管理结构体
	pStruct = (_BMS_BCS_CONTEXT *)&controlstrcut->pdata[0];
	
	//BMS_BCS_Context[GUN_A] = *pStruct;
	memcpy(&BMS_BCS_Context[GUN_A],controlstrcut->pdata,sizeof(_BMS_BCS_CONTEXT));
	

	//20211118
//	if(BMS_BCS_Context[GUN_A].SOC != 50)
//	{
//		printf("GUN_A BCS  ERR,%d",BMS_BCS_Context[GUN_A].SOC);
//	}
	
	
	
	BMS_CONNECT_StateSet(GUN_A,BMS_BCS_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBCS
* Description	: 电池充电总状态BCS
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBCSB(CanInterfaceStruct *controlstrcut)
{
	_BMS_BCS_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BCS_Context))
		controlstrcut->len = sizeof(BMS_BCS_Context);//长度做保护
	//有效值拷贝到管理结构体
	pStruct = (_BMS_BCS_CONTEXT *)&controlstrcut->pdata[0];
	
	//BMS_BCS_Context[GUN_B] = *pStruct;
	memcpy(&BMS_BCS_Context[GUN_B],controlstrcut->pdata,sizeof(_BMS_BCS_CONTEXT));
	
	
	//20211118
//	if(BMS_BCS_Context[GUN_B].SOC != 80)
//	{
//		printf("GUN_B BCS  ERR,%d",BMS_BCS_Context[GUN_B].SOC);
//	}
	
	
	
	BMS_CONNECT_StateSet(GUN_B,BMS_BCS_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBSM
* Description	: 电池充电总状态BSM
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSMA(CanInterfaceStruct *controlstrcut)
{
	_BMS_BSM_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BSM_Context))
		controlstrcut->len = sizeof(BMS_BSM_Context);//长度做保护
	//有效值拷贝到管理结构体
	pStruct = (_BMS_BSM_CONTEXT *)&controlstrcut->pdata[0];
	BMS_BSM_Context[GUN_A] = *pStruct;
	BMS_CONNECT_StateSet(GUN_A,BMS_BSM_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBSM
* Description	: 电池充电总状态BSM
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSMB(CanInterfaceStruct *controlstrcut)
{
	_BMS_BSM_CONTEXT *pStruct;

	if(controlstrcut->len > sizeof(BMS_BSM_Context))
		controlstrcut->len = sizeof(BMS_BSM_Context);//长度做保护
	//有效值拷贝到管理结构体
	pStruct = (_BMS_BSM_CONTEXT *)&controlstrcut->pdata[0];
	BMS_BSM_Context[GUN_B] = *pStruct;
	BMS_CONNECT_StateSet(GUN_B,BMS_BSM_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBMV
* Description	: 单体动力蓄电池电压
* Input			: 
* Return		: 
* Note(s)		: 单体动力蓄电池电压，只做流程逻辑控制，不解析具体内容，只走流程
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBMVA(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMV_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//长度做保护
//    //有效值拷贝到管理结构体
//    pStruct = (_BMS_BMV_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMV_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BMV_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBMV
* Description	: 单体动力蓄电池电压
* Input			: 
* Return		: 
* Note(s)		: 单体动力蓄电池电压，只做流程逻辑控制，不解析具体内容，只走流程
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBMVB(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMV_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//长度做保护
//    //有效值拷贝到管理结构体
//    pStruct = (_BMS_BMV_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMV_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BMV_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBMT
* Description	: 动力蓄电池温度
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBMTA(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMT_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//长度做保护
//    //有效值拷贝到管理结构体
//    pStruct = (_BMS_BMT_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMT_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BMT_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBMT
* Description	: 动力蓄电池温度
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBMTB(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BMT_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//长度做保护
//    //有效值拷贝到管理结构体
//    pStruct = (_BMS_BMT_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BMT_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BMT_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBSP
* Description	: 动力蓄电池预留报文
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSPA(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BSP_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//长度做保护
//    //有效值拷贝到管理结构体
//    pStruct = (_BMS_BSP_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BSP_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BSP_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBSP
* Description	: 动力蓄电池预留报文
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSPB(CanInterfaceStruct *controlstrcut)
{
//    _BMS_BSP_CONTEXT *pStruct;
//    
//    if(controlstrcut->len > 8)
//        controlstrcut->len = 8;//长度做保护
//    //有效值拷贝到管理结构体
//    pStruct = (_BMS_BSP_CONTEXT *)&controlstrcut->pdata[0];
//    BMS_BSP_Context = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BSP_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBST
* Description	: BMS中止充电
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSTA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BST_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BST_Context))
        controlstrcut->len = sizeof(BMS_BST_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BST_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BST_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BST_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBST
* Description	: BMS中止充电
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSTB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BST_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BST_Context))
        controlstrcut->len = sizeof(BMS_BST_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BST_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BST_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BST_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBSD
* Description	: BMS统计数据
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSDA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BSD_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BSD_Context))
        controlstrcut->len = sizeof(BMS_BSD_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BSD_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BSD_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BSD_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBSD
* Description	: BMS统计数据
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBSDB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BSD_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BSD_Context))
        controlstrcut->len = sizeof(BMS_BSD_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BSD_CONTEXT *)&controlstrcut->pdata[0];
    BMS_BSD_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BSD_SUCCESS);//设置状态为成功
    return TRUE; 
}
/***********************************************************************************************
* Function		: CanProtocolOperationBEM
* Description	: BMS错误报文
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBEMA(CanInterfaceStruct *controlstrcut)
{
    _BMS_BEM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BEM_Context))
        controlstrcut->len = sizeof(BMS_BEM_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BEM_CONTEXT *)(&controlstrcut->pdata[0]);
    BMS_BEM_Context[GUN_A] = *pStruct;
    BMS_CONNECT_StateSet(GUN_A,BMS_BEM_SUCCESS);//设置状态为成功
    return TRUE; 
}

/***********************************************************************************************
* Function		: CanProtocolOperationBEM
* Description	: BMS错误报文
* Input			: 
* Return		: 
* Note(s)		: 
* Contributor	: 160523	叶喜雨
***********************************************************************************************/
INT8U CanProtocolOperationBEMB(CanInterfaceStruct *controlstrcut)
{
    _BMS_BEM_CONTEXT *pStruct;
    
    if(controlstrcut->len > sizeof(BMS_BEM_Context))
        controlstrcut->len = sizeof(BMS_BEM_Context);//长度做保护
    //有效值拷贝到管理结构体
    pStruct = (_BMS_BEM_CONTEXT *)(&controlstrcut->pdata[0]);
    BMS_BEM_Context[GUN_B] = *pStruct;
    BMS_CONNECT_StateSet(GUN_B,BMS_BEM_SUCCESS);//设置状态为成功
    return TRUE; 
}
/************************(C)COPYRIGHT 2016 汇誉科技*****END OF FILE****************************/
