/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: MenuDisp.c
* Author			: 
* Date First Issued	:    
* Version			: 
* Description		: 提供每个界面的菜单结构体，被DispKey.c调用
*----------------------------------------历史版本信息-------------------------------------------
* History			: 
* //2013	        : V
* Description		: 
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "MenuDisp.h"
#include "DispKeyFunction.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private macro------------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
//主菜单子菜单
/****************************************汇誉屏幕*******************************************/

ST_Menu HYMenu0 = 
{
    NULL,                                            //用于记录上一级菜�?
    0,                                              //页面ID
    HYKeyEvent0,                                      //当前界面对键值的处理函数
    HYDisplayMenu0,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu1 = 
{
    NULL,                                            //用于记录上一级菜�?
    1,                                              //页面ID
    HYKeyEvent1,                                      //当前界面对键值的处理函数
    HYDisplayMenu1,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//主界�?
ST_Menu HYMenu2 = 
{
    NULL,                                            //用于记录上一级菜�?
    2,                                              //页面ID
    HYKeyEvent2,                                      //当前界面对键值的处理函数
    HYDisplayMenu2,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//主界�?
ST_Menu HYMenu3 = 
{
    NULL,                                            //用于记录上一级菜�?
    3,                                              //页面ID
    HYKeyEvent3,                                      //当前界面对键值的处理函数
    HYDisplayMenu3,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//费率详情
ST_Menu HYMenu4 = 
{
    NULL,                                            //用于记录上一级菜�?
    4,                                              //页面ID
    HYKeyEvent4,                                      //当前界面对键值的处理函数
    HYDisplayMenu4,                                   //当前界面变量显示函数
    10,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//密码输入界面
ST_Menu HYMenu5 = 
{
    NULL,                                            //用于记录上一级菜�?
    5,                                              //页面ID
    HYKeyEvent5,                                      //当前界面对键值的处理函数
    HYDisplayMenu5,                                   //当前界面变量显示函数
    60,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//电卡信息界面
ST_Menu HYMenu6 = 
{
    NULL,                                            //用于记录上一级菜�?
    6,                                              //页面ID
    HYKeyEvent6,                                      //当前界面对键值的处理函数
    HYDisplayMenu6,                                   //当前界面变量显示函数
    15,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//无充电记录界�?
ST_Menu HYMenu7 = 
{
    NULL,                                            //用于记录上一级菜�?
    7,                                              //页面ID
    HYKeyEvent7,                                      //当前界面对键值的处理函数
    HYDisplayMenu7,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//记录信息
ST_Menu HYMenu8 = 
{
    NULL,                                            //用于记录上一级菜�?
    8,                                              //页面ID
    HYKeyEvent8,                                      //当前界面对键值的处理函数
    HYDisplayMenu8,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//记录信息
ST_Menu HYMenu9 = 
{
    NULL,                                            //用于记录上一级菜�?
    9,                                              //页面ID
    HYKeyEvent9,                                      //当前界面对键值的处理函数
    HYDisplayMenu9,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//故障信息
ST_Menu HYMenu10 = 
{
    NULL,                                            //用于记录上一级菜�?
    10,                                              //页面ID
    HYKeyEvent10,                                      //当前界面对键值的处理函数
    HYDisplayMenu10,                                   //当前界面变量显示函数
	0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//故障信息
ST_Menu HYMenu11 = 
{
    NULL,                                            //用于记录上一级菜�?
    11,                                              //页面ID
    HYKeyEvent11,                                      //当前界面对键值的处理函数
    HYDisplayMenu11,                                   //当前界面变量显示函数
    15,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//无记录信�?
ST_Menu HYMenu12 = 
{
    NULL,                                            //用于记录上一级菜�?
    12,                                              //页面ID
    HYKeyEvent12,                                      //当前界面对键值的处理函数
    HYDisplayMenu12,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};


//无故障记�?
ST_Menu HYMenu13 = 
{
    NULL,                                            //用于记录上一级菜�?
    13,                                              //页面ID
    HYKeyEvent13,                                      //当前界面对键值的处理函数
    HYDisplayMenu13,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//充电枪未连接提示界面
ST_Menu HYMenu14 = 
{
    NULL,                                            //用于记录上一级菜�?
    14,                                              //页面ID
    HYKeyEvent14,                                      //当前界面对键值的处理函数
    HYDisplayMenu14,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//故障界面
ST_Menu HYMenu15 = 
{
    NULL,                                            //用于记录上一级菜�?
    15,                                              //页面ID
    HYKeyEvent15,                                      //当前界面对键值的处理函数
    HYDisplayMenu15,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//充电方式选择
ST_Menu HYMenu16 = 
{
    NULL,                                            //用于记录上一级菜�?
    16,                                              //页面ID
    HYKeyEvent16,                                      //当前界面对键值的处理函数
    HYDisplayMenu16,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//刷卡方式选择
ST_Menu HYMenu17 = 
{
    NULL,                                            //用于记录上一级菜�?
    17,                                              //页面ID
    HYKeyEvent17,                                      //当前界面对键值的处理函数
    HYDisplayMenu17,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//充电时间选择
ST_Menu HYMenu18 = 
{
    NULL,                                            //用于记录上一级菜�?
    18,                                              //页面ID
    HYKeyEvent18,                                      //当前界面对键值的处理函数
    HYDisplayMenu18,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//充电电量选择
ST_Menu HYMenu19 = 
{
    NULL,                                            //用于记录上一级菜�?
    19,                                              //页面ID
    HYKeyEvent19,                                      //当前界面对键值的处理函数
    HYDisplayMenu19,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//充电金额选择
ST_Menu HYMenu20 = 
{
    NULL,                                            //用于记录上一级菜�?
    20,                                              //页面ID
    HYKeyEvent20,                                      //当前界面对键值的处理函数
    HYDisplayMenu20,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//电卡密码输入
ST_Menu HYMenu21 = 
{
    NULL,                                            //用于记录上一级菜�?
    21,                                              //页面ID
    HYKeyEvent21,                                      //当前界面对键值的处理函数
    HYDisplayMenu21,                                   //当前界面变量显示函数
    60,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//刷卡界面
ST_Menu HYMenu22 = 
{
    NULL,                                            //用于记录上一级菜�?
    22,                                              //页面ID
    HYKeyEvent22,                                      //当前界面对键值的处理函数
    HYDisplayMenu22,                                   //当前界面变量显示函数
    20,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//管理员密码输�?
ST_Menu HYMenu23 = 
{
    NULL,                                            //用于记录上一级菜�?
    23,                                              //页面ID
    HYKeyEvent23,                                      //当前界面对键值的处理函数
    HYDisplayMenu23,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};


//密码错误提示
ST_Menu HYMenu24 = 
{
    NULL,                                            //用于记录上一级菜�?
    24,                                              //页面ID
    HYKeyEvent24,                                      //当前界面对键值的处理函数
    HYDisplayMenu24,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//卡内余额不足
ST_Menu HYMenu25 = 
{
    NULL,                                            //用于记录上一级菜�?
    25,                                              //页面ID
    HYKeyEvent25,                                      //当前界面对键值的处理函数
    HYDisplayMenu25,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//卡被锁住
ST_Menu HYMenu26 = 
{
    NULL,                                            //用于记录上一级菜�?
    26,                                              //页面ID
    HYKeyEvent26,                                      //当前界面对键值的处理函数
    HYDisplayMenu26,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//无法识别此卡
ST_Menu HYMenu27 = 
{
    NULL,                                            //用于记录上一级菜�?
    27,                                              //页面ID
    HYKeyEvent27,                                      //当前界面对键值的处理函数
    HYDisplayMenu27,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//读卡超时
ST_Menu HYMenu28 = 
{
    NULL,                                            //用于记录上一级菜�?
    28,                                              //页面ID
    HYKeyEvent28,                                      //当前界面对键值的处理函数
    HYDisplayMenu28,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//非发行方�?
ST_Menu HYMenu29 = 
{
    NULL,                                            //用于记录上一级菜�?
    29,                                              //页面ID
    HYKeyEvent29,                                      //当前界面对键值的处理函数
    HYDisplayMenu29,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//A枪正在启�?
ST_Menu HYMenu30 = 
{
    NULL,                                            //用于记录上一级菜�?
    30,                                              //页面ID
    HYKeyEvent30,                                      //当前界面对键值的处理函数
    HYDisplayMenu30,                                   //当前界面变量显示函数
    90,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//B枪正在启�?
ST_Menu HYMenu31 = 
{
    NULL,                                            //用于记录上一级菜�?
    31,                                              //页面ID
    HYKeyEvent31,                                      //当前界面对键值的处理函数
    HYDisplayMenu31,                                   //当前界面变量显示函数
    90,												//倒计时时�?单位�?0表示不需要倒计�?   
};


//启动失败刷卡结算
ST_Menu HYMenu32 = 
{
    NULL,                                            //用于记录上一级菜�?
    32,                                              //页面ID
    HYKeyEvent32,                                      //当前界面对键值的处理函数
    HYDisplayMenu32,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//刷卡结算界面
ST_Menu HYMenu33 = 
{
    NULL,                                            //用于记录上一级菜�?
    33,                                              //页面ID
    HYKeyEvent33,                                      //当前界面对键值的处理函数
    HYDisplayMenu33,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//卡号不一�?
ST_Menu HYMenu34 = 
{
    NULL,                                            //用于记录上一级菜�?
    34,                                              //页面ID
    HYKeyEvent34,                                      //当前界面对键值的处理函数
    HYDisplayMenu34,                                   //当前界面变量显示函数
    5,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//充电枪停止中
ST_Menu HYMenu35 = 
{
    NULL,                                            //用于记录上一级菜�?
    35,                                              //页面ID
    HYKeyEvent35,                                      //当前界面对键值的处理函数
    HYDisplayMenu35,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//A枪充电界�?
ST_Menu HYMenu36 = 
{
    NULL,                                            //用于记录上一级菜�?
    36,                                              //页面ID
    HYKeyEvent36,                                      //当前界面对键值的处理函数
    HYDisplayMenu36,                                   //当前界面变量显示函数
    10,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//B枪充电界�?
ST_Menu HYMenu37 = 
{
    NULL,                                            //用于记录上一级菜�?
    37,                                              //页面ID
    HYKeyEvent37,                                      //当前界面对键值的处理函数
    HYDisplayMenu37,                                   //当前界面变量显示函数
    10,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//A枪信�?
ST_Menu HYMenu38 = 
{
    NULL,                                            //用于记录上一级菜�?
    38,                                              //页面ID
    HYKeyEvent38,                                      //当前界面对键值的处理函数
    HYDisplayMenu38,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//B枪信�?
ST_Menu HYMenu39 = 
{
    NULL,                                            //用于记录上一级菜�?
    39,                                              //页面ID
    HYKeyEvent39,                                      //当前界面对键值的处理函数
    HYDisplayMenu39,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//A枪结算界�?
ST_Menu HYMenu40 = 
{
    NULL,                                            //用于记录上一级菜�?
    40,                                              //页面ID
    HYKeyEvent40,                                      //当前界面对键值的处理函数
    HYDisplayMenu40,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//B枪结算界�?
ST_Menu HYMenu41 = 
{
    NULL,                                            //用于记录上一级菜�?
    41,                                              //页面ID
    HYKeyEvent41,                                      //当前界面对键值的处理函数
    HYDisplayMenu41,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//设备信息1
ST_Menu HYMenu42 = 
{
    NULL,                                            //用于记录上一级菜�?
    42,                                              //页面ID
    HYKeyEvent42,                                      //当前界面对键值的处理函数
    HYDisplayMenu42,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//设备信息2
ST_Menu HYMenu43 = 
{
    NULL,                                            //用于记录上一级菜�?
    43,                                              //页面ID
    HYKeyEvent43,                                      //当前界面对键值的处理函数
    HYDisplayMenu43,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//设备信息3
ST_Menu HYMenu44 = 
{
    NULL,                                            //用于记录上一级菜�?
    44,                                              //页面ID
    HYKeyEvent44,                                      //当前界面对键值的处理函数
    HYDisplayMenu44,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//设备信息4
ST_Menu HYMenu45 = 
{
    NULL,                                            //用于记录上一级菜�?
    45,                                              //页面ID
    HYKeyEvent45,                                      //当前界面对键值的处理函数
    HYDisplayMenu45,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//用户配置界面
ST_Menu HYMenu46 = 
{
    NULL,                                            //用于记录上一级菜�?
    46,                                              //页面ID
    HYKeyEvent46,                                      //当前界面对键值的处理函数
    HYDisplayMenu46,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//系统配置选择
ST_Menu HYMenu47 = 
{
    NULL,                                            //用于记录上一级菜�?
    47,                                              //页面ID
    HYKeyEvent47,                                      //当前界面对键值的处理函数
    HYDisplayMenu47,                                   //当前界面变量显示函数
    30,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//电价设置
ST_Menu HYMenu48 = 
{
    NULL,                                            //用于记录上一级菜�?
    48,                                              //页面ID
    HYKeyEvent48,                                      //当前界面对键值的处理函数
    HYDisplayMenu48,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//手动模式
ST_Menu HYMenu49 = 
{
    NULL,                                            //用于记录上一级菜�?
    49,                                              //页面ID
    HYKeyEvent49,                                      //当前界面对键值的处理函数
    HYDisplayMenu49,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//手动模式输入
ST_Menu HYMenu50 = 
{
    NULL,                                            //用于记录上一级菜�?
    50,                                              //页面ID
    HYKeyEvent50,                                      //当前界面对键值的处理函数
    HYDisplayMenu50,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//B枪手动模�?
ST_Menu HYMenu51 = 
{
    NULL,                                            //用于记录上一级菜�?
    51,                                              //页面ID
    HYKeyEvent51,                                      //当前界面对键值的处理函数
    HYDisplayMenu51,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu52 = 
{
    NULL,                                            //用于记录上一级菜�?
    52,                                              //页面ID
    HYKeyEvent52,                                      //当前界面对键值的处理函数
    HYDisplayMenu52,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu53 = 
{
    NULL,                                            //用于记录上一级菜�?
    53,                                              //页面ID
    HYKeyEvent53,                                      //当前界面对键值的处理函数
    HYDisplayMenu53,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu54 = 
{
    NULL,                                            //用于记录上一级菜�?
    54,                                              //页面ID
    HYKeyEvent54,                                      //当前界面对键值的处理函数
    HYDisplayMenu54,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu55 = 
{
    NULL,                                            //用于记录上一级菜�?
    55,                                              //页面ID
    HYKeyEvent55,                                      //当前界面对键值的处理函数
    HYDisplayMenu55,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu56 = 
{
    NULL,                                            //用于记录上一级菜�?
    56,                                              //页面ID
    HYKeyEvent56,                                      //当前界面对键值的处理函数
    HYDisplayMenu56,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

//系统设置
ST_Menu HYMenu57 = 
{
    NULL,                                            //用于记录上一级菜�?
    57,                                              //页面ID
    HYKeyEvent57,                                      //当前界面对键值的处理函数
    HYDisplayMenu57,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu58 = 
{
    NULL,                                            //用于记录上一级菜�?
    58,                                              //页面ID
    HYKeyEvent58,                                      //当前界面对键值的处理函数
    HYDisplayMenu58,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu59 = 
{
    NULL,                                            //用于记录上一级菜�?
    59,                                              //页面ID
    HYKeyEvent59,                                      //当前界面对键值的处理函数
    HYDisplayMenu59,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu60 = 
{
    NULL,                                            //用于记录上一级菜�?
    60,                                              //页面ID
    HYKeyEvent60,                                      //当前界面对键值的处理函数
    HYDisplayMenu60,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu61 = 
{
    NULL,                                            //用于记录上一级菜�?
    61,                                              //页面ID
    HYKeyEvent61,                                      //当前界面对键值的处理函数
    HYDisplayMenu61,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu62 = 
{
    NULL,                                            //用于记录上一级菜�?
    62,                                              //页面ID
    HYKeyEvent62,                                      //当前界面对键值的处理函数
    HYDisplayMenu62,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu63 = 
{
    NULL,                                            //用于记录上一级菜�?
    63,                                              //页面ID
    HYKeyEvent63,                                      //当前界面对键值的处理函数
    HYDisplayMenu63,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu64 = 
{
    NULL,                                            //用于记录上一级菜�?
    64,                                              //页面ID
    HYKeyEvent64,                                      //当前界面对键值的处理函数
    HYDisplayMenu64,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu65 = 
{
    NULL,                                            //用于记录上一级菜�?
    65,                                              //页面ID
    HYKeyEvent65,                                      //当前界面对键值的处理函数
    HYDisplayMenu65,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu66 = 
{
    NULL,                                            //用于记录上一级菜�?
    66,                                              //页面ID
    HYKeyEvent66,                                      //当前界面对键值的处理函数
    HYDisplayMenu65,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};


//网络版本界面
ST_Menu HYMenu73 = 
{
    NULL,                                            //用于记录上一级菜�?
    73,                                              //页面ID
    HYKeyEvent73,                                      //当前界面对键值的处理函数
    HYDisplayMenu73,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu74 = 
{
    NULL,                                            //用于记录上一级菜�?
    74,                                              //页面ID
    HYKeyEvent74,                                      //当前界面对键值的处理函数
    HYDisplayMenu74,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu75 = 
{
    NULL,                                            //用于记录上一级菜�?
    75,                                              //页面ID
    HYKeyEvent75,                                      //当前界面对键值的处理函数
    HYDisplayMenu75,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu76 = 
{
    NULL,                                            //用于记录上一级菜�?
    76,                                              //页面ID
    HYKeyEvent76,                                      //当前界面对键值的处理函数
    HYDisplayMenu76,                                   //当前界面变量显示函数
    0,												//倒计时时�?单位�?0表示不需要倒计�?   
};

ST_Menu HYMenu77 = 
{
    NULL,                                            //用于记录上一级菜卿
    77,                                              //页面ID
    HYKeyEvent77,                                      //当前界面对键值的处理函数
    HYDisplayMenu77,                                   //当前界面变量显示函数
    0,												//倒计时时闿单位秿0表示不需要倒计旿   
};

ST_Menu HYMenu78 = 
{
    NULL,                                            //用于记录上一级菜卿
    78,                                              //页面ID
    HYKeyEvent78,                                      //当前界面对键值的处理函数
    HYDisplayMenu78,                                   //当前界面变量显示函数
    0,												//倒计时时闿单位秿0表示不需要倒计旿   
};

ST_Menu HYMenu79 = 
{
    NULL,                                            //用于记录上一级菜卿
    79,                                              //页面ID
    HYKeyEvent79,                                      //当前界面对键值的处理函数
    HYDisplayMenu79,                                   //当前界面变量显示函数
    0,												//倒计时时闿单位秿0表示不需要倒计旿   
};

ST_Menu HYMenu80 = 
{
    NULL,                                            //用于记录上一级菜卿
    80,                                              //页面ID
    HYKeyEvent80,                                      //当前界面对键值的处理函数
    HYDisplayMenu80,                                   //当前界面变量显示函数
    0,												//倒计时时闿单位秿0表示不需要倒计旿   
};

ST_Menu HYMenu81 = 
{
    NULL,                                            //用于记录上一级菜卿
    81,                                              //页面ID
    HYKeyEvent81,                                      //当前界面对键值的处理函数
    HYDisplayMenu81,                                   //当前界面变量显示函数
    0,												//倒计时时闿单位秿0表示不需要倒计旿   
};

ST_Menu HYMenu82 = 
{
    NULL,                                           
    82,                                             
    HYKeyEvent82,                                
    HYDisplayMenu82,                              
    10,											
};

ST_Menu HYMenu83 = 
{
    NULL,                                            //用于记录上一级菜卿
    83,                                              //页面ID
    HYKeyEvent83,                                      //当前界面对键值的处理函数
    HYDisplayMenu83,                                   //当前界面变量显示函数
    10,												//倒计时时闿单位秿0表示不需要倒计旿   
};
/*****************************二级菜单******************************************/
//子菜单，第一页自动测�?
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/************************(C)COPYRIGHT 2018 杭州快电*****END OF FILE****************************/
