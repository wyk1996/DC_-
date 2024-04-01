/*****************************************Copyright(C)******************************************
*******************************************Êù≠Â∑ûÂø´Áîµ*********************************************
*------------------------------------------Êñá‰ª∂‰ø°ÊÅØ---------------------------------------------
* FileName			: bsp_MX25.h
* Author			: 
* Date First Issued	:    
* Version			: 
* Description		: 
*----------------------------------------ÂéÜÂè≤ÁâàÊú¨‰ø°ÊÅØ-------------------------------------------
* History			: 
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include <string.h>
#include "sysconfig.h"
#include "bsp_spi.h"
#include "bsp_MX25.h"

/* Private define-----------------------------------------------------------------------------*/
/*** MX25 series command hex code definition ***/
	INT8U dataBuf[SECTOR_SIZE];//
//ID comands
#define    FLASH_CMD_RDID      0x9F    //RDID (Read Identification)
#define    FLASH_CMD_RES       0xAB    //RES (Read Electronic ID)
#define    FLASH_CMD_REMS      0x90    //REMS (Read Electronic & Device ID)
#define    FLASH_CMD_REMS2     0xEF    //REMS2 (Read ID for 2 x I/O mode)
#define    FLASH_CMD_REMS4     0xDF    //REMS4 (Read ID for 4 x I/O mode)
#define    FLASH_CMD_REMS4D    0xCF    //REMS4D (Read ID for 4 x I/O DT mode)

//Register comands
#define    FLASH_CMD_WRSR      0x01    //WRSR (Write Status Register)
#define    FLASH_CMD_RDSR      0x05    //RDSR (Read Status Register)
#define    FLASH_CMD_WRSCUR    0x2F    //WRSCUR (Write Security Register)
#define    FLASH_CMD_RDSCUR    0x2B    //RDSCUR (Read Security Register)
#define    FLASH_CMD_CLSR      0x30    //CLSR (Clear SR Fail Flags)

//READ comands
#define    FLASH_CMD_READ        0x03    //READ (1 x I/O)
#define    FLASH_CMD_2READ       0xBB    //2READ (2 x I/O)
#define    FLASH_CMD_4READ       0xEB    //4READ (4 x I/O)
#define    FLASH_CMD_FASTREAD    0x0B    //FAST READ (Fast read data).
#define    FLASH_CMD_FASTDTRD    0x0D    //FASTDTRD (1 x I/O DT read)
#define    FLASH_CMD_2DTRD       0xBD    //2DTRD (Dual DT read) BD
#define    FLASH_CMD_4DTRD       0xED    //4DTRD (Quad DT read)
#define    FLASH_CMD_RDDMC       0x5A    //DMCRD (Read DMC)

//Program comands
#define    FLASH_CMD_WREN     0x06    //WREN (Write Enable)
#define    FLASH_CMD_WRDI     0x04    //WRDI (Write Disable)
#define    FLASH_CMD_PP       0x02    //PP (page program)
#define    FLASH_CMD_4PP      0x38    //4PP (Quad page program)
#define    FLASH_CMD_CP       0xAD    //CP (Continously program)

//Erase comands
#define    FLASH_CMD_SE       0x20    //SE (Sector Erase)
#define    FLASH_CMD_BE32K    0x52    //BE32K (Block Erase 32kb)
#define    FLASH_CMD_BE       0xD8    //BE (Block Erase)
#define    FLASH_CMD_CE       0x60    //CE (Chip Erase) hex code: 60 or C7

//Mode setting comands
#define    FLASH_CMD_DP       0xB9    //DP (Deep Power Down)
#define    FLASH_CMD_RDP      0xAB    //RDP (Release form Deep Power Down)
#define    FLASH_CMD_ENSO     0xB1    //ENSO (Enter Secured OTP)
#define    FLASH_CMD_EXSO     0xC1    //EXSO  (Exit Secured OTP)
#define    FLASH_CMD_ESRY     0x70    //ESRY (Enable SO to output RY/BY)
#define    FLASH_CMD_DSRY     0x80    //DSRY (Enable SO to output RY/BY)
#define    FLASH_CMD_WPSEL    0x68    //WPSEL (Enable block protect mode)
#define    FLASH_CMD_ENPLM    0x55    //ENPLM( Enter Parallel Mode )
#define    FLASH_CMD_EXPLM    0x45    //EXPLM( Exit Parallel Mode )

//Reset comands

//Security comands
#define    FLASH_CMD_SBLK       0x36    //SBLK (Single Block Lock)
#define    FLASH_CMD_SBULK      0x39    //SBULK (Single Block Unlock)
#define    FLASH_CMD_RDBLOCK    0x3C    //RDBLOCK (Block Protect Read)
#define    FLASH_CMD_GBLK       0x7E    //GBLK (Gang Block Lock)
#define    FLASH_CMD_GBULK      0x98    //GBULK (Gang Block Unlock)

// Flash control register mask define
// status register
#define    FLASH_WIP_MASK         0x01
#define    FLASH_LDSO_MASK        0x02
#define    FLASH_QE_MASK          0x40
// security register
#define    FLASH_OTPLOCK_MASK     0x03
#define    FLASH_4BYTE_MASK       0x04
#define    FLASH_WPSEL_MASK       0x80
// other
#define    BLOCK_PROTECT_MASK     0xff
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
INT8U CMD_RDSR( INT8U *StatusReg);
INT8U CMD_RDSCUR( INT8U *SecurityReg);
/* Private functions--------------------------------------------------------------------------*/
/***********************************************************************************************
* Function		: Send_One_Byte
* Description	: 
* Input			: 
* Output		: ∑¢ÀÕ“ª∏ˆ◊÷Ω⁄SPI ˝æ›
* Note(s)		: 
* Contributor	: 2018Âπ¥7Êúà25Êó• 
***********************************************************************************************/
void Send_One_Byte(SPI_TypeDef * SPIx ,INT8U  data)  
{  
	INT8U SPI_statu = TRUE;	
	SPI_SendRcvByte(SPIx, data, &SPI_statu);
}  
/***********************************************************************************************
* Function		: Get_One_Byte
* Description	: ªÒ»°“ª∏ˆ ˝æ›
* Input			: 
* Output		: ªÒ»°µΩµƒ 8 Œª ˝æ› 
* Note(s)		: 
* Contributor	: 2018Âπ¥7Êúà25Êó•
***********************************************************************************************/
INT8U Get_One_Byte(SPI_TypeDef * SPIx)  
{  
	INT8U SPI_statu = TRUE;
	return SPI_SendRcvByte(SPIx, 0xFF, &SPI_statu);
} 
/***********************************************************************************************
* Function		: Write_Enable
* Description	: –¥ (WEL)  πƒ‹Œª. 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018Âπ¥7Êúà25Êó•
***********************************************************************************************/ 
void Write_Enable(void)  
{  		
	SPI_CSEnable(SPI2); 
    Send_One_Byte(SPI2,FLASH_CMD_WREN);  
	SPI_CSDisable(SPI2);  
}  
/***********************************************************************************************
* Function		: Write_Disable
* Description	:
 –¥Ω˚÷π (WRID) ÷∏¡Ó,–¥ πƒ‹À¯¥Ê (WEL) Œª∏¥Œª 
 *              WELŒª∏¥Œª «”–“‘œ¬º∏÷÷«Èøˆ£∫ 
 *              1.–æ∆¨…œµÁ ± 
 *              2.–¥Ω˚÷π£®WRDI£©µƒ÷∏¡ÓÕÍ≥…∫Û 
 *              3.–¥◊¥Ã¨ºƒ¥Ê∆˜ (WRSR) µƒ÷∏¡ÓÕÍ≥…∫Û 
 *              4.“≥≥Ã–Ú (PP) µƒ÷∏¡ÓÕÍ≥…÷Æ∫Û 
 *              5.…»«¯≤¡≥˝ (SE) µƒ÷∏¡ÓÕÍ≥…÷Æ∫Û 
 *              6.øÈ≤¡≥˝ (BE) µƒ÷∏¡ÓÕÍ≥…÷Æ∫Û 
 *              7.–æ∆¨≤¡≥˝£®CE£©µƒ÷∏¡ÓÕÍ≥…. 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018Âπ¥7Êúà25Êó•
***********************************************************************************************/ 
void Write_Disable(void)  
{  
    INT8U SPI_statu = TRUE;

    SPI_CSEnable(SPI2);//∆¨—°”––ß  
    SPI_SendRcvByte(SPI2, FLASH_CMD_WRDI,&SPI_statu);
    SPI_CSDisable(SPI2);  
} 
/***********************************************************************************************
* Function		: Read_Status_Register
* Description	: 
 *          ∂¡»° flash ◊¥Ã¨ºƒ¥Ê∆˜◊¥Ã¨ RDSR ÷∏¡Ó «∂¡◊¥Ã¨ºƒ¥Ê∆˜°£∂¡◊¥Ã¨ºƒ¥Ê∆˜ 
 *          ø…“‘‘⁄»Œ∫Œ ±∫Ú∂¡»° (º¥ π‘⁄±‡≥Ã/≤¡≥˝/–¥◊¥Ã¨ºƒ¥Ê∆˜µƒÃıº˛),Ω®“È‘⁄∑¢ÀÕ“ª 
 *          ∏ˆ÷∏¡Ó÷Æ«∞,»Á ±‡≥Ã/≤¡≥˝/–¥◊¥Ã¨ºƒ¥Ê∆˜≤Ÿ◊˜ ±,ºÏ≤È“ªœ¬ (WIP) Œª. 
 *              ◊¥Ã¨ºƒ¥Ê∆˜Œª∂®“Â»Áœ¬£∫ 
 *                  1.WIP bit(bit 0):–¥Ω¯––Œª (WIP),’‚ «∏ˆ≤ª»∑∂®µƒŒª,±Ì æ∏√…Ë±∏ « 
 *          ∑Ò¥¶”⁄√¶¬µ◊¥Ã¨(±‡≥Ã/≤¡≥˝/–¥◊¥Ã¨ºƒ¥Ê∆˜≤Ÿ◊˜),µ±’‚∏ˆŒªŒ™ 1  ±,±Ì æµ±«∞¥¶ 
 *          ”⁄√¶¬µ◊¥Ã¨,»Áπ˚’‚∏ˆŒªŒ™ 0 ,±Ì æµ±«∞¥¶”⁄µ»¥˝◊¥Ã¨. 
 * 
 *                  2.WEL bit(bit 1):–¥ πƒ‹À¯¥Ê∆˜£®WEL£©Œª,±Ì æ∏√…Ë±∏ «∑Ò…Ë÷√ƒ⁄≤ø 
 *          –¥ πƒ‹À¯¥Ê,µ± (WEL)Œª…Ë÷√Œ™1£¨’‚“‚Œ∂◊≈ƒ⁄≤ø–¥ πƒ‹À¯¥Ê∆˜÷√£¨∏√◊∞÷√ø…“‘ 
 *          Ω” ‹µƒ±‡≥Ã/≤¡≥˝/–¥◊¥Ã¨ºƒ¥Ê∆˜÷∏¡Ó.µ±WELŒª…Ë÷√Œ™0£¨’‚“‚Œ∂◊≈√ª”–ƒ⁄≤ø–¥ π 
 *          ƒ‹À¯¥Ê∆˜…Ë±∏Ω´≤ªΩ” ‹±‡≥Ã/≤¡≥˝/–¥◊¥Ã¨ºƒ¥Ê∆˜÷∏¡Ó. 
 * 
 *                  3.BP2, BP1, BP0 bits(bit 2,3,4): øÈ±£ª§Œª,∂®“Â±£ª§«¯”Ú 
 * 
 *                  4.SRWD bit(bit 7):◊¥Ã¨ºƒ¥Ê∆˜–¥Ω˚÷π£®SRWD£©Œª, 
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 2018Âπ¥7Êúà25Êó•
***********************************************************************************************/   
INT8U Read_Status_Register(INT8U *status)  
{   
    INT8U spi_com_buf = FLASH_CMD_RDSR ;
    
    Write_Enable();  
    SPI_CSEnable(SPI2);                      // πƒ‹ Flash –æ∆¨  
    //∑¢ÀÕ√¸¡ÓÕ∑
	if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
    //Ω” ’◊÷¥Æ
	if( SPI_BufferReceive1(SPI2,status,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
	SPI_CSDisable(SPI2);                     //πÿ±’ Flash –æ∆¨  
    return TRUE;  
}  
/***********************************************************************************************
* Function		: WaitFlashFree
* Description	: µ»¥˝ Õ∑≈
* Input			: 
* Output		: 
* Note(s)		: 
* Contributor	: 
***********************************************************************************************/ 
INT8U WaitFlashFree(void)  
{   
    INT8U status = 0x00;

	while(1)
	{
		//∂¡»°FLASHµƒ◊¥Ã¨◊÷Ω⁄
		if ( Read_Status_Register(&status) == FALSE )
			return FALSE;
		//ø’œ–ÕÀ≥ˆ
		if( (status & 0x01) == 0)
			break;		
	}
	return TRUE; 	
} 
/*******************************************************************************
* Function Name  : IsFlashBusy
* Description    : 
* Input          :  Check status register WIP bit.
 *                  If  WIP bit = 1: return TRUE ( Busy )
 *                             = 0: return FALSE ( Ready ).
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U IsFlashBusy(void)
{
    INT8U  gDataBuffer;

    CMD_RDSR( &gDataBuffer);
    
    if( (gDataBuffer & FLASH_WIP_MASK)  == FLASH_WIP_MASK )
        return TRUE;
    else
        return FALSE;
}
/*******************************************************************************
* Function Name  : IsFlashQIO
* Description    : 
* Input          : If flash QE bit = 1: return TRUE
 *                                 = 0: return FALSE.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U IsFlashQIO(void)
{
    INT8U  gDataBuffer;
    CMD_RDSR( &gDataBuffer);
    if( (gDataBuffer & FLASH_QE_MASK) == FLASH_QE_MASK )
        return TRUE;
    else
        return FALSE;
}

/*******************************************************************************
* Function Name  : IsFlash4Byte
* Description    : 
* Input          : Check flash address is 3-byte or 4-byte.
 *                 If flash 4BYTE bit = 1: return TRUE
 *                                    = 0: return FALSE.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U IsFlash4Byte( void )
{
    INT8U  gDataBuffer;
    
    CMD_RDSCUR( &gDataBuffer );
    if( (gDataBuffer & FLASH_4BYTE_MASK) == FLASH_4BYTE_MASK )
        return TRUE;
    else
        return FALSE;
}

/*******************************************************************************
* Function Name  : SendFlashAddr
* Description    : 
* Input          : Send flash address with 3-byte or 4-byte mode.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
void SendFlashAddr( INT32U flash_address, INT8U addr_4byte_mode )
{
    /* Check flash is 3-byte or 4-byte mode.
       4-byte mode: Send 4-byte address (A31-A0)
       3-byte mode: Send 3-byte address (A23-A0) */
    if( addr_4byte_mode == TRUE )
    {
        Send_One_Byte(SPI2, flash_address >> 24); // A31-A24
    }
    /* A23-A0 */
    Send_One_Byte(SPI2, flash_address >> 16);
    Send_One_Byte(SPI2, flash_address >> 8);
    Send_One_Byte(SPI2, flash_address);
    
}

/*******************************************************************************
* Function Name  : CMD_RES
* Description    : 
* Input          : The RDID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 2-byte.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_RDID( INT32U *Identification)
{
    INT32U temp;
    INT8U  gDataBuffer[3];
    INT8U spi_com_buf = FLASH_CMD_RDID;
    // Chip select go low to start a flash command
    SPI_CSEnable(SPI2);

    // Send command
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
    //Ω” ’◊÷¥Æ
	if( SPI_BufferReceive1(SPI2,gDataBuffer,3) == FALSE )
	{
		SPI_CSDisable(SPI2);                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
//     Send_One_Byte(SPI2,FLASH_CMD_RDID);

//     // Get manufacturer identification, device identification
//     gDataBuffer[0] = Get_One_Byte(SPI2);
//     gDataBuffer[1] = Get_One_Byte(SPI2);
//     gDataBuffer[2] = Get_One_Byte(SPI2);
    // Chip select go high to end a command
    SPI_CSDisable(SPI2);

    // Store identification
    temp =  gDataBuffer[0];
    temp =  (temp << 8 )  | gDataBuffer[1];
    *Identification =  (temp << 8)  | gDataBuffer[2];

    return TRUE;
}

/*******************************************************************************
* Function Name  : CMD_RES
* Description    : 
* Input          : The RES instruction is to read the Device
*                 electric identification of 1-byte.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_RES( INT8U *ElectricIdentification)
{
    INT8U spi_com_buf = FLASH_CMD_RES;
    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    // Send flash command and insert dummy cycle
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);;                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
    //Ω” ’◊÷¥Æ
	if( SPI_BufferReceive1(SPI2,ElectricIdentification,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	} 
//     Send_One_Byte(SPI2,FLASH_CMD_RES);
//     InsertDummyCycle( 24 );

//     // Get electric identification
//     *ElectricIdentification = Get_One_Byte(SPI2);
    // Chip select go high to end a flash command
       SPI_CSDisable(SPI2);

    return TRUE;
}
/*******************************************************************************
* Function Name  : CMD_RDSR
* Description    : 
* Input          : The RDSR instruction is for reading Status
*                 Register Bits.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_RDSR( INT8U *StatusReg)
{
    INT8U spi_com_buf = FLASH_CMD_RDSR;
    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    // Send command
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);             //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
    //Ω” ’◊÷¥Æ
	if( SPI_BufferReceive1(SPI2,StatusReg,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}  
    // Chip select go high to end a flash command
    SPI_CSDisable(SPI2);      
    
    return TRUE;
}
/*******************************************************************************
* Function Name  : CMD_READ
* Description    : 
* Input          : The RDSCUR instruction is for reading the value of
*                   Security Register bits.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_RDSCUR( INT8U *SecurityReg )
{
    INT8U spi_com_buf = FLASH_CMD_RDSCUR;
    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    //Send command
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                 //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}
    //Ω” ’◊÷¥Æ
	if( SPI_BufferReceive1(SPI2,SecurityReg,1) == FALSE )
	{
		SPI_CSDisable(SPI2);               //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}    
    // Chip select go high to end a flash command
	SPI_CSDisable(SPI2);
    return TRUE;

}

/*******************************************************************************
* Function Name  : CMD_READ
* Description    : 
* Input          : The READ instruction is for reading data out.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_READ( INT32U flash_address, INT8U *target_address, INT32U byte_length )
{
    INT8U  addr_4byte_mode;
    INT8U spi_com_buf = FLASH_CMD_READ;
    // Check flash address
    if( flash_address > MX25L128_SIZE ) 
        return FALSE;


    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte() )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    // Write READ command and address
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                     //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}    
    SendFlashAddr( flash_address, addr_4byte_mode );
     //Ω” ’◊÷¥Æ
	if( SPI_BufferReceive1(SPI2,target_address,byte_length) == FALSE )
	{
		SPI_CSDisable(SPI2);                   //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}  
    // Chip select go high to end a flash command
	SPI_CSDisable(SPI2);    

    return TRUE;
}
/*******************************************************************************
* Function Name  : CMD_WREN
* Description    : 
* Input          : The WREN instruction is for setting
 *                 Write Enable Latch (WEL) bit.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_WREN( void )
{ 
    INT8U spi_com_buf = FLASH_CMD_WREN;
    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    // Write Enable command = 0x06, Setting Write Enable Latch Bit
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);              //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}  
    // Chip select go high to end a flash command
	SPI_CSDisable(SPI2);

    return TRUE;
}
/*******************************************************************************
* Function Name  : CMD_WRDI
* Description    : 
* Input          : The WRDI instruction is to reset
 *                 Write Enable Latch (WEL) bit.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_WRDI( void )
{
    INT8U spi_com_buf = FLASH_CMD_WRDI;
    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    // Write Disable command = 0x04, resets Write Enable Latch Bit
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);                //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}  
	SPI_CSDisable(SPI2);

    return TRUE;
}
/*******************************************************************************
* Function Name  : Pageprogram
* Description    : “≥±‡≥Ã
* Input          : The PP instruction is for programming
 *                 the memory to be "0".
 *                 The device only accept the last 256 byte ( one page ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U Pageprogram( INT32U flash_address, INT8U *source_address, INT32U byte_length)
{
    INT8U  addr_4byte_mode;
    INT8U spi_com_buf = FLASH_CMD_PP;
    // Check flash address
    if( flash_address > MX25L128_SIZE ) 
        return FALSE;

    // Check flash is busy or not
    if( IsFlashBusy() )    
        return FALSE;


    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte() )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    // Write Page Program command
    if( SPI_BufferSend1(SPI2,&spi_com_buf,1) == FALSE )
	{
		SPI_CSDisable(SPI2);    //πÿ±’ Flash –æ∆¨  
		return FALSE;
	}  
    SendFlashAddr( flash_address, addr_4byte_mode );

    // Set a loop to down load whole page data into flash's buffer
    // Note: only last 256 byte will be programmed
    if(SPI_BufferSend1(SPI2,source_address,byte_length) == FALSE)
    {
		SPI_CSDisable(SPI2); 
        return FALSE;
    }
    // Chip select go high to end a flash command
	SPI_CSDisable(SPI2); 
    
    return WaitFlashFree();
    
}
//≤¡≥˝√¸¡Ó
/*******************************************************************************
* Function Name  : CMD_CE
* Description    : 
* Input          : The SE instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1". 
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_SE( INT32U flash_address)
{
    INT8U  addr_4byte_mode;

    // Check flash address
    if( flash_address > MX25L128_SIZE ) 
        return FALSE;

    // Check flash is busy or not
    if( IsFlashBusy() )    
        return FALSE;

    // Check 3-byte or 4-byte mode
    if( IsFlash4Byte() )
        addr_4byte_mode = TRUE;  // 4-byte mode
    else
        addr_4byte_mode = FALSE; // 3-byte mode

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    //Write Sector Erase command = 0x20;
    Send_One_Byte(SPI2,FLASH_CMD_SE);
    SendFlashAddr( flash_address, addr_4byte_mode );

    // Chip select go high to end a flash command
	SPI_CSDisable(SPI2); 
    
    return WaitFlashFree();
}
/*******************************************************************************
* Function Name  : CMD_CE
* Description    : 
* Input          : The CE instruction is for erasing the data
*                  of the whole chip to be "1". 
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U CMD_CE(void)
{
    // Check flash is busy or not
    if( IsFlashBusy() )    
        return FALSE;

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
	SPI_CSEnable(SPI2);

    //Write Chip Erase command = 0x60;
    Send_One_Byte(SPI2,FLASH_CMD_CE);
    // Chip select go high to end a flash command
	SPI_CSDisable(SPI2); 

   return WaitFlashFree();
}

/*******************************************************************************
* Function Name  : BSP_MX25Write
* Description    : MX25–¥Ω”ø⁄∫Ø ˝
* Input          : FlashAddr  -- µÿ÷∑
*                  pdata  -- ˝æ› 
*				   Len    --≥§∂»    
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
void BSP_MX25Read(INT32U addr,INT8U *pdata,INT16U len)
{
	CMD_READ( addr, pdata, len);
}
/*******************************************************************************
* Function Name  : BSP_MX25Write
* Description    : MX25–¥Ω”ø⁄∫Ø ˝
* Input          : FlashAddr  -- µÿ÷∑
*                  pdata  -- ˝æ› 
*				   Len    --≥§∂»    
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
void BSP_MX25Write(INT32U addr,INT8U *pdata,INT16U len)
{
	//FlashStatus  flash_state = {0};
	INT16U sectorNum,pageNum;
    INT16U pagepos,sectorpos; 
	INT16U n;

    if(len == 0 ) 
        return;
	
	sectorNum = addr/SECTOR_SIZE;           //sector number
	sectorpos = addr%SECTOR_SIZE;           //sector addr
	pageNum = sectorpos/PAGE_SIZE ;         //page number
	pagepos = sectorpos%PAGE_SIZE;          //page addr

	if((pagepos+len) > PAGE_SIZE)   
	{

		CMD_READ(sectorNum*SECTOR_SIZE, dataBuf, SECTOR_SIZE );
		CMD_SE(sectorNum*SECTOR_SIZE);
		memcpy(&dataBuf[sectorpos],pdata,(PAGE_SIZE-pagepos));
		sectorpos += (PAGE_SIZE-pagepos);//256 - pagepos 
		len -= (PAGE_SIZE-pagepos); 
		pageNum++;
		if(pageNum > ( SECTOR_PAGE_NUM - 1 ))
		{
			for(n = 0; n < 16; n++)
			{
				Pageprogram( sectorNum*SECTOR_SIZE+n*PAGE_SIZE, &dataBuf[n*PAGE_SIZE], PAGE_SIZE);
			}
			
			pageNum = 0; 
			sectorNum++; 
			sectorpos = 0;      
			CMD_READ( sectorNum*SECTOR_SIZE, dataBuf, SECTOR_SIZE );
			CMD_SE(sectorNum*SECTOR_SIZE);
		}
		while(1)
    	{
    		if(len > PAGE_SIZE ) 
    		{
    			memcpy(&dataBuf[sectorpos],pdata,PAGE_SIZE);            
				sectorpos += PAGE_SIZE;
				len -= PAGE_SIZE;
				pageNum++;
				if(pageNum > ( SECTOR_PAGE_NUM - 1 ))
				{
					for(n = 0; n < 16; n++)
					{
						Pageprogram( sectorNum*SECTOR_SIZE+n*PAGE_SIZE, &dataBuf[n*PAGE_SIZE], PAGE_SIZE);
					}
					pageNum = 0;
					sectorNum++;
					sectorpos = 0;
					CMD_READ( sectorNum*SECTOR_SIZE, dataBuf, SECTOR_SIZE );
					CMD_SE(sectorNum*SECTOR_SIZE);
				}
    		}
    		else
    		{
				memcpy(&dataBuf[sectorpos],pdata,len);
				for(n = 0; n < 16; n++)
				{
					Pageprogram( sectorNum*SECTOR_SIZE+n*PAGE_SIZE, &dataBuf[n*PAGE_SIZE], PAGE_SIZE);
				}
				break;
    		}
    	}
	}
	else
	{

		CMD_READ( sectorNum*SECTOR_SIZE, dataBuf, SECTOR_SIZE );
		CMD_SE(sectorNum*SECTOR_SIZE);
        memcpy(&dataBuf[sectorpos],pdata,len);
		for(n = 0; n < 16; n++)
		{
			Pageprogram( sectorNum*SECTOR_SIZE+n*PAGE_SIZE, &dataBuf[n*PAGE_SIZE], PAGE_SIZE );
		}
	}
}
/*******************************************************************************
* Function Name  : BSP_MX25Init
* Description    : 
* Input          :  
* Output         : None
* Return         : TRUE / FALSE
*******************************************************************************/
INT8U BSP_MX25Init(void)
{
    INT32U flashid = 0;
    
    BSP_SPI2Init();

    CMD_RDID(&flashid);
    if(flashid == 0x00c22018)
        return TRUE;
    return FALSE;    
}
