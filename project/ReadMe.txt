1、周期性任务需要整合再一起
2、#warnging "20210712"    需要处理       




GetModuleOutPutVolandCur
OSQFlush 关注这个  A B公用的时候不能释放


1、PowerModuleInfo  模块信息需要补齐
2、绝缘检测部分需要处理，到时候放在控制任务处理，不需要单独开辟一个任务
8、APP_CheckPowerModule 模块信息需要对应上
9、FP32  需要处理
10、_SYS_SET  存储需要处理


待解决事项
resbuf  数据越界，临时天这个   MODULERxMessage 把这个改大了 ，后面需要具体分析  必须要找到 ！！！！（将   MODULERxMessage改下）  
电表A 485没有调试通，目前使用了备用的
ChargeErrSwitch  故障需要细分





PcuControl.ChargePortReserved[GUN_A] = 1;  //端口预约


SHOpenCloseOneModule 有问题

盛弘模块起来的时候电压会被拉低下

SHSetVolGears_750V(),SHSetVolGears_500V 这个问题导致,需要优化，当再借用其他模块的时候会出现问题。




20200909   
GetBMSStopOnce  已经在停止  APP_CHARGE_END
SetBMSStartCharge



20220917  通合模块调试（国网协议）
模块地址从0x20开始
Charge_FristOpenGun

0x05 遥控命令帧
0x40 心跳帧
0x03 动态分组F


0x20  0x21 模块上传
Display_ParaInit


2022 09 21
为什么云快充B枪发送数据需要屏蔽
注册成功，需要读取在线记录，需要确认A B枪


20230524
通合模块关闭部分模块关不掉


20230530
增加云快充刷卡协议  需要测试
