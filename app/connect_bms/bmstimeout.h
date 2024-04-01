/*****************************************Copyright(H)******************************************
*******************************************汇誉科技*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: bmstimeout.h
* Author			  :      
* Date First Issued	: 10/6/2015
* Version			  : V0.1
* Description		: 大部分驱动都可以在这里初始化
*----------------------------------------历史版本信息-------------------------------------------
* History			  :
* //2010		    : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef	__BMS_TIMEOUT_H_
#define	__BMS_TIMEOUT_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "gbstandard.h"
/* Private define-----------------------------------------------------------------------------*/       
#define RX_BMS_TIMEOUT_ENTER    0      //超时判断开始进入
#define RX_BMS_TIMEOUT_CHECK    1      //超时判断监测

/* Private typedef----------------------------------------------------------------------------*/


// typedef struct
// {
//     _BMS_STEP step;
//     INT32U timeoutval;
// }_TIMEOUT_MANAGE_TABLE;

// _TIMEOUT_MANAGE_TABLE  Timeout_Manage_Table[]=
// {
//     {BMS_RTS_STEP, 
//     {BMS_BHM_STEP,
//     {BMS_BRM_STEP,
//     {BMS_BCP_STEP,
//     {BMS_BRO_STEP,
//     {BMS_BCL_STEP,
//     {BMS_BCS_STEP,
//     {BMS_BSM_STEP,
//     {BMS_BMV_STEP,
//     {BMS_BMT_STEP,
//     {BMS_BSP_STEP,
//     {BMS_BST_STEP,
//     {BMS_BSD_STEP,
//     {BMS_BEM_STEP, 
// };



_BMS_STEP BMS_TIMEOUT_ENTER(INT8U gun,_BMS_STEP step,INT32U time);
_BMS_STEP BMS_TIMEOUT_CHECK(INT8U gun,_BMS_STEP step,INT32U time);
#endif //__BMS_TIMEOUT_H_
/************************(H)COPYRIGHT 2010 汇誉科技*****END OF FILE****************************/
