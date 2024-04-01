/*****************************************Copyright(C)******************************************
*******************************************杭州快电*********************************************
*------------------------------------------文件信息---------------------------------------------
* FileName			: MyList.c
* Author			:
* Date First Issued	:
* Version			:
* Description		:
*----------------------------------------历史版本信息-------------------------------------------
* History			:
* Description		:
*-----------------------------------------------------------------------------------------------
***********************************************************************************************/
/* Includes-----------------------------------------------------------------------------------*/
#include "MyList.h"
/* Private define-----------------------------------------------------------------------------*/
/* Private typedef----------------------------------------------------------------------------*/
/* Private variables--------------------------------------------------------------------------*/
/* Private function prototypes----------------------------------------------------------------*/
/* Private functions--------------------------------------------------------------------------*/
/*****************************************************************************
* Function     : __list_add
* Description  : 把一个链表项加入到链表中(内部使用)
* Input        : struct list_head *new   ：要加入的链表项
                 struct list_head *prev  ：要加入的链表项的前一个链表项
                 struct list_head *next  ：要加入的链表项的后一个链表项
* Output       : None
* Return       : static
* Note(s)      : 
* Contributor  : 2018年9月11日        
*****************************************************************************/
static INT8U __list_add(struct list_head *pNew, struct list_head *Prev, struct list_head *pNext)
{
    if ( (pNew == NULL) || (Prev == NULL) || (pNext == NULL) )
    {
        return FALSE;
    }
    pNext->prev = pNew;
    pNew->next = pNext;
    pNew->prev = Prev;
    Prev->next = pNew;
    return TRUE;
}

/*****************************************************************************
* Function     : MyListAdd
* Description  : 往链表中添加一个链表项
* Input        : struct list_head *new   ：要加入的链表项
                 struct list_head *head  ：要添加到的链表头
* Output       : None
* Return       : TRUE  --- 添加成功
                 FALSE --- 添加失败
* Note(s)      : 
* Contributor  : 2018年7月11日        
*****************************************************************************/
INT8U MyListAdd(struct list_head* const pNew, struct list_head* pHead, const LIST_MODE Mode)
{
    if ( (pNew == NULL) || (pHead == NULL) )
    {
        return FALSE;
    }
    
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif

    OS_ENTER_CRITICAL();
    if (Mode == LIST_MODE_END)         //最尾部插入
    {
        if (__list_add(pNew, pHead->prev, pHead) == FALSE)
        {
            OS_EXIT_CRITICAL();
            return FALSE;
        }
    }
    else if (Mode == LIST_MODE_FRONT)  //最前面插入
    {
        if (__list_add(pNew, pHead, pHead->next) == FALSE)
        {
            OS_EXIT_CRITICAL();
            return FALSE;
        }
        pHead = pNew;                  //调整链表头
    }
    OS_EXIT_CRITICAL();
    return TRUE;
}

/*****************************************************************************
* Function     : __list_del
* Description  : 删除链表中的一个链表项(内部使用)
* Input        : struct list_head * Prev   ：要删除的链表项的前一个链表项
                 struct list_head * pNext  ：要删除的链表项的下一个链表项
* Output       : None
* Return       : TRUE  --- 删除成功
                 FALSE --- 删除失败
* Note(s)      : 
* Contributor  : 2018年7月11日        
*****************************************************************************/
static INT8U __list_del(struct list_head * Prev, struct list_head * pNext)
{
    if ( (Prev == NULL) || (pNext == NULL) )
    {

        return FALSE;
    }
    pNext->prev = Prev;
    Prev->next = pNext;
    return TRUE;
}

/*****************************************************************************
* Function     : MyListDel
* Description  : 从链表中删除一个链表项
* Input        : struct list_head *entry  ：要删除的链表项
* Output       : None
* Return       : TRUE  --- 删除成功
                 FALSE --- 删除失败
* Note(s)      : 
* Contributor  : 2018年7月11日        
*****************************************************************************/
INT8U MyListDel(struct list_head *pEntry)
{
    if (pEntry == NULL) 
    {
        return FALSE;
    }
    
#if OS_CRITICAL_METHOD == 3
    OS_CPU_SR  cpu_sr = 0;
#endif

    OS_ENTER_CRITICAL();
    if (__list_del(pEntry->prev, pEntry->next) == FALSE)
    {
        OS_EXIT_CRITICAL();
        return FALSE;
    }
    OS_EXIT_CRITICAL();
    return TRUE;
}

