/*****************************************Copyright(C)******************************************
*******************************************浙江万马*********************************************
**-----------------------------------------文件信息---------------------------------------------
**文    件    名: CardApp.c
**创    建    人: 
**创  建  日  期: 2015/10/10
**最  新  版  本: V0.1
**描          述: 
**---------------------------------------历史版本信息-------------------------------------------
**修    改    人: 
**日          期: 
**版          本: 
**描          述:
**----------------------------------------------------------------------------------------------
***********************************************************************************************/
#include "string.h" 
#include "stdio.h"
#include "Des.h"
/******************************************以下为8个S盒****************************************/
const unsigned char SBox1[64]=
{
	14,  4, 13,  1,  2,  15, 11,  8,  3,  10,  6,  12,  5,  9,  0,  7, 
	 0, 15,  7,  4,  14, 2,  13,  1,  10, 6 ,  12, 11,  9,  5,  3,  8,	 
	4,  1,  14,  8,  13, 6,  2,  11, 15,  12, 9,   7,  3,  10, 5,  0,
	15, 12, 8,   2,  4,  9,  1,   7,  5,  11,  3,  14, 10, 0,  6,  13 
};
const unsigned char SBox2[64]=
{
	15,  1,   8,  14,  6, 11,   3,  4,  9,  7,  2, 13, 12,  0,  5, 10,
	3,  13,   4,   7, 15,  2,   8, 14, 12,  0,  1, 10,  6,  9, 11,  5,
	0,  14,   7,  11, 10,  4,  13,  1,  5,  8, 12,  6,  9,  3,  2, 15,
	13,  8,  10,   1,  3, 15,   4,  2, 11,  6,  7, 12,  0,  5, 14,  9
};
const unsigned char SBox3[64]=
{
	10,  0,   9,  14,  6,  3,  15,  5,  1, 13, 12,  7, 11,  4,  2,  8, 
	13,  7,   0,   9,  3,  4,   6, 10,  2,  8,  5, 14, 12, 11, 15,  1, 
	13,  6,   4 ,  9,  8, 15,   3,  0, 11,  1,  2, 12,  5, 10, 14,  7, 
	 1, 10,  13,   0,  6,  9,   8,  7,  4, 15, 14,  3, 11,  5,  2, 12
};
const unsigned char SBox4[64]=
{
	 7, 13,  14,   3,  0,  6,   9, 10,  1,  2,  8,  5, 11, 12,  4, 15, 
    13,  8,  11,   5,  6, 15,   0,  3,  4,  7,  2, 12,  1, 10, 14,  9, 
    10,  6,   9,   0, 12, 11,   7, 13, 15,  1,  3, 14,  5,  2,  8,  4, 
     3, 15,   0,   6, 10,  1,  13,  8,  9,  4,  5, 11, 12,  7,  2, 14 
};
const unsigned char SBox5[64]=
{
	 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9, 
    14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6, 
     4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14, 
    11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3 
};
const unsigned char SBox6[64]=
{
	12,  1, 10, 15, 9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11, 
    10, 15,  4,  2, 7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8, 
     9, 14, 15,  5, 2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6, 
     4,  3,  2, 12, 9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13 
};
const unsigned char SBox7[64]=
{
	 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1, 
    13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6, 
     1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2, 
     6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12 
};
const unsigned char SBox8[64]=
{
	13,  2,  8, 4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7, 
     1, 15, 13, 8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2, 
     7, 11,  4, 1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8, 
     2,  1, 14, 7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11
};

/******************************************以下移位表******************************************/
const unsigned char ShiftValue[16]={1, 1, 2, 2, 2, 2, 2, 2, 1, 2, 2, 2, 2, 2, 2, 1 };					 	

	 
/**********************************************************************************************
* Function		: Des
* Description	: - DES加密解密
* Param			  : - Mode   ：0 : 加密，1：解密
*             : - MsgIn  : - 加密时输入的是明文，解密输入为密文(注意一般8字节)
*             : - Key    : - 密码(注意一般8字节)
*             : - MsgOut : - 加密时输出密文 ，解密是为明文(注意一般8字节)
* Output		  : - 0 
* Note(s)		  : 
* Contributor	: 10/10/2014	jjy
***********************************************************************************************/ 
unsigned char Des( unsigned char Mode,unsigned char *MsgIn, unsigned char *Key, unsigned char *MsgOut)
{	
	unsigned char i;
	unsigned char temp1,temp2,temp3,temp4;
	unsigned char C0[4], D0[4] ;
	unsigned char Ki[8] ; 

  // 输入的明文或者密文进行IP变换 	
	for(i = 0 ; i < 8 ; i++)
	{
		MsgOut[0] >>= 1 ;
		MsgOut[1] >>= 1 ;
		MsgOut[2] >>= 1 ;
		MsgOut[3] >>= 1 ;
		MsgOut[4] >>= 1 ;
		MsgOut[5] >>= 1 ;
		MsgOut[6] >>= 1 ;
		MsgOut[7] >>= 1 ;
		
		temp1 = MsgIn[i];
		MsgOut[4] |= (temp1 & 0x80);
		
		temp1 <<= 1;
		MsgOut[0] |= (temp1 & 0x80);
	
		temp1 <<= 1 ;
		MsgOut[5] |= (temp1&0x80);
		
		temp1 <<= 1 ;
		MsgOut[1] |= (temp1&0x80);
		
		temp1 <<= 1 ;
		MsgOut[6] |= (temp1&0x80);
		
		temp1 <<= 1 ;
		MsgOut[2] |= (temp1&0x80);
		
		temp1 <<= 1 ;
		MsgOut[7] |= (temp1&0x80);
		
		temp1 <<= 1;
		MsgOut[3] |= (temp1&0x80);
	}

/*================================================================================*/
  // 输入的 密码进行 PC-1 变换，前28bit 放在C0[]中 ，后28bit 放在D0[]中
	for(i = 0 ; i < 8 ; i++)
	{
		C0[0] >>= 1 ;
		C0[1] >>= 1 ;
		C0[2] >>= 1 ;
		C0[3] >>= 1 ;
		D0[0] >>= 1 ;
		D0[1] >>= 1 ;
		D0[2] >>= 1 ;

		temp1   = Key[i] ;
		C0[0]  |= (temp1 & 0x80);
		
		temp1 <<= 1 ;
		C0[1]  |= (temp1 & 0x80);
		
		temp1 <<= 1 ;
		C0[2]  |= (temp1 & 0x80);
		
		temp1 <<= 1 ;
		C0[3]  |= (temp1 & 0x80);
		
		temp1  <<= 1 ;
		D0[2]   |= (temp1 & 0x80);
		
		temp1  <<= 1 ;
		D0[1]   |= (temp1 & 0x80);
		
		temp1   <<= 1 ;
		D0[0]    |= (temp1 & 0x80) ;		
	}

	D0[3]  = (C0[3] & 0x0f) | (D0[2] << 4) ;
	D0[2]  = (D0[2] >> 4) | (D0[1] << 4) ;
	D0[1]  = (D0[1] >> 4) | (D0[0] << 4) ;
	D0[0]  = (D0[0] >> 4) & 0x0F ;
	C0[3] &= 0xf0;
  /*================================================================================*/
  // 开始进行16轮加密（或者解密迭代）
	for(i = 0 ; i < 16 ; i++)
	{
		/*===========================================================================*/
		if(Mode == ENCRY)	  // 加密
		{
			for(temp1 = 0 ; temp1 < ShiftValue[i] ; temp1++)   // 经过PC-1变换的密文经过移位
			{
			  // C0 前28位循环左移
				if((C0[0] & Dbit1) != 0)   C0[3] |= Dbit5 ; 
				C0[0] <<= 1 ;
				
				if((C0[1] & Dbit1 ) != 0)  C0[0] |= Dbit8 ; 
				C0[1] <<= 1 ; 
				
				if((C0[2] & Dbit1) != 0)   C0[1] |= Dbit8 ; 
				C0[2] <<= 1 ;
				
				if((C0[3] & Dbit1) != 0)   C0[2]  |= Dbit8 ; 
				C0[3] <<= 1 ;
				
			  // D0 后28位循环左移
				D0[0] <<= 1 ;
				if((D0[1] & Dbit1) != 0)    D0[0] |= Dbit8 ; 
				
				D0[1] <<= 1 ;
				if((D0[2] & Dbit1) != 0)    D0[1] |= Dbit8 ; 
				
				D0[2] <<= 1 ;
				if((D0[3] & Dbit1) != 0)    D0[2] |= Dbit8 ; 
				
				D0[3] <<= 1 ;
				if((D0[0] & Dbit4) !=0)
				{
					 D0[3] |= Dbit8 ;
					 D0[0] &= 0x0f ; //(~bit4) ;
				}					
			}
		}
		//把C[i]D[i]串联起来，变成Ki,共有56bit	
		Ki[0] = C0[0] ; 
		Ki[1] = C0[1] ;  
		Ki[2] = C0[2] ; 
		Ki[3] = C0[3] | D0[0] ; 
		Ki[4] = D0[1] ; 
		Ki[5] = D0[2] ; 
		Ki[6] = D0[3] ; 
		
		//序列变换２(PC-2)，变换后存入Ki中，共有48bit
		temp1 = 0 ;
		temp2 = 0 ;
		temp3 = 0 ;
		
		if((Ki[0] & Dbit1) != 0) temp1 |= Dbit5 ;  //1
		if((Ki[0] & Dbit2) != 0) temp3 |= Dbit8 ;  //2
		if((Ki[0] & Dbit3) != 0) temp1 |= Dbit7 ;  //3
		if((Ki[0] & Dbit4) != 0) temp2 |= Dbit8 ;  //4
		if((Ki[0] & Dbit5) != 0) temp1 |= Dbit6 ;  //5
		if((Ki[0] & Dbit6) != 0) temp2 |= Dbit2 ;  //6
		if((Ki[0] & Dbit7) != 0) temp3 |= Dbit4 ;  //7
		if((Ki[0] & Dbit8) != 0) temp3 |= Dbit2 ;  //8
		                                  
    //if((Ki[1] & bit1) != 0) temp1 |= bit5 ;  // 9
		if((Ki[1] & Dbit2) != 0) temp2 |= Dbit4 ;  //10
		if((Ki[1] & Dbit3) != 0) temp1 |= Dbit3 ;  //11
		if((Ki[1] & Dbit4) != 0) temp2 |= Dbit7 ;  //12
		if((Ki[1] & Dbit5) != 0) temp3 |= Dbit7 ;  //13
		if((Ki[1] & Dbit6) != 0) temp1 |= Dbit1 ;  //14
		if((Ki[1] & Dbit7) != 0) temp2 |= Dbit1 ;  //15
		if((Ki[1] & Dbit8) != 0) temp3 |= Dbit3 ;  //16
	                                      //
		if((Ki[2] & Dbit1) != 0) temp1 |= Dbit2 ;  //17
//		if((Ki[2] & bit2) != 0) temp3 |= bit8 ;  //18
		if((Ki[2] & Dbit3) != 0) temp2 |= Dbit6 ;  //19
		if((Ki[2] & Dbit4) != 0) temp3 |= Dbit6 ;  //20
		if((Ki[2] & Dbit5) != 0) temp2 |= Dbit3 ;  //21
//		if((Ki[2] & bit6) != 0) temp2 |= bit2 ;  //22
		if((Ki[2] & Dbit7) != 0) temp2 |= Dbit5 ;  //23
		if((Ki[2] & Dbit8) != 0) temp1 |= Dbit4;  //24
		
//		if((Ki[3] & bit1) != 0) temp1 |= bit5 ;  //25
		if((Ki[3] & Dbit2) != 0) temp3 |= Dbit1 ;  //26
		if((Ki[3] & Dbit3) != 0) temp3 |= Dbit5 ;  //27
		if((Ki[3] & Dbit4) != 0) temp1 |= Dbit8 ;  //28
    	
		Ki[0] = temp1 ;
		Ki[1] = temp2 ;
		Ki[2] = temp3 ;
	
		temp1 = 0 ;
		temp2 = 0 ;
		temp3 = 0 ;
		
		if((Ki[3] & Dbit5) != 0) temp3 |= Dbit7 ;
		if((Ki[3] & Dbit6) != 0) temp1 |= Dbit7 ;
		if((Ki[3] & Dbit7) != 0) temp1 |= Dbit3 ;
		if((Ki[3] & Dbit8) != 0) temp3 |= Dbit8 ;
		
		if((Ki[4] & Dbit1) != 0) temp2 |= Dbit3 ;
		if((Ki[4] & Dbit2) != 0) temp3 |= Dbit1 ;
//		if((Ki[4] & bit3) != 0) temp1 |= bit7 ;
		if((Ki[4] & Dbit4) != 0) temp3 |= Dbit6 ;
		if((Ki[4] & Dbit5) != 0) temp1 |= Dbit4 ;
//		if((Ki[4] & bit6) != 0) temp2 |= bit2 ;
		if((Ki[4] & Dbit7) != 0) temp2 |= Dbit7 ;
		if((Ki[4] & Dbit8) != 0) temp1 |= Dbit8 ;
	
		if((Ki[5] & Dbit1) != 0) temp1 |= Dbit1 ;
		if((Ki[5] & Dbit2) != 0) temp3 |= Dbit4 ;
//		if((Ki[5] & bit3) != 0) temp1 |= bit7 ;
		if((Ki[5] & Dbit4) != 0) temp2 |= Dbit5 ;
		if((Ki[5] & Dbit5) != 0) temp2 |= Dbit2 ;
		if((Ki[5] & Dbit6) != 0) temp3 |= Dbit3 ;
		if((Ki[5] & Dbit7) != 0) temp1 |= Dbit5 ;
		if((Ki[5] & Dbit8) != 0) temp2 |= Dbit4 ;
		
		if((Ki[6] & Dbit1) != 0) temp2 |= Dbit6 ;
		if((Ki[6] & Dbit2) != 0) temp3 |= Dbit5 ;
		if((Ki[6] & Dbit3) != 0) temp2 |= Dbit1 ;
		if((Ki[6] & Dbit4) != 0) temp1 |= Dbit2 ;
		if((Ki[6] & Dbit5) != 0) temp3 |= Dbit2 ;
//		if((Ki[6] & bit6) != 0) temp2 |= bit2 ;
		if((Ki[6] & Dbit7) != 0) temp1 |= Dbit6 ;
		if((Ki[6] & Dbit8) != 0) temp2 |= Dbit8 ;
		
		Ki[3] = temp1 ;
		Ki[4] = temp2 ;
		Ki[5] = temp3 ;
		
		//此时已计算出Ki
		//把32位R扩展成48位,即把MsgOut[4]、MsgOut[５]、
		//MsgOut[６]、MsgOut[７]扩展,取得然后与Ki异或

	   	/*===========================================================================*/	     
		//1 进行扩展 
		temp1 = ((MsgOut[4] >> 1) & 0x7c) ;
		if((MsgOut[7] & Dbit8) != 0) temp1 |= Dbit1 ;
		if((MsgOut[4] & Dbit4) != 0) temp1 |= Dbit7 ;
		if((MsgOut[4] & Dbit5) != 0) temp1 |= Dbit8 ;  // 	
	 	Ki[0]^=temp1;   //扩展后R与Ki异或
	 	
	 	//2	进行扩展
	 	temp1 = ((MsgOut[4] << 5) & 0xE0) ;
		if((MsgOut[5] & Dbit1) != 0) temp1 |= Dbit4 ;
		if((MsgOut[4] & Dbit8) != 0) temp1 |= Dbit5 ;
		temp1 |= ((MsgOut[5] >> 5) & 0x07) ;	
	 	Ki[1] ^= temp1 ;	 	//扩展后R与Ki异或
	 	
	 	//3	进行扩展
	 	temp1 =((MsgOut[5] << 3) & 0xc0)  | ((MsgOut[5] << 1) & 0x3e) ;	
		if((MsgOut[6] & Dbit1) != 0) temp1 |= Dbit8 ;
	 	Ki[2] ^= temp1 ; //扩展后R与Ki异或
	 	
	 	//4	进行扩展
	 	temp1 = ((MsgOut[6] >> 1) & 0x7c) ;
		if((MsgOut[5] & Dbit8) != 0) temp1 |= Dbit1 ;
		temp1 |= ((MsgOut[6] >> 3) & 0x03) ;	
		Ki[3] ^= temp1 ;
	 	
	 	//5 进行扩展
	 	temp1 =((MsgOut[6] << 5) & 0xE0) ;
		if((MsgOut[7] & Dbit1) != 0) temp1 |= Dbit4 ;
		if((MsgOut[6] & Dbit8) != 0) temp1 |= Dbit5 ;
		temp1 |= ((MsgOut[7] >> 5) & 0x07) ;	
	 	Ki[4] ^= temp1 ;
	 	
	 	//6	进行扩展
	 	temp1 = ((MsgOut[7] << 1) & 0x3E) ;
		if((MsgOut[7] & Dbit4) != 0) temp1 |= (Dbit1) ;
		if((MsgOut[7] & Dbit5) != 0) temp1 |= (Dbit2);
		if((MsgOut[4] & Dbit1) != 0) temp1 |= Dbit8 ;
		Ki[5]^=temp1;  	//扩展后R与Ki异或
		/*===========================================================================*/
	 	/* 开始 计算 S盒输出*/
		temp1 = 0 ;
		
		if((Ki[5] & Dbit8) != 0) temp1 |= Dbit4 ;
		if((Ki[5] & Dbit7) != 0) temp1 |= Dbit8 ;
		if((Ki[5] & Dbit6) != 0) temp1 |= Dbit7 ;
		if((Ki[5] & Dbit5) != 0) temp1 |= Dbit6 ;
		if((Ki[5] & Dbit4) != 0) temp1 |= Dbit5 ;
		if((Ki[5] & Dbit3) != 0) temp1 |= Dbit3 ;
		Ki[7] = SBox8[temp1] ; 	                  

		temp1 = 0 ;
		if((Ki[5] & Dbit2) != 0) temp1 |= Dbit4 ;
		if((Ki[5] & Dbit1) != 0) temp1 |= Dbit8 ;
		if((Ki[4] & Dbit8) != 0) temp1 |= Dbit7 ;
		if((Ki[4] & Dbit7) != 0) temp1 |= Dbit6 ;
		if((Ki[4] & Dbit6) != 0) temp1 |= Dbit5 ;
		if((Ki[4] & Dbit5) != 0) temp1 |= Dbit3 ;
		Ki[6] = SBox7[temp1] ;
 		
		temp1 = 0 ;
		if((Ki[4] & Dbit4) != 0) temp1 |= Dbit4 ;
		if((Ki[4] & Dbit3) != 0) temp1 |= Dbit8 ;
		if((Ki[4] & Dbit2) != 0) temp1 |= Dbit7 ;
		if((Ki[4] & Dbit1) != 0) temp1 |= Dbit6 ;
		if((Ki[3] & Dbit8) != 0) temp1 |= Dbit5 ;
		if((Ki[3] & Dbit7) != 0) temp1 |= Dbit3 ;
		Ki[5] = SBox6[temp1] ;
 		
		temp1=0;
		if((Ki[3] & Dbit6) != 0) temp1 |= Dbit4 ;
		if((Ki[3] & Dbit5) != 0) temp1 |= Dbit8 ;
		if((Ki[3] & Dbit4) != 0) temp1 |= Dbit7 ;
		if((Ki[3] & Dbit3) != 0) temp1 |= Dbit6 ;
		if((Ki[3] & Dbit2) != 0) temp1 |= Dbit5 ;
		if((Ki[3] & Dbit1) != 0) temp1 |= Dbit3 ;
		Ki[4] = SBox5[temp1];
		
		temp1 = 0 ;
		if((Ki[2] & Dbit8) != 0) temp1 |= Dbit4 ;
		if((Ki[2] & Dbit7) != 0) temp1 |= Dbit8 ;
		if((Ki[2] & Dbit6) != 0) temp1 |= Dbit7 ;
		if((Ki[2] & Dbit5) != 0) temp1 |= Dbit6 ;
		if((Ki[2] & Dbit4) != 0) temp1 |= Dbit5 ;
		if((Ki[2] & Dbit3) != 0) temp1 |= Dbit3 ;
		Ki[3] = SBox4[temp1] ;
		
		temp1 = 0 ; 
		if((Ki[2] & Dbit2) != 0) temp1 |= Dbit4 ;
		if((Ki[2] & Dbit1) != 0) temp1 |= Dbit8 ;
		if((Ki[1] & Dbit8) != 0) temp1 |= Dbit7 ;
		if((Ki[1] & Dbit7) != 0) temp1 |= Dbit6 ;
		if((Ki[1] & Dbit6) != 0) temp1 |= Dbit5 ;
		if((Ki[1] & Dbit5) != 0) temp1 |= Dbit3 ;
		Ki[2] = SBox3[temp1] ;
		
		temp1 = 0;
		if((Ki[1] & Dbit4) != 0) temp1 |= Dbit4 ;
		if((Ki[1] & Dbit3) != 0) temp1 |= Dbit8 ;
		if((Ki[1] & Dbit2) != 0) temp1 |= Dbit7 ;
		if((Ki[1] & Dbit1) != 0) temp1 |= Dbit6 ;
		if((Ki[0] & Dbit8) != 0) temp1 |= Dbit5 ;
		if((Ki[0] & Dbit7) != 0) temp1 |= Dbit3 ;
		Ki[1] = SBox2[temp1] ;
		
		temp1 = 0 ;
		if((Ki[0] & Dbit6) != 0) temp1 |= Dbit4 ;
		if((Ki[0] & Dbit5) != 0) temp1 |= Dbit8 ;
		if((Ki[0] & Dbit4) != 0) temp1 |= Dbit7 ;
		if((Ki[0] & Dbit3) != 0) temp1 |= Dbit6 ;
		if((Ki[0] & Dbit2) != 0) temp1 |= Dbit5 ;
		if((Ki[0] & Dbit1) != 0) temp1 |= Dbit3 ;
		Ki[0] = SBox1[temp1] ;
		/*===========================================================================*/
		
		/*===========================================================================*/
		//把上面的结果进行P置换
		temp1 = 0 ;
		temp2 = 0 ;
		temp3 = 0 ;
		temp4 = 0 ; 
		
		if(Ki[0] & Dbit5)        temp2 |= Dbit1 ; //1
		if(Ki[0] & Dbit6)        temp3 |= Dbit1 ; //2
		if(Ki[0] & Dbit7)        temp3 |= Dbit7 ; //3
		if(Ki[0] & Dbit8)        temp4 |= Dbit7 ; //4

		if(Ki[1] & Dbit5)        temp2 |= Dbit5 ; //5
		if(Ki[1] & Dbit6)        temp4 |= Dbit4 ; //6
		if(Ki[1] & Dbit7)        temp1 |= Dbit2 ; //7
		if(Ki[1] & Dbit8)        temp3 |= Dbit2 ; //8
		 
		if(Ki[2] & Dbit5)        temp3 |= Dbit8 ; //9
		if(Ki[2] & Dbit6)        temp2 |= Dbit8 ; //10
		if(Ki[2] & Dbit7)        temp4 |= Dbit6 ; //11
		if(Ki[2] & Dbit8)        temp1 |= Dbit6 ; //12
	
		if(Ki[3] & Dbit5)        temp4 |= Dbit2 ; //13
		if(Ki[3] & Dbit6)        temp3 |= Dbit4 ; //14
		if(Ki[3] & Dbit7)        temp2 |= Dbit2 ; //15
		if(Ki[3] & Dbit8)        temp1 |= Dbit1 ; //16

		if(Ki[4] & Dbit5)        temp1 |= Dbit8 ; //17
		if(Ki[4] & Dbit6)        temp2 |= Dbit6 ; //18
		if(Ki[4] & Dbit7)        temp4 |= Dbit1;  //19
		if(Ki[4] & Dbit8)        temp1 |= Dbit3;  //20

		if(Ki[5] & Dbit5)        temp1 |= Dbit4 ; //21
		if(Ki[5] & Dbit6)        temp4 |= Dbit5 ; //22	
		if(Ki[5] & Dbit7)        temp2 |= Dbit3 ; //23
		if(Ki[5] & Dbit8)        temp3 |= Dbit3 ; //24
		    
		if(Ki[6] & Dbit5)        temp4 |= Dbit8 ; //25
		if(Ki[6] & Dbit6)        temp2 |= Dbit4 ; //26
		if(Ki[6] & Dbit7)        temp3 |= Dbit6 ; //27
		if(Ki[6] & Dbit8)        temp1 |= Dbit7 ; //28

		if(Ki[7] & Dbit5)        temp1 |= Dbit5 ; //29
		if(Ki[7] & Dbit6)        temp4 |= Dbit3 ; //30
		if(Ki[7] & Dbit7)        temp2 |= Dbit7 ; //31
		if(Ki[7] & Dbit8)        temp3 |= Dbit5 ; //32
		
		/*===========================================================================*/	     
		Ki[0] = MsgOut[4] ;      //
		Ki[1] = MsgOut[5] ;      //
		Ki[2] = MsgOut[6] ;      //
		Ki[3] = MsgOut[7] ;      //
			
		//计算新的R	
		MsgOut[4] = temp1 ^ MsgOut[0] ;	
		MsgOut[5] = temp2 ^ MsgOut[1] ;	
		MsgOut[6] = temp3 ^ MsgOut[2] ;	
		MsgOut[7] = temp4 ^ MsgOut[3] ;
		//用上一次的R作为这一次的L
		
		MsgOut[0] = Ki[0] ;
		MsgOut[1] = Ki[1] ;
		MsgOut[2] = Ki[2] ;
		MsgOut[3] = Ki[3] ;
		
		
	  /*===========================================================================*/	
		if(Mode != ENCRY)     // 解密 
		{
		 	for(temp1 = 0 ; temp1 < ShiftValue[15-i] ; temp1++)
			{
				C0[3] >>= 1 ;
				if((C0[2] & Dbit8) != 0)  C0[3] |= Dbit1 ;
				
				C0[2] >>= 1 ;
				if((C0[1] & Dbit8) != 0)  C0[2] |= Dbit1 ;
				
				C0[1] >>= 1 ;
				if((C0[0] & Dbit8) != 0)  C0[1] |= Dbit1 ;
				
				C0[0] >>= 1 ;
				if((C0[3] & Dbit5) != 0)
				{
					C0[3] &= 0xF0 ; //(~bit5);
					C0[0] |= Dbit1 ; 
				}
				
				if((D0[3] & Dbit8) != 0) D0[0] |= Dbit4 ;
				D0[3] >>= 1 ;
				
				if((D0[2] & Dbit8) != 0) D0[3] |= Dbit1 ;
				D0[2] >>= 1 ;
				
				if((D0[1] & Dbit8) != 0) D0[2] |= Dbit1 ;
				D0[1] >>= 1 ;
				
				if((D0[0] & Dbit8) != 0) D0[1] |= Dbit1 ;
				D0[0] >>= 1 ;
			}
		}		
	}
		
	//对R[16]L[16]进行置换，Final Permutation (IP**-1)
	Ki[0] = MsgOut[0];
	Ki[1] = MsgOut[4];
	Ki[2] = MsgOut[1];
	Ki[3] = MsgOut[5];
		
	Ki[4] = MsgOut[2];
	Ki[5] = MsgOut[6];
	Ki[6] = MsgOut[3];
	Ki[7] = MsgOut[7];
		
	for(i = 0; i < 8; i++)
	{
		MsgOut[0] <<= 1 ;
		MsgOut[1] <<= 1 ;
		MsgOut[2] <<= 1 ;
		MsgOut[3] <<= 1 ;
		MsgOut[4] <<= 1 ;
		MsgOut[5] <<= 1 ;
		MsgOut[6] <<= 1 ;
		MsgOut[7] <<= 1 ;
		
		MsgOut[0] |= (Ki[i]&0x01);
		Ki[i] >>= 1 ;
		
		MsgOut[1] |= (Ki[i]&0x01);
		Ki[i] >>= 1 ;
		
		MsgOut[2] |= (Ki[i]&0x01);
		Ki[i] >>= 1 ;
		
		MsgOut[3] |= (Ki[i]&0x01);
		Ki[i] >>= 1;
	
		MsgOut[4] |= (Ki[i]&0x01);
		Ki[i] >>= 1 ;
	
		MsgOut[5] |= (Ki[i]&0x01);
		Ki[i] >>= 1 ;
		
		MsgOut[6] |= (Ki[i]&0x01);
		Ki[i] >>= 1 ;
		MsgOut[7] |= (Ki[i]&0x01);
	}
	 return 0 ;
	/*
	40 8 48 16 56 24 64 32 
	39 7 47 15 55 23 63 31 
	38 6 46 14 54 22 62 30 
	37 5 45 13 53 21 61 29 
	36 4 44 12 52 20 60 28 
	35 3 43 11 51 19 59 27 
	34 2 42 10 50 18 58 26 
	33 1 41 9  49 17 57 25 
	*/
}


/**************************************************************************************/
//函 数 名：Tdes
//功    能：对一个8字节的字符串进行3DES加解密计算
//输    入：
//Mode=0  : 加密 ，Mode=1：解密
//MsgIn   ：8字节输入数组，明文（加密）或密文（解密）
//Key     : 16字节密钥数组
//		  		
//输	  出：MsgOut：8字节输出数组，密文（加密）或明文（解密）
//返 回 值：0
/**************************************************************************************/
unsigned char Tdes( unsigned char Mode,unsigned char *MsgIn, unsigned char *Key, unsigned char *MsgOut)
{
	unsigned char des_key_temp[8];
	unsigned char Min[8];
	unsigned char Mout[8];

	if(Mode == ENCRY)//加密
	{
		memcpy(des_key_temp,Key,8);
		memcpy(Min,MsgIn,8);
	
		Des(ENCRY,Min,des_key_temp,Mout);
	
		memcpy(des_key_temp,Key+8,8);
		memcpy(Min,Mout,8);
	
		Des(DECRY,Min,des_key_temp,Mout);
	
		memcpy(des_key_temp,Key,8);
		memcpy(Min,Mout,8);
	
		Des(ENCRY,Min,des_key_temp,Mout);
	}
	
	if(Mode == DECRY)
	{
		memcpy(des_key_temp,Key,8);
		memcpy(Min,MsgIn,8);
	
		Des(DECRY,Min,des_key_temp,Mout);
	
		memcpy(	des_key_temp,Key+8,8);
		memcpy(	Min,Mout,8);
	
		Des(ENCRY,Min,des_key_temp,Mout);
	
		memcpy(des_key_temp,Key,8);
		memcpy(Min,Mout,8);
		
		Des(DECRY,Min,des_key_temp,Mout);
	}
	memcpy(	MsgOut,Mout,8);
   	
	return 0;
}



unsigned char MAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data)
{
	 unsigned char mac_data0[8],mac_data1[8];
   unsigned char len;
   unsigned char i,j;    

	 memcpy(	mac_data0,init_data,8);
							                  
	 len = data_len/8;
	 for (i = 0 ;i< (8*(len+1)-data_len) ; i++)
	 {
		  if (i == 0)
		  {
			   in_data[data_len+i] = 0x80 ;
		  } 
			
		  if (i > 0)
		  {
			   in_data[data_len+i] = 0x00 ;
		  }		
   }

	 data_len = data_len + i ;
				       
	 len = data_len / 8 ;

	 for( j = 0 ;j < len ;j++)
   {
    	for(i = 0 ;i < 8; i++)
		  {
			   mac_data0[i] ^= in_data[i + j * 8] ;
		  }      	
		  
			Des(ENCRY,mac_data0,mac_key,mac_data1);     
	    
			memcpy(mac_data0,mac_data1,8);
	 }
				
 	 memcpy(mac_data,mac_data1,8);
   
	 return 0 ;    
}


unsigned char TripleMAC(unsigned char *init_data,unsigned char *mac_key,unsigned char data_len,unsigned char *in_data,unsigned char *mac_data)
{
	 unsigned char mac_data0[8],mac_data1[8];
   unsigned char len;
   unsigned char i,j;    
	 unsigned char L[8],R[8];

	 memcpy(L,mac_key,8);
	 memcpy(R,mac_key+8,8);
	 memcpy(mac_data0,init_data,8);
                 
	 len = data_len / 8 ;
	 for (i = 0 ;i <	(8 *(len+1)-data_len) ; i++)
	 {
		  if(i == 0)
		  {
			   in_data[data_len+i] = 0x80 ;
		  }	
		  if (i>0)
		  {
			   in_data[data_len+i] = 0x00 ;
		  }		
	 }

	 data_len = data_len + i ;
				       
	 len = data_len / 8 ;
	 for(j = 0 ; j < len ; j++)
   {
    	for(i = 0 ;i < 8 ; i++) 
		  {
			   mac_data0[i] ^= in_data[i + j * 8];
		  }     	
		  Des(ENCRY,mac_data0,L,mac_data1);     
	    memcpy(mac_data0,mac_data1,8);
	 }
					
	 Des(DECRY,mac_data0,R,mac_data1);
   memcpy(mac_data0,mac_data1,8);
	 Des(ENCRY,mac_data0,L,mac_data1);
	
	 memcpy(	mac_data,mac_data1,8);

   return 0 ;    
}
