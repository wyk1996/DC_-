/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: MyList.h
* Author			:
* Date First Issued	:
* Version			:
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
#ifndef __MY_LIST_H_
#define __MY_LIST_H_

/* Includes-----------------------------------------------------------------------------------*/
#include "sysconfig.h"
/* Private define-----------------------------------------------------------------------------*/
//计算一个结构体里的member成员相对结构体开始地址的偏移值
//#define OFFSETOF(type, member) ((INT32U) &((type *)0)->member)

//初始化一个链表头，指向name本身
#define MY_LIST_HEAD_INIT(name) { &(name), &(name) }

//定义并初始化一个链表头，都指向name本身
#define MY_LIST_HEAD(name)  struct list_head name = MY_LIST_HEAD_INIT(name)

//初始化一个链表头，指向ptr本身
#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

//从一个结构体的成员地址找到结构体的起始地址
//ptr   --- 结构体成员的地址
//type  --- 结构体类型
//member--- 结构体成员名字
#define CONTAINER_OF(ptr, type, member) ( (type *)( (INT32U)(ptr) - OFFSETOF(type,member) ) )

//遍历整个链表
#define LIST_FOR_EACH(pos, head)  for ( (pos) = (head)->next; (pos) != (head); (pos) = (pos)->next)

//安全的遍历整个链表
#define LIST_FOR_EACH_SAFE(pos, n, head) \
	for ( (pos) = (head)->next, (n) = (pos)->next; (pos) != (head); (pos) = (n), (n) = (pos)->next) 

//得到链表的起始地址
//ptr - 指向结构 list_head 的指针
//type - 结构体类型
//member - 在结构体内类型为list_head 的变量的名字
#define MY_LIST_ENTRY(ptr, type, member) CONTAINER_OF(ptr, type, member)

//链表是否为空,head为链表头地址
#define MY_LIST_EMPTY(head) ( ( ( (head)->next) == (head) ) && ( ( (head)->prev) == (head) ) )
/* Private typedef----------------------------------------------------------------------------*/
typedef struct list_head 
{
    struct list_head *next;
    struct list_head *prev;
}_LIST_HEAD;

//链表写入方式
typedef enum 
{
    LIST_MODE_END,       //链表从最后面插入
    LIST_MODE_FRONT,     //链表从最前面插入
}LIST_MODE;
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : MyListAdd
* Description  : 往链表中添加一个链表项
* Input        : struct list_head *new   ：要加入的链表项
                 struct list_head *head  ：要添加到的链表头
* Output       : None
* Return       : TRUE  --- 添加成功
                 FALSE --- 添加失败
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U MyListAdd(struct list_head* const pNew, struct list_head *pHead, const LIST_MODE Mode);

/*****************************************************************************
* Function     : MyListDel
* Description  : 从链表中删除一个链表项
* Input        : struct list_head *entry  ：要删除的链表项
* Output       : None
* Return       : TRUE  --- 删除成功
                 FALSE --- 删除失败
* Note(s)      : 
* Contributor  : 2018年7月25日
*****************************************************************************/
INT8U MyListDel(struct list_head *pEntry);
#endif //__MY_LIST_H_

