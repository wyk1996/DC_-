#ifndef __XIAOJU_PAL_H__
#define __XIAOJU_PAL_H__

#include "includes.h"

#include "redefine.h"
#include "xiaoju_struct.h"



/*******************************************
�������ȼ�:
�û�����ʵ��ʹ�õĲ���ϵͳ��ʵ������޸Ķ�Ӧ���ȼ�
�� FreeRTOS�У�����Խ�����ȼ�Խ�ߣ���������ȼ�0 ��������ȼ�(configMAX_PRIORITIES �C 1)
   UCOS�С�����ԽС�����ȼ�Խ��
********************************************/
typedef enum
{
    xj_osPriorityIdle          = 0,          ///< priority: idle (lowest)
    xj_osPriorityLow           = 1,          ///< priority: low
    xj_osPriorityBelowNormal   = 2,          ///< priority: below normal
    xj_osPriorityNormal        = 3,          ///< priority: normal (default)
    xj_osPriorityAboveNormal   = 4,          ///< priority: above normal
    xj_osPriorityHigh          = 5,          ///< priority: high
    xj_osPriorityRealtime      = 6,          ///< priority: realtime (highest)
    xj_osPriorityError         =  0x84        ///< system cannot determine priority or thread has illegal priority
} Task_Priority;

/*�����ջ���òο�ֵ*/
#define XJ_OS_STACK_SIZE_LITTLE     128
#define XJ_OS_STACK_SIZE_MEDIUM     256
#define XJ_OS_STACK_SIZE_LARGE      512




/*������־�Ƿ����*/
#define XJ_LOG_RECORD_SWITCH                    _LOG_RECORD_ON_// _LOG_RECORD_OFF_

/*Э����ʼ���ʶ����*/
#define CHARGER_IDENTIFIER_HEADER               0xD07D

/* mqtt ���ͻ����С*/
#define MQTT_SNED_BUF_LEN                       1024

/* mqtt ���ջ����С*/
#define MQTT_RECV_BUF_LEN                       1536

/* unused */
#define CHARGE_GUN_CNT                          2

/* ���׮ǹ�������*/
#define CHARGE_GUN_CNT_MAX                      4

/* Ĭ��ǹ����*/
#define CHARGE_GUN_CNT_DEFAULT                  2

/* ��󱣴��˵�����,�������ǹ����*/
#define MAX_HISTORY_ORDER_CNT	        (CHARGE_GUN_CNT_MAX * 2)

/* ǹ���� (Ĭ��ֱ��)*/
#define XJ_GUN_TYPE                     XJ_GUN_TYPE_DC

/* ���׮����(Ĭ�Ͽɵ��ӿ�����) */
#define XJ_EQUIPMENT_ID                 "TEST00001"

/* ���ķ�������ַ */
#define XJ_CENTER_SVR_ADDR              "unicron.didichuxing.com"

/* ���ķ������˿ں�*/
#define XJ_CENTER_SVR_PORT              1883

/* �߼���������ַ (epower-equipment-server.xiaojukeji.com��ʽ׮)*/
#define XJ_LOGIC_SVR_ADDR               "epower-equipment-server-test.xiaojukeji.com"

/* �߼��������˿ں�*/
#define XJ_LOGIC_SVR_PORT               1884

/* mqtt�û��� (��ʽ׮��Ҫ�޸�)*/
#define XJ_MQTT_USERNAME                "91110113MA01CF8F83"

/* mqtt ���� */
#define XJ_MQTT_PASSWORD                "JvL8so96zyM6ppaTPfEe2JRt9lsnJ07EhT/oQhcCAyuE7Eyo5RoQ0MXBIXyyD13cNN2LqK3ViHLKCFbE/IkKXpeDfIMpCWt8niVn29Vpaf38gtVf0ne7RWPpHC4PlP+gIWLPRVUV1ei1RSeCWfJ4GtDJ0fuOuq7ij0gq/4BIiKU="//��ʽ���޸�

/* mac addr */
#define XJ_MAC_ADDR                     "000000"

/* �û�׮����汾�� (��������)*/
#define USER_VERSION                    0x00010000

/* ��С�� ������(Ĭ��С��)*/
#define XJ_LITTLE_ENDIAN

/* ʹ�ù��ϻ��� */
#define XJ_ERR_CACHE_USED

//MQTT Э����Ҫ���ֽ���ǰ
#if defined(XJ_LITTLE_ENDIAN)
#define XJ_PAL_HTONS(s)     BigLittleSwap16(s)
#define XJ_PAL_NTOHS(s)     BigLittleSwap16(s)
#define XJ_PAL_HTONL(s)     BigLittleSwap32(s)
#else
#define XJ_PAL_HTONS(s)     (s)
#define XJ_PAL_NTOHS(s)     (s)
#define XJ_PAL_HTONL(s)     (s)
#endif

/* type redefined */
typedef OS_EVENT * xj_pal_mutex_t;//�����źŽṹ
typedef int xj_pal_socket_handle;//�������ӿ��ƿ�

struct ucosii_task
{
	OS_STK *stk;
	int8_t task_prio;
};

typedef struct ucosii_task xj_pal_thread_t;//������ƿ�
typedef unsigned int xj_pal_time_t;//ʱ�������


/*
*@brief ��������
*/
int xj_pal_thread_create(char* threadName,xj_pal_thread_t *thread,void *(*start_routine)(void *arg),void *par,Task_Priority prio,uint32_t stack_size );

/*
*@brief ��������
*/
void xj_pal_thread_cancel(xj_pal_thread_t* thread);

/*
*@brief ���뼶sleep
*/
void xj_pal_msleep(uint32_t millsec);

/*
*@brief ����������
*/
void Redefine_OSMutexCreate(xj_pal_mutex_t  * p_mutex );

/*
*@brief ��ȡ������
*/
void Redefine_OSMutexPend(xj_pal_mutex_t*         p_mutex,uint32_t timeout);

/*
*@brief �ͷŻ�����
*/
void Redefine_OSMutexPost(xj_pal_mutex_t*         p_mutex);

/*
*@brief ���ٻ�����
*/
void Redefine_OSMutexDel(xj_pal_mutex_t*         p_mutex);

/*
*@brief ������������
*/
ssize_t xj_pal_sendall(xj_pal_socket_handle fd, const void* buf, size_t len, int flags);

/*
*@brief ������������
*/
ssize_t xj_pal_recvall(xj_pal_socket_handle fd, void* buf, size_t bufsz, int flags);

/*
*@brief ����������
*/
xj_pal_socket_handle* xj_pal_open_nb_socket(const char* addr, const char* port);

/*
*@brief �ر�������
*/
void xj_pal_close_socket(xj_pal_socket_handle* socket);

/*
*@brief ��ȡ��ǰʱ��-��
*/
int xj_pal_get_int_sec(void);

/*
*@brief ��ȡ��ǰʱ��-��
*/
int xj_pal_get_int_minute(void);

/*
*@brief ��ȡ��ǰʱ��-Сʱ(24��)
*/
int xj_pal_get_int_hour(void);

/*
*@brief ��ȡ��ǰʱ��-��
*/
int xj_pal_get_int_day(void);

/*
*@brief ��ȡ��ǰʱ��-��
*/
int xj_pal_get_int_month(void);

/*
*@brief ��ȡ��ǰʱ��-��
*/
int xj_pal_get_int_year(void);

/*
*@brief ��ȡʱ���(�����1970-1-1�����ڵ�����)
*/
xj_pal_time_t xj_pal_time(void);

/*
*@brief ����д��
*/
int8_t xj_pal_write_persist_params(char* input,int size);

/*
*@brief ���ݶ�ȡ
*/
int8_t xj_pal_read_persist_params(char* output,int limit);

/*
*@brief ��־���
*/
void User_log_export(const char*str,uint32_t len);

/* for mqtt */
#define XJ_PAL_MUTEX_INIT(mtx_ptr)      (Redefine_OSMutexCreate((mtx_ptr)))
#define XJ_PAL_MUTEX_LOCK(mtx_ptr)      (Redefine_OSMutexPend((mtx_ptr),100))
#define XJ_PAL_MUTEX_UNLOCK(mtx_ptr)    (Redefine_OSMutexPost((mtx_ptr)))
#define XJ_PAL_MUTEX_DESTORY(mtx_ptr)   (Redefine_OSMutexDel((mtx_ptr)))



#define MQTT_PAL_MUTEX_INIT(mtx_ptr) XJ_PAL_MUTEX_INIT(mtx_ptr)
#define MQTT_PAL_MUTEX_LOCK(mtx_ptr) XJ_PAL_MUTEX_LOCK(mtx_ptr)
#define MQTT_PAL_MUTEX_UNLOCK(mtx_ptr) XJ_PAL_MUTEX_UNLOCK(mtx_ptr)
#define MQTT_PAL_MUTEX_DESTORY(mtx_ptr) XJ_PAL_MUTEX_DESTORY(mtx_ptr)

#define MQTT_PAL_TIME() xj_pal_time()
#define MQTT_PAL_HTONS(s) XJ_PAL_HTONS(s)
#define MQTT_PAL_NTOHS(s) XJ_PAL_NTOHS(s)
#define mqtt_pal_recvall xj_pal_recvall
#define mqtt_pal_sendall xj_pal_sendall

typedef xj_pal_time_t mqtt_pal_time_t;
typedef xj_pal_mutex_t mqtt_pal_mutex_t;
typedef xj_pal_socket_handle mqtt_pal_socket_handle;


/********************************************************
  * @Description���û�ʵ�� �˵�չʾ
  * @Arguments	��
			     bill_data:�˵���Ϣ
			     gun_cnt���˵���Ӧ��ǹ��
			     0��ʾǹ1 ,1��ʾǹ2,�Դ�����
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
 *******************************************************/
extern int8_t callback_show_bill(xj_bill_info bill_data,uint8_t gun_cnt);

/********************************************************
  * @Description���û�ʵ�� ��ʾ��ά��
  * @Arguments	��
			    gun_cnt:Ҫ��ʾ��ǹ�� ������С��ǹ����
			    0��ʾǹ1 ,1��ʾǹ2,�Դ�����
			    qr_str: ��ά���ַ���
			    len���ַ�����
			    err: ִ�н��,��4�ֽ�ASCII��ʾ��"0000"��ʾ�ɹ� "FFFF"��ʾʧ��
  * @Returns	��
                ��
 *******************************************************/
extern void callback_display_qr(uint8_t gun_cnt,uint8_t*qr_str,uint16_t len,char* err);




/********************************************************
  * @Description���û�ʵ�� vin������������ص�
  * @Arguments	��
			     result:�ο�ע��
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
    @remark     ��SDK���ڼ�Ȩ�ɹ�����¶Ըûص��Ľ���жϣ�����0�ɹ�������Լ�״̬��
                    �û�������ȫ�����������    xj_send_touch_charge_start   ��
                  ����Ȩʧ�ܻ�ûص���������������ԭ��״̬
 *******************************************************/
extern int8_t callback_vin_start_charge_result(st_user_vin_send_result result);


/********************************************************
  * @Description���û�ʵ�� ������� �ص�����
  * @Arguments	��
			    gun_cnt:������ǹ�� ������С��ǹ����
			    0��ʾǹ1 ,1��ʾǹ2,�Դ�����
			    err: ִ�н��,��4�ֽ�ASCII��ʾ��"0000"��ʾ�ɹ�
			    id��������
			    id_len �����ų���
			    user_tel �û��ֻ�β��
  * @Returns	��
                ��
 *******************************************************/
extern void callback_start_charge(uint8_t gun_cnt,char* err,char* id,uint8_t id_len,uint16_t user_tel);


/********************************************************
  * @Description���û�ʵ�� ֹͣ��� �ص�����
  * @Arguments	��
			    gun_cnt:������ǹ�� ������С��ǹ����
			    0��ʾǹ1 ,1��ʾǹ2,�Դ�����
			    err: ִ�н��,��4�ֽ�ASCII��ʾ��"0000"��ʾ�ɹ�
  * @Returns	��
                ��
 *******************************************************/
extern void callback_stop_charge(uint8_t gun_cnt, char* err);




/********************************************************
  * @Description���û�ʵ�� ����ϵͳʱ��
  * @Arguments	��
			    time:ʱ��ṹ��
			    err: ִ�н��,��4�ֽ�ASCII��ʾ��"0000"��ʾ�ɹ� "FFFF"��ʾʧ��
  * @Returns	��
                ��
 *******************************************************/
extern void callback_set_sys_time(xj_sync_system_time_param time,char* err);


/********************************************************
  * @Description���û�ʵ�� �������  ���첽ִ��
  * @Arguments	��
			    param->upgrade_type ��������
                param->target_type Ŀ������
                param->url�������ص�http url �ַ���         ��󳤶�128
                param->checksum:�ļ���MD5У����   ��󳤶�32
                param->serial_code:code ���к��� �ظ�1102ʱʹ��
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
 *******************************************************/
extern int8_t callback_software_download(xj_software_upgrade_param *param);



/********************************************************
  * @Description���û�ʵ�� �˵��洢
  * @Arguments	��
			    bill:д���˵���Ϣָ��
			    size:���ݴ�С
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
 *******************************************************/
extern int8_t callback_save_bill(char* bill,uint32_t size);


/********************************************************
  * @Description���û�ʵ�� ��ȡ�����˵�
  * @Arguments	��
			    bill:��ȡ�˵���Ϣָ��
			    len:�˵���Ϣ��С
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
 *******************************************************/
extern int8_t callback_read_bill(char* bill,uint32_t len);


/********************************************************
  * @Description���û�ʵ�� �ϴ���־
  * @Arguments	��
			     upload_url:http post�ĵ�ַ
			     log_name:�û��ϴ�����־��
			     max_len����־�������д�볤��
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
 *******************************************************/
extern int8_t callback_upload_log(const xj_upload_log_param* upload_url,char* log_name,uint16_t max_len);


/********************************************************
  * @Description���û�ʵ�� ����������
  * @Arguments	��
			     gun_cnt:Ҫ��ʾ��ǹ�� ������С��ǹ����
			     0��ʾǹ1 ,1��ʾǹ2,�Դ�����
			     type:�������� 0���� 1����
  * @Returns	��
                  �ɹ�����0
                  ʧ�ܷ�������
 *******************************************************/
extern int8_t callback_lock_control(uint8_t gun_cnt,uint8_t type);




#endif

