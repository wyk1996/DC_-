/****************************************Copyright (c)****************************************************
**                               HangzhouKuaidianXinnengyuan Co.,LTD.
**                                				EFC 
**
**--------------File Info---------------------------------------------------------------------------------
** File name:           Des.h
** Last modified Date:  
** Last Version:        V1.0
** Descriptions:        
**--------------------------------------------------------------------------------------------------------
** Created by:          ZL
** Created date:        2018.5.24
** Version:             V1.0
** Descriptions:        
**--------------------------------------------------------------------------------------------------------
** Modified by:
** Modified date:
** Version:
** Descriptions:
** Rechecked by:
*********************************************************************************************************/
#ifndef _DES_H_
#define _DES_H_

#define Dbit1    0x80
#define Dbit2    0x40
#define Dbit3    0x20
#define Dbit4    0x10
#define Dbit5    0x08
#define Dbit6    0x04
#define Dbit7    0x02
#define Dbit8    0x01

#define ENCRY	0x0
#define DECRY	0x1 

unsigned char Des( unsigned char Mode,unsigned char *MsgIn, unsigned char *Key, unsigned char *MsgOut) ;
unsigned char Tdes( unsigned char Mode,unsigned char *MsgIn, unsigned char *Key, unsigned char *MsgOut) ;
unsigned char MAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data) ;
unsigned char TripleMAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data) ;
//void MacArithmetic(unsigned char *sMacKey,unsigned char *pInData,int iInLen,unsigned char *pRetData);
#endif

