#include "xiaoju_pal.h"
#include "mqtt.h"
#include "xiaoju.h"

static int8_t first = 0;
static char mem_addr[8][512];
OS_MEM *g_mem;

static void create_memory_pool()
{
	INT8U err;
	if(first)
	{
		return ;
	}
	else
	{
		first = 1;
		/* create */
		g_mem = OSMemCreate(&mem_addr[0][0],8,512,&err);
	}
}

/********************************************************
  * @Description：   创建一个任务
  * @Arguments	：
			    threadName      任务名称，指向ASCII字符串的指针
			    thread          任务控制块指针
			    start_routine   任务函数，通常是一个死循环
			    arg             传递给任务函数的参数
			    prio            任务的优先级
			    stack_size      任务的堆栈的总空间大小，单位是word (4byte)
  * @Returns	：
                0   正常
                -1   错误
 *******************************************************/
int xj_pal_thread_create(char* threadName,xj_pal_thread_t *thread,void *(*start_routine)(void *arg),void *par,Task_Priority prio,uint32_t stack_size )
{
	INT8U err;
	
	create_memory_pool();
	
	if(!thread || !g_mem)
		return -1;
	
	thread->stk = (OS_STK *)OSMemGet(g_mem,&err);
	if(err != OS_ERR_NONE)
		return -1;
	thread->task_prio = prio;
	err = OSTaskCreate((void (*)(void *))start_routine,par,thread->stk,prio);
	return (err == OS_ERR_NONE) ? 0 : -1;
}



/********************************************************
  * @Description：删除一个任务
  * @Arguments	：
			    thread          任务控制块指针

  * @Returns	：
                无
 *******************************************************/
void xj_pal_thread_cancel(xj_pal_thread_t* thread)
{
	 INT8U err;
	 if(thread && g_mem)
	 {
			err = OSTaskDel(thread->task_prio);
		  if(err == OS_ERR_NONE)
			{
				OSMemPut(g_mem,(void *)thread->stk);
				
				thread->task_prio = -1;
			}
	 }
}



/********************************************************
  * @Description：任务延时(阻塞)
  * @Arguments	：
			    millsec  延时心跳周期值(1=1ms)

  * @Returns	：
                无
 *******************************************************/
void xj_pal_msleep(uint32_t millsec)
{
	OSTimeDlyHMSM(0,0,0,millsec);
}





/********************************************************
  * @Description：创建互斥信号量-重定义
  * @Arguments	：
			    p_mutex          信号量指针

  * @Returns	：
                无
 *******************************************************/
void Redefine_OSMutexCreate(xj_pal_mutex_t  * p_mutex )
{
	*p_mutex = OSSemCreate(1);
}



/********************************************************
  * @Description：申请互斥信号量-重定义
  * @Arguments	：
			    p_mutex     信号量指针
                timeout     等待时间
  * @Returns	：
                无
 *******************************************************/
void Redefine_OSMutexPend(xj_pal_mutex_t*         p_mutex,uint32_t timeout)
{
	INT8U err;
	if(*p_mutex)
		OSSemPend(*p_mutex,timeout,&err);
}



/********************************************************
  * @Description：释放一个互斥信号量-重定义
  * @Arguments	：
			    p_mutex     信号量指针
  * @Returns	：
                无
 *******************************************************/
void Redefine_OSMutexPost(xj_pal_mutex_t*         p_mutex)
{
	INT8U err;
	if(*p_mutex)
		err = OSSemPost(*p_mutex);
}



/********************************************************
  * @Description：删除一个互斥信号量-重定义
  * @Arguments	：
			    p_mutex     信号量指针
  * @Returns	：
                无
 *******************************************************/
void Redefine_OSMutexDel(xj_pal_mutex_t*         p_mutex)
{
	INT8U err;
	OSSemDel(*p_mutex,OS_DEL_NO_PEND,&err);
}



/********************************************************
  * @Description：消息发送
  * @Arguments	：
			    fd  消息端口句柄
			    buf 待发送缓存
                len 待发送长度
                flags
  * @Returns	：发送字节数

 *******************************************************/
ssize_t xj_pal_sendall(xj_pal_socket_handle fd, const void* buf, size_t len, int flags)
{


}

/********************************************************
  * @Description：消息接收
  * @Arguments	：
			    fd  消息端口句柄
			    buf 待接收缓存
                len 待接收长度
                flags
  * @Returns	：接收字节数

 *******************************************************/
ssize_t xj_pal_recvall(xj_pal_socket_handle fd, void* buf, size_t bufsz, int flags)
{

}


/********************************************************
  * @Description：建立网络连接
  * @Arguments	：
			    addr  ASSCII地址指针
			    port ASCII端口指针
  * @Returns	：连接控制块指针

 *******************************************************/
xj_pal_socket_handle* xj_pal_open_nb_socket(const char* addr, const char* port)
{

	return NULL;
}

/********************************************************
  * @Description：断开网络连接
  * @Arguments	：
			    socket  网络控制块指针
  * @Returns	：

 *******************************************************/
void xj_pal_close_socket(xj_pal_socket_handle* socket)
{

}


/********************************************************
  * @Description：获取实时时间-年
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
int xj_pal_get_int_year(void)
{

}


/********************************************************
  * @Description：获取实时时间-月
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
int xj_pal_get_int_month(void)
{

}


/********************************************************
  * @Description：获取实时时间-日
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
int xj_pal_get_int_day(void)
{

}


/********************************************************
  * @Description：获取实时时间-时
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
int xj_pal_get_int_hour(void)
{

}


/********************************************************
  * @Description：获取实时时间-分
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
int xj_pal_get_int_minute(void)
{

}


/********************************************************
  * @Description：获取实时时间-秒
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
int xj_pal_get_int_sec(void)
{

}


/********************************************************
  * @Description：获取实时时间-时间戳
  * @Arguments	：
			    NULL
  * @Returns	：
                NULL
 *******************************************************/
xj_pal_time_t xj_pal_time(void)
{

}


/********************************************************
  * @Description：保存参数
  * @Arguments	：
                input：参数指针
                size：大小
  * @Returns	：
                0：写入成功
               -1：写入失败
 *******************************************************/
int8_t xj_pal_write_persist_params(char* input,int size)
{

}

/********************************************************
  * @Description：读取参数
  * @Arguments	：
                input：参数指针
                size：大小
  * @Returns	：
                0：读取成功
               -1：读取失败
 *******************************************************/
int8_t xj_pal_read_persist_params(char* output,int limit)
{

}


/********************************************************
  * @Description：日志导出
  * @Arguments	：
			    str  日志字符串
			    len  日志长度
  * @Returns	：

 *******************************************************/
void User_log_export(const char*str,uint32_t len)
{
	console_printf(str);
}




/** @endcond */


