/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName          : FlashDataDeal.h
* Author            : 
* Date First Issued : 
* Version           : 
* Description       :写flash地址
*----------------------------------------历史版本信息-------------------------------------------
* History           :
* Description       :
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __FLASHDATA_DEAL_H_
#define __FLASHDATA_DEAL_H_
/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
/* Private define-----------------------------------------------------------------------------*/  
#define FLASH_BASEADDR_CFG	      		500u         //配置信息基地址 从flash的1000个字节处开始存储
#define FLASH_BASEADDR_PRIC	  			4*4096u		  //费率信息基地址 从flash的1100个字节处开始存储
#define FLASH_PRESET_VALUE	  			8*4096u		  //费率信息基地址 从flash的1100个字节处开始存储
#define FALSE_BASEADDR_RECODE_LEN		12*4096u		  //记录长度基地址，从flash的1200个字节开始存储
#define FALSE_BASEADDR_RECODE_CHARGE	250000u		  //交易记录长度基地址，从flash的1250个字节开始存储
#define FALSE_BASEADDR_RECODE_ERRWARN   80000u		  //故障告警记录基地址，从falsh的80000u自己开始写
#define FALSE_BASEADDRA_BILL			100000u		  //B枪订单，需要周期性存，方便断电重新联网上报			
#define FALSE_BASEADDRB_BILL			110000u		  //B枪订单，需要周期性存，方便断电重新联网上报		
#define FALSE_BASEADD_CARD_WHITEL		125000u			//卡白名单
#define FALSE_BASEADD_VIN_WHITEL		130000u			//VIN白名单
#define FALSE_BASEADD_OFFLINE_NUM		139000u			//离线交易记录
#define FALSE_BASEADD_OFFLINE_BILL		140000u			//离线交易记录基地址，最多保存50个
#define FALSE_BASEADD_OFFLINEFS_BILL	170000u			//离线交易记录基地址，最多保存50个
#define FALSE_BASEADD_JUST				200000u			//绝缘检测地址


#define RECODE_CHARGE_LEN              	89u			  //交易记录长度
#define RECODE_ERRWARN_LEN            	25u			  //故障/告警记录长度


#define PARATABLELEN                    (12u)           //铁电参数存储长度
/* Private typedef----------------------------------------------------------------------------*/
typedef struct
{
	INT16U DataID;       // 数据标识
	INT32U Addr;         // 地址
	INT16U DataLen;      // 数据长度
}_FLASH_MANAGE_TABLE;


typedef enum 
{
    FLASH_ORDER_READ = 0x01, //0x01:读操作
    FLASH_ORDER_WRITE,       //0x02:写操作
}_FLASH_ORDER;

//数据传输控制接口体
typedef struct
{
    INT16U DataID;       //数据ID
    INT16U Len;          //数据长度
    INT8U  *ptr;         //具体数据缓冲区
    _FLASH_ORDER  RWChoose; //读写功能选择
}_FLASH_OPERATION;

//以下是充电桩配置信息


//对所有配置信息进行操作
#define PARA_CFGALL_ID			        (1030u)										//ID查表使用			
#define PARA_CFGALL_FLADDR        		(FLASH_BASEADDR_CFG) 		
#define PARA_CFGALL_FLLEN         		(200)		//字节数 所有配置信息相加

	  

//以下是费率信息

//对所有费率信息进行操作
#define PARA_PRICALL_ID			        (1130u)										//ID查表使用			
#define PARA_PRICALL_FLADDR        		(FLASH_BASEADDR_PRIC) 		
#define PARA_PRICALL_FLLEN         		(625)		//字节数 所有配置信息相加


#define PARA_PRESETVOL_ID			(1230)
#define PARA_PRESETVOL_FLADDR 	(FLASH_PRESET_VALUE) 		
#define PARA_PRESETVOL_FLLEN   	(30)   //单个长度




//当前交易记录个数，若超过1000，则直接覆盖前面的
#define PARA_1200_ID					(1200u)
#define PARA_1200_FLADDR                (FALSE_BASEADDR_RECODE_LEN) 		
#define PARA_1200_FLLEN                 (4)

//当前故障/告警记录个数，若超过1000，则直接覆盖前面的
#define PARA_1201_ID					(1201u)
#define PARA_1201_FLADDR                (PARA_1200_FLADDR + PARA_1200_FLLEN) 		
#define PARA_1201_FLLEN                 (4)

//留给操作sd卡使用的buf1
#define PARA_1202_ID					(1202u)
#define PARA_1202_FLADDR                (PARA_1201_FLADDR + PARA_1201_FLLEN) 		
#define PARA_1202_FLLEN                 (4)

//留给操作sd卡使用的buf2
#define PARA_1203_ID					(1203u)
#define PARA_1203_FLADDR                (PARA_1202_FLADDR + PARA_1202_FLLEN) 		
#define PARA_1203_FLLEN                 (4)

//交易记录中的卡号（查询账户信息需要卡号对比）
#define PARA_1250_ID					(1250u)
#define PARA_1250_FLADDR          		(FALSE_BASEADDR_RECODE_CHARGE) 		
#define PARA_1250_FLLEN             	(6)

//交易记录
#define PARA_RECODE_ID					(1260u)
#define PARA_RECODE_FLADDR          	(FALSE_BASEADDR_RECODE_CHARGE) 		
#define PARA_RECODE_FLLEN             	(RECODE_CHARGE_LEN)

//故障/告警记录
#define PARA_ERRWARN_ID					(1300u)
#define PARA_ERRWARN_FLADDR          	(FALSE_BASEADDR_RECODE_ERRWARN) 		
#define PARA_ERRWARN_FLLEN             	(RECODE_ERRWARN_LEN)


#define PARA_BILLGUNA_ID				(1500u)
#define PARA_BILLGUNA_FLADDR          	(FALSE_BASEADDRA_BILL) 		
#define PARA_BILLGUNA_FLLEN             	(500)

#define PARA_BILLGUNB_ID				(1501u)
#define PARA_BILLGUNB_FLADDR          	(FALSE_BASEADDRB_BILL) 		
#define PARA_BILLGUNB_FLLEN        		(500)


#define PARA_CARDWHITEL_ID				(1510u)
#define PARA_CARDWHITEL_FLADDR      	(FALSE_BASEADD_CARD_WHITEL) 		
#define PARA_CARDWHITEL_FLLEN   		(3000)

#define PARA_VINWHITEL_ID				(1511u)
#define PARA_VINWHITEL_FLADDR      		(FALSE_BASEADD_VIN_WHITEL) 		
#define PARA_VINWHITEL_FLLEN   			(5000)


#define PARA_OFFLINEBILLNUM_ID			(1512u)
#define PARA_OFFLINEBILLNUM_FLADDR 		(FALSE_BASEADD_OFFLINE_NUM) 		
#define PARA_OFFLINEBILLNUM_FLLEN   	(1)   //单个长度

#define PARA_OFFLINEBILL_ID			(1513u)
#define PARA_OFFLINEBILL_FLADDR 	(FALSE_BASEADD_OFFLINE_BILL) 		
#define PARA_OFFLINEBILL_FLLEN   	(300)   //单个长度


#define PARA_JUST_ID				(1555u)
#define  PARA_JUST_FLADDR 			(FALSE_BASEADD_JUST) 		
#define  PARA_JUST_FLLEN   			(160)   //单个长度


#define PARA_OFFLINEBILLNUM_ID			(1512u)
#define PARA_OFFLINEBILLNUM_FLADDR 		(FALSE_BASEADD_OFFLINE_NUM) 		
#define PARA_OFFLINEBILLNUM_FLLEN   	(1)   //单个长度

#define PARA_OFFLINEBILL_ID			(1513u)
#define PARA_OFFLINEBILL_FLADDR 	(FALSE_BASEADD_OFFLINE_BILL) 		
#define PARA_OFFLINEBILLFS_ID		(1514u)
#define PARA_OFFLINEBILLFS_FLADDR 	(FALSE_BASEADD_OFFLINEFS_BILL) 		
#define PARA_OFFLINEBILL_FLLEN   	(300)   //单个长度

/*****************************************************************************
* Function     : APP_FlashOperation
* Description  : FLASH读写操作
* Input        : _FLASH_OPERATION *pOps  
* Output       : None
* Return       : 返回读写成功的字节数
* Note(s)      : 
* Contributor  : 2018年7月30日        
*****************************************************************************/
INT16U APP_FlashOperation(_FLASH_OPERATION *pOps);

/*****************************************************************************
* Function     : APP_ChargeRecodeFlashOperation
* Description  : 读写交易记录
* Input        :
				count  读写在第几条 1 - 1000条
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_ChargeRecodeFlashOperation(INT16U count,_FLASH_OPERATION *pOps);

/*****************************************************************************
* Function     : APP_ErrWarningFlashOperation
* Description  : 读写故障/告警记录
* Input        :
				cmd = 0 写全部整条故障/告警记录  cmd = 1只写故障消除时间
				count  读写在第几条 1 - 1000条
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_ErrWarningFlashOperation(INT8U cmd,INT16U count,_FLASH_OPERATION *pOps);

/*****************************************************************************
* Function     : APP_ChargeOffLineFlashOperation
* Description  : 读写离线交易激励
* Input        :
				count  读写在第几条 1 - 100条
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年8月24日
*****************************************************************************/
INT8U APP_ChargeOffLineFlashOperation(INT16U count,_FLASH_OPERATION *pOps);


/*****************************************************************************
* Function     : ProtocolFlashOperation
* Description  : flash真正读写函数
* Input        : INT8U offset         
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月30日
*****************************************************************************/
INT8U BootLoadWrite(INT8U* pBuffer,INT32U WriteAddr,INT16U NumByteToWrite);

/*****************************************************************************
* Function     : ProtocolFlashOperation
* Description  : flash真正读写函数
* Input        : INT8U offset         
                 _FM_OPERATION *pOps  
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年7月30日
*****************************************************************************/
INT8U BootLoadRead(INT8U* pBuffer,INT32U WriteAddr,INT16U NumByteToWrite);
#endif  //__BSP_ADC_H_
/************************(C)COPYRIGHT 2018 汇誉科技*****END OF FILE****************************/
