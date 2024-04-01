/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : FmDataDeal.h
* Author            : 
* Date First Issued : 
* Version           : 
* Description       :写铁电地址
*----------------------------------------历史版本信息-------------------------------------------
* History           :
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __FMDATA_DEAL_H_
#define __FMDATA_DEAL_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
#define FM_BASEADDR_RECODE	      		100u         //充电记录基地址
/* Private define-----------------------------------------------------------------------------*/  
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	INT16U DataID;       // 数据标识
	INT16U Addr;         // 地址
	INT16U DataLen;      // 数据长度
}_FM_MANAGE_TABLE;


typedef enum 
{
    FM_ORDER_READ = 0x01, //0x01:读操作
    FM_ORDER_WRITE,       //0x02:写操作
}_FM_ORDER;

//数据传输控制接口体
typedef struct
{
    INT16U DataID;       //数据ID
    INT16U Len;          //数据长度
    INT8U  *ptr;         //具体数据缓冲区
    _FM_ORDER  RWChoose; //读写功能选择
}_FM_OPERATION;

//以下是充电记录

//A枪当前时间
#define FM_0100_ID			          (100u)						//ID查表使用			
#define FM_0100_FLADDR                (FM_BASEADDR_RECODE) 		//配置信息基地址
#define FM_0100_FLLEN                 (7)            			//字节数
//A枪充电记录
#define FM_0101_ID			          (101u)						//ID查表使用			
#define FM_0101_FLADDR                (FM_0100_FLADDR + FM_0100_FLLEN) 		//配置信息基地址
#define FM_0101_FLLEN                 (83)            					//交易记录长度

//B枪当前时间
#define FM_0102_ID			          (102u)						//ID查表使用			
#define FM_0102_FLADDR                (FM_0101_FLADDR + FM_0101_FLLEN) 	 		//配置信息基地址
#define FM_0102_FLLEN                 (7)            			//字节数
//B枪充电记录
#define FM_0103_ID			          (103u)						//ID查表使用			
#define FM_0103_FLADDR                (FM_0102_FLADDR + FM_0102_FLLEN) 		//配置信息基地址
#define FM_0103_FLLEN                 (83)            					//交易记录长度
/*****************************************************************************
* Function     : APP_FMOperation
* Description  : FM读写操作
* Input        : _FM_OPERATION *pOps  
* Output       : None
* Return       : 返回读写成功的字节数
* Note(s)      : 
* Contributor  : 2018年7月30日        
*****************************************************************************/
INT16U APP_FMOperation(_FM_OPERATION *pOps);

#endif  //__BSP_ADC_H_
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
