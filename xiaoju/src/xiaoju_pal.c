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
  * @Description��   ����һ������
  * @Arguments	��
			    threadName      �������ƣ�ָ��ASCII�ַ�����ָ��
			    thread          ������ƿ�ָ��
			    start_routine   ��������ͨ����һ����ѭ��
			    arg             ���ݸ��������Ĳ���
			    prio            ��������ȼ�
			    stack_size      ����Ķ�ջ���ܿռ��С����λ��word (4byte)
  * @Returns	��
                0   ����
                -1   ����
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
  * @Description��ɾ��һ������
  * @Arguments	��
			    thread          ������ƿ�ָ��

  * @Returns	��
                ��
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
  * @Description��������ʱ(����)
  * @Arguments	��
			    millsec  ��ʱ��������ֵ(1=1ms)

  * @Returns	��
                ��
 *******************************************************/
void xj_pal_msleep(uint32_t millsec)
{
	OSTimeDlyHMSM(0,0,0,millsec);
}





/********************************************************
  * @Description�����������ź���-�ض���
  * @Arguments	��
			    p_mutex          �ź���ָ��

  * @Returns	��
                ��
 *******************************************************/
void Redefine_OSMutexCreate(xj_pal_mutex_t  * p_mutex )
{
	*p_mutex = OSSemCreate(1);
}



/********************************************************
  * @Description�����뻥���ź���-�ض���
  * @Arguments	��
			    p_mutex     �ź���ָ��
                timeout     �ȴ�ʱ��
  * @Returns	��
                ��
 *******************************************************/
void Redefine_OSMutexPend(xj_pal_mutex_t*         p_mutex,uint32_t timeout)
{
	INT8U err;
	if(*p_mutex)
		OSSemPend(*p_mutex,timeout,&err);
}



/********************************************************
  * @Description���ͷ�һ�������ź���-�ض���
  * @Arguments	��
			    p_mutex     �ź���ָ��
  * @Returns	��
                ��
 *******************************************************/
void Redefine_OSMutexPost(xj_pal_mutex_t*         p_mutex)
{
	INT8U err;
	if(*p_mutex)
		err = OSSemPost(*p_mutex);
}



/********************************************************
  * @Description��ɾ��һ�������ź���-�ض���
  * @Arguments	��
			    p_mutex     �ź���ָ��
  * @Returns	��
                ��
 *******************************************************/
void Redefine_OSMutexDel(xj_pal_mutex_t*         p_mutex)
{
	INT8U err;
	OSSemDel(*p_mutex,OS_DEL_NO_PEND,&err);
}



/********************************************************
  * @Description����Ϣ����
  * @Arguments	��
			    fd  ��Ϣ�˿ھ��
			    buf �����ͻ���
                len �����ͳ���
                flags
  * @Returns	�������ֽ���

 *******************************************************/
ssize_t xj_pal_sendall(xj_pal_socket_handle fd, const void* buf, size_t len, int flags)
{


}

/********************************************************
  * @Description����Ϣ����
  * @Arguments	��
			    fd  ��Ϣ�˿ھ��
			    buf �����ջ���
                len �����ճ���
                flags
  * @Returns	�������ֽ���

 *******************************************************/
ssize_t xj_pal_recvall(xj_pal_socket_handle fd, void* buf, size_t bufsz, int flags)
{

}


/********************************************************
  * @Description��������������
  * @Arguments	��
			    addr  ASSCII��ַָ��
			    port ASCII�˿�ָ��
  * @Returns	�����ӿ��ƿ�ָ��

 *******************************************************/
xj_pal_socket_handle* xj_pal_open_nb_socket(const char* addr, const char* port)
{

	return NULL;
}

/********************************************************
  * @Description���Ͽ���������
  * @Arguments	��
			    socket  ������ƿ�ָ��
  * @Returns	��

 *******************************************************/
void xj_pal_close_socket(xj_pal_socket_handle* socket)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-��
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
int xj_pal_get_int_year(void)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-��
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
int xj_pal_get_int_month(void)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-��
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
int xj_pal_get_int_day(void)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-ʱ
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
int xj_pal_get_int_hour(void)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-��
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
int xj_pal_get_int_minute(void)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-��
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
int xj_pal_get_int_sec(void)
{

}


/********************************************************
  * @Description����ȡʵʱʱ��-ʱ���
  * @Arguments	��
			    NULL
  * @Returns	��
                NULL
 *******************************************************/
xj_pal_time_t xj_pal_time(void)
{

}


/********************************************************
  * @Description���������
  * @Arguments	��
                input������ָ��
                size����С
  * @Returns	��
                0��д��ɹ�
               -1��д��ʧ��
 *******************************************************/
int8_t xj_pal_write_persist_params(char* input,int size)
{

}

/********************************************************
  * @Description����ȡ����
  * @Arguments	��
                input������ָ��
                size����С
  * @Returns	��
                0����ȡ�ɹ�
               -1����ȡʧ��
 *******************************************************/
int8_t xj_pal_read_persist_params(char* output,int limit)
{

}


/********************************************************
  * @Description����־����
  * @Arguments	��
			    str  ��־�ַ���
			    len  ��־����
  * @Returns	��

 *******************************************************/
void User_log_export(const char*str,uint32_t len)
{
	console_printf(str);
}




/** @endcond */


