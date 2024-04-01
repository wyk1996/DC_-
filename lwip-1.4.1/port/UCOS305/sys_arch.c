/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#define SYS_ARCH_GLOBALS

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"
#include "lwip/stats.h"
//#include "includes.h"
#include "UCOS305/sys_arch.h"
#include "string.h"


/*----------------------------------------------------------------------------*/
/*                      DEFINITIONS                                           */
/*----------------------------------------------------------------------------*/
#define LWIP_ARCH_TICK_PER_MS       1000/OS_CFG_TICK_RATE_HZ


/*----------------------------------------------------------------------------*/
/*                      VARIABLES                                             */
/*----------------------------------------------------------------------------*/
//static OS_MEM StackMem;

__align(4) OS_STK       LwIP_Task_Stk[LWIP_TASK_MAX*LWIP_STK_SIZE];
__align(4) INT8U    LwIP_task_priority_stask[LWIP_TASK_MAX];
OS_TCB        LwIP_task_TCB[LWIP_TASK_MAX];

static  OS_MEM  *pQueueMem, *pStackMem;

const void * const pvNullPointer = (mem_ptr_t*)0xffffffff;


#pragma data_alignment=4   //对齐设置
INT8U     pcQueueMemoryPool[MAX_QUEUES * sizeof(TQ_DESCR)];
#pragma data_alignment=4
OS_STK    LwIP_Task_Stk[LWIP_TASK_MAX*LWIP_STK_SIZE];

INT8U     LwIP_task_priopity_stask[LWIP_TASK_MAX];



/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
//  OS_ERR       ucErr;
//      
//  OSQCreate(mbox,"LWIP quiue", size, &ucErr); 
//  LWIP_ASSERT( "OSQCreate ", ucErr == OS_ERR_NONE );
//  
//  if( ucErr == OS_ERR_NONE){ 
//    return 0; 
//  }
//  return -1;
	    /* prarmeter "size" can be ignored in your implementation. */
    INT8U       ucErr;
    PQ_DESCR    pQDesc;

    pQDesc = OSMemGet( pQueueMem, &ucErr );

    LWIP_ASSERT("OSMemGet ", ucErr == OS_ERR_NONE );
    if( ucErr == OS_ERR_NONE )
    {
        if( size > MAX_QUEUE_ENTRIES )
            size = MAX_QUEUE_ENTRIES;
        pQDesc->pQ = OSQCreate( &(pQDesc->pvQEntries[0]), size );
        OSEventNameSet (pQDesc->pQ, "LWIP quie", &ucErr);
        LWIP_ASSERT( "OSQCreate ", pQDesc->pQ != NULL );
        if( pQDesc->pQ != NULL )
        {
            *mbox = pQDesc;
            return 0;
        }
        else
        {
            ucErr = OSMemPut( pQueueMem, pQDesc );
            *mbox = NULL;
            return ucErr;
        }
    }
    else
    {
        return -1;
    }
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
//	OS_ERR     ucErr;
//	LWIP_ASSERT( "sys_mbox_free ", mbox != SYS_MBOX_NULL );      
//			
//	OSQFlush(mbox,& ucErr);
//	
//	OSQDel(mbox, OS_OPT_DEL_ALWAYS, &ucErr);
//	LWIP_ASSERT( "OSQDel ", ucErr == OS_ERR_NONE );
	
	u8_t     ucErr;
	sys_mbox_t m_box = *mbox;
//	LWIP_ASSERT( "sys_mbox_free ", m_box != SYS_MBOX_NULL );

	OSQFlush( m_box->pQ );

	(void)OSQDel( m_box->pQ, OS_DEL_NO_PEND, &ucErr);
	LWIP_ASSERT( "OSQDel ", ucErr == OS_ERR_NONE );

	ucErr = OSMemPut( pQueueMem, m_box );
	LWIP_ASSERT( "OSMemPut ", ucErr == OS_ERR_NONE );
	*mbox = NULL;
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void sys_mbox_post(sys_mbox_t *mbox, void *data)
{
//	OS_ERR     ucErr;
//  CPU_INT08U  i=0; 
//  if( data == NULL ) data = (void*)&pvNullPointer;
//  /* try 10 times */
//  while(i<10){
//    OSQPost(mbox, data,0,OS_OPT_POST_ALL,&ucErr);
//    if(ucErr == OS_ERR_NONE)
//      break;
//    i++;
//    OSTimeDly(5,OS_OPT_TIME_DLY,&ucErr);
//  }
//  LWIP_ASSERT( "sys_mbox_post error!\n", i !=10 );  
	
	
	   u8_t  i=0;
    sys_mbox_t m_box = *mbox;
    if( data == NULL )
        data = (void*)&pvNullPointer;
    /* try 10 times */
    while((i<10) && ((OSQPost( m_box->pQ, data)) != OS_ERR_NONE))
    {
    	i++;
    	OSTimeDly(10);
    }
    LWIP_ASSERT( "sys_mbox_post error!\n", i !=10 );
}


/*-----------------------------------------------------------------------------------*/
//   Try to post the "msg" to the mailbox.
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
//  OS_ERR     ucErr;
//  if(msg == NULL ) msg = (void*)&pvNullPointer;  
//  OSQPost(mbox, msg,0,OS_OPT_POST_ALL,&ucErr);    
//  if(ucErr != OS_ERR_NONE){
//    return ERR_MEM;
//  }
//  return ERR_OK;
	
		sys_mbox_t m_box = *mbox;
    if(msg == NULL )
        msg = (void*)&pvNullPointer;

    if((OSQPost(m_box->pQ, msg)) != OS_ERR_NONE)
    {
        return ERR_MEM;
    }
    return ERR_OK;
		
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
//  OS_ERR	ucErr;
//  OS_MSG_SIZE   msg_size;
//  CPU_TS        ucos_timeout;  
//  CPU_TS        in_timeout = timeout/LWIP_ARCH_TICK_PER_MS;
//  if(timeout && in_timeout == 0)
//    in_timeout = 1;
//  *msg  = OSQPend (mbox,in_timeout,OS_OPT_PEND_BLOCKING,&msg_size, 
//                          &ucos_timeout,&ucErr);

//  if ( ucErr == OS_ERR_TIMEOUT ) 
//      ucos_timeout = SYS_ARCH_TIMEOUT;  
//  return ucos_timeout; 
	
		u8_t	ucErr;
    u32_t	ucos_timeout, timeout_new;
    void	*temp;
    sys_mbox_t m_box = *mbox;
    /* convert to timetick */
    if(timeout != 0)
    {
        ucos_timeout = (timeout * OS_TICKS_PER_SEC)/1000;
        if(ucos_timeout < 1)  ucos_timeout = 1;
    }
    else
    {
        ucos_timeout = 0;
    }

    timeout = OSTimeGet();
    temp = OSQPend(m_box->pQ, (u16_t)ucos_timeout, &ucErr );
    if(msg != NULL)
    {
        if( temp == (void*)&pvNullPointer )
            *msg = NULL;
        else
            *msg = temp;
    }

    if ( ucErr == OS_ERR_TIMEOUT )
    {
        //DBG_RECV("in sys_arch_mbox_fetch 1: OS_ERR_TIMEOUT\r\n");
        timeout = SYS_ARCH_TIMEOUT;
    }
    else
    {
        LWIP_ASSERT( "OSQPend ", ucErr == OS_ERR_NONE );
        timeout_new = OSTimeGet();
        if (timeout_new>timeout)
            timeout_new = timeout_new - timeout;
        else
            timeout_new = 0xffffffff - timeout + timeout_new;
        /* convert to millisecond */
        timeout = timeout_new * 1000 / OS_TICKS_PER_SEC + 1;
    }
    return timeout;
}


/*----------------------------------------------------------------------------------*/
int sys_mbox_valid(sys_mbox_t *mbox)          
{      
//  if(mbox->NamePtr)  
//    return (strcmp(mbox->NamePtr,"?Q"))? 1:0;
//  else
//    return 0;
	
	sys_mbox_t m_box = *mbox;
  u8_t	ucErr;
  int ret;
  OS_Q_DATA q_data;
  if(m_box == NULL) //必须加该保护，发现启动时有时接收消息为空。这里就死了。
      return 0;
  memset(&q_data,0,sizeof(OS_Q_DATA));
  ucErr = OSQQuery (m_box->pQ,&q_data);
  ret =  ( ucErr <2 && (q_data.OSNMsgs < q_data.OSQSize) )? 1:0;
  return ret;
	
}                                             
/*-----------------------------------------------------------------------------------*/                                              
void sys_mbox_set_invalid(sys_mbox_t *mbox)   
{                                             
//  if(sys_mbox_valid(mbox))
//    sys_mbox_free(mbox);     
	*mbox=NULL;	
}                                             

/*-----------------------------------------------------------------------------------*/
//  Creates a new semaphore. The "count" argument specifies
//  the initial state of the semaphore.
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
//	OS_EVENT	ucErr;
//  sys_sem_t = OSSemCreate (count);
//  if(ucErr != OS_ERR_NONE ){
//    LWIP_ASSERT("OSSemCreate ",ucErr == OS_ERR_NONE );
//    return -1;    
//  }
  u8_t err;

  * sem = OSSemCreate((u16_t)count);
  if(*sem == NULL){
    return -1;
  }

  OSEventNameSet (*sem,"LWIP Sem", &err);
  LWIP_ASSERT("OSSemCreate ",*sem != NULL );
  return 0;
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
//  OS_ERR	ucErr;
//  CPU_TS        ucos_timeout;
//  CPU_TS        in_timeout = timeout/LWIP_ARCH_TICK_PER_MS;
//  if(timeout && in_timeout == 0)
//    in_timeout = 1;  
//  OSSemPend (sem,in_timeout,OS_OPT_PEND_BLOCKING,&ucos_timeout,&ucErr);
//    /*  only when timeout! */
//  if(ucErr == OS_ERR_TIMEOUT)
//      ucos_timeout = SYS_ARCH_TIMEOUT;	
//  return ucos_timeout;
	
		u8_t ucErr;
    u32_t ucos_timeout, timeout_new;

//    DBG_CON("in sys_arch_sem_wait 0:\r\n");

    if(	timeout != 0)
    {
        ucos_timeout = (timeout * OS_TICKS_PER_SEC) / 1000; // convert to timetick
        if(ucos_timeout < 1)    ucos_timeout = 1;
    }
    else
    {
        ucos_timeout = 0;
    }

    timeout = OSTimeGet();

    OSSemPend (*sem,(u16_t)ucos_timeout, (u8_t *)&ucErr);
    /*  only when timeout! */
    if(ucErr == OS_ERR_TIMEOUT)
    {
        timeout = SYS_ARCH_TIMEOUT;
       // DBG_CON("in sys_arch_sem_wait 1: OS_ERR_TIMEOUT \r\n");
    }
    else
    {

        /* for pbuf_free, may be called from an ISR */
        timeout_new = OSTimeGet();
        if (timeout_new>=timeout)
        {
            timeout_new = timeout_new - timeout;
        }
        else
        {
            timeout_new = 0xffffffff - timeout + timeout_new;
        }

        /* convert to milisecond */
        timeout = (timeout_new * 1000 / OS_TICKS_PER_SEC + 1);

//        DBG_NET("in sys_arch_sem_wait 2: \r\n");

    }
    return timeout;
		
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void sys_sem_signal(sys_sem_t *sem)
{
//	OS_ERR	ucErr;  
//  OSSemPost(sem,OS_OPT_POST_ALL,&ucErr);
//  LWIP_ASSERT("OSSemPost ",ucErr == OS_ERR_NONE ); 
	
	OSSemPost(*sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void sys_sem_free(sys_sem_t *sem)
{
//  OS_ERR     ucErr;
//	OSSemDel(sem, OS_OPT_DEL_ALWAYS, &ucErr );
//	LWIP_ASSERT( "OSSemDel ", ucErr == OS_ERR_NONE );
		u8_t     ucErr;
    (void)OSSemDel( *sem, OS_DEL_ALWAYS, &ucErr );
    LWIP_ASSERT( "OSSemDel ", ucErr == OS_ERR_NONE );
    *sem = NULL;
	
}
/*-----------------------------------------------------------------------------------*/
int sys_sem_valid(sys_sem_t *sem)                                               
{
//  if(sem->NamePtr)
//    return (strcmp(sem->NamePtr,"?SEM"))? 1:0;
//  else
//    return 0;         
  OS_SEM_DATA  sem_data;
  return (OSSemQuery (*sem,&sem_data) == OS_ERR_NONE )? 1:0;	
}

/*-----------------------------------------------------------------------------------*/                                                                                                                                                                
void sys_sem_set_invalid(sys_sem_t *sem)                                        
{                                                                               
//  if(sys_sem_valid(sem))
//    sys_sem_free(sem);   
	*sem=NULL;	
} 

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void sys_init(void)
{
//  OS_ERR ucErr;
//  memset(LwIP_task_priority_stask,0,sizeof(LwIP_task_priority_stask));
//  /* init mem used by sys_mbox_t, use ucosIII functions */
//  OSMemCreate(&StackMem,"LWIP TASK STK",(void*)LwIP_Task_Stk,LWIP_TASK_MAX,LWIP_STK_SIZE*sizeof(CPU_STK),&ucErr);
//  LWIP_ASSERT( "sys_init: failed OSMemCreate STK", ucErr == OS_ERR_NONE );
	
	 u8_t ucErr;
    memset(LwIP_task_priopity_stask,0,sizeof(LwIP_task_priopity_stask));

    //由ucos内存管理系统管理该消息队列内存区 pcQueueMemoryPool ，命名为 LWIP mem
    //这块内存区被组织成一张链表结构: TQ_DESCR * MAX_QUEUES，即最多可创建MAX_QUEUES个消息队列
    //由内存控制块pQueueMem控制这块内存
    pQueueMem = OSMemCreate((void*)pcQueueMemoryPool,MAX_QUEUES,sizeof(TQ_DESCR),&ucErr);//为消息队列创建内存分区
    OSMemNameSet (pQueueMem, "LWIP mem", &ucErr);
    LWIP_ASSERT( "sys_init: failed OSMemCreate Q", ucErr == OS_ERR_NONE );

	//由ucos内存管理系统管理总计8个任务堆栈 LwIP_Task_Stk ，命名为 LWIP TASK STK
    //由内存控制块pStackMem控制这块内存
    pStackMem = OSMemCreate((void*)LwIP_Task_Stk,LWIP_TASK_MAX,LWIP_STK_SIZE*sizeof(OS_STK),&ucErr);
    OSMemNameSet (pQueueMem, "LWIP TASK STK", &ucErr);
    LWIP_ASSERT( "sys_init: failed OSMemCreate STK", ucErr == OS_ERR_NONE );
		
}

/*-----------------------------------------------------------------------------------*/
// TODO
/*-----------------------------------------------------------------------------------*/
/*
  Starts a new thread with priority "prio" that will begin its execution in the
  function "thread()". The "arg" argument will be passed as an argument to the
  thread() function. The id of the new thread is returned. Both the id and
  the priority are system dependent.
*/
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread , void *arg, int stacksize, int prio)
{
		int i;
		OS_STK * task_stk;
    u8_t ubPrio = LWIP_TASK_START_PRIO;
    u8_t ucErr;
    arg = arg;
   
    if(prio){
      ubPrio +=(prio-1);
      for(i=0; i<LWIP_TASK_MAX; ++i)
        if(LwIP_task_priopity_stask[i] == ubPrio)
          break;
      if(i == LWIP_TASK_MAX){
        for(i=0; i<LWIP_TASK_MAX; ++i)
          if(LwIP_task_priopity_stask[i]==0){
            LwIP_task_priopity_stask[i] = ubPrio;
            break;
          }
        if(i == LWIP_TASK_MAX){
          LWIP_ASSERT( "sys_thread_new: there is no space for priority", 0 );
          return (-1);
        }
      }else
        prio = 0;
    }
  /* Search for a suitable priority */
    if(!prio){
      ubPrio = LWIP_TASK_START_PRIO;//默认为10
      while(ubPrio < (LWIP_TASK_START_PRIO+LWIP_TASK_MAX)){
        for(i=0; i<LWIP_TASK_MAX; ++i)
          if(LwIP_task_priopity_stask[i] == ubPrio){
            ++ubPrio;
            break;
          }
        if(i == LWIP_TASK_MAX)
          break;
      }
      if(ubPrio < (LWIP_TASK_START_PRIO+LWIP_TASK_MAX))
        for(i=0; i<LWIP_TASK_MAX; ++i)
          if(LwIP_task_priopity_stask[i]==0){
            LwIP_task_priopity_stask[i] = ubPrio;
            break;
          }
      if(ubPrio >= (LWIP_TASK_START_PRIO+LWIP_TASK_MAX) || i == LWIP_TASK_MAX){
        LWIP_ASSERT( "sys_thread_new: there is no free priority", 0 );
        return (-1);
      }
    }
    if(stacksize > LWIP_STK_SIZE || !stacksize)
        stacksize = LWIP_STK_SIZE;
  /* get Stack from pool */
    task_stk = OSMemGet( pStackMem, &ucErr );
    if(ucErr != OS_ERR_NONE){
      LWIP_ASSERT( "sys_thread_new: impossible to get a stack", 0 );
      return (-1);
    }
    //INT16U task_id = ubPrio - LWIP_TASK_START_PRIO + LWIP_TSK_ID;
#if (OS_TASK_STAT_EN == 0)
    OSTaskCreate(thread,
                 (void *)arg,
                 &task_stk[stacksize-1],
                 ubPrio);
#else
    OSTaskCreateExt(thread,
                    (void *)arg,
                    &task_stk[stacksize-1],
                    ubPrio,
                    ubPrio,//task_id,采用ID号与优先级号相同
                    &task_stk[0],
                    stacksize,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK + OS_TASK_OPT_STK_CLR);

#endif
    //OSTaskNameSet(ubPrio, (u8_t*)name, &ucErr);不使能name

    return ubPrio;
}
////lwip延时函数
////ms:要延时的ms数
//void sys_msleep(u32_t ms)
//{
//	delay_ms(ms);
//}
//获取系统时间,LWIP1.4.1增加的函数
//返回值:当前系统时间(单位:毫秒)
u32_t sys_now(void)
{
	u32_t ucos_time, lwip_time;
	ucos_time=OSTimeGet();	//获取当前系统时间 得到的是UCOS的节拍数
	lwip_time=(ucos_time*1000/OS_TICKS_PER_SEC+1);//将节拍数转换为LWIP的时间MS
	return lwip_time; 		//返回lwip_time;
}
