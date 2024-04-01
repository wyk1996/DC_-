#include "xiaoju.h"
#include "stdio.h"
#include "string.h"

int _g_is_login = 0;

st_user_gun_info* p_User_gun_info;
st_user_bms_info* p_User_bms_info;
static xj_gun_status g_xj_gun_status[CHARGE_GUN_CNT_MAX];
static xj_bms_status g_xj_bms_status[CHARGE_GUN_CNT_MAX];

#if defined(XJ_ERR_CACHE_USED)
/* 缓存断网时发送的故障产生或恢复事件 */
static st_errcache_queue_t g_err_event_info;
#endif // defined

static st_xj_incident_info  xj_incident_info;//xj事件处理列表；

//char System_Storage;
static xj_params g_xj_params;
static xj_pal_thread_t xj_thread_incident;
static xj_pal_thread_t xj_thread_message_handle;
static xj_pal_thread_t xj_thread_sync_status;



static char _g_stack_var_for_callback_thread[360];
static char _g_stack_var_for_interval_work_thread[360];
static _mqtt_resp_to_send _g_mqtt_resp_to_send_queue[MAX_BLOCKED_MQTT_RESP_NUM];
static xj_params* _g_xj_params;

static st_xj_histroy_bill xj_histroy_bill_info[MAX_HISTORY_ORDER_CNT];
static xj_bill_info _g_xj_current_bill_info[CHARGE_GUN_CNT_MAX];

/* 函数声明 */
//static void set_qr(void);
void* _on_cmd_received_callback(void* cmd,xj_mqtt_cmd_enum cmd_type);


uint16_t set_sign_in_min_interval = 1;
uint16_t set_upload_gun_status_min_interval = 1;
uint16_t set_heartbeat_min_interval = 1;
uint16_t set_heartbeat_timeout_check_cnt_min = 1;






const st_set_param_info set_501_param_list[] =
{
    {//0
        0,0,0,0
    },//保留
    {//1
        &g_xj_params.sign_in_interval,
        type_uint16,
        NULL,
        &set_sign_in_min_interval
    },
    {
        &g_xj_params.upload_gun_status_interval,
        type_uint16,
        NULL,
        &set_upload_gun_status_min_interval
    },
    {
        &g_xj_params.heartbeat_interval,
        type_uint16,
        NULL,
        &set_heartbeat_min_interval
    },
    {
        &g_xj_params.heartbeat_timeout_check_cnt,
        type_uint16,
        NULL,
        &set_heartbeat_timeout_check_cnt_min
    },
    {
        &g_xj_params.equipment_id[0],
        type_string32,
        NULL,
        NULL
    },
    {
        &g_xj_params.gun_qr_code,
        type_string130,
        NULL,
        NULL
    },
    {
        &g_xj_params.logic_svr_addr[0],
        type_string64,
        NULL,
        NULL
    },
    {
        &g_xj_params.logic_svr_port,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.log_level,
        type_uint8,
        NULL,
        NULL
    },
    {
        &g_xj_params.log_strategy,
        type_uint8,
        NULL,
        NULL
    },
    {
        &g_xj_params.log_period,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_12,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_13,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_14,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_15,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_16,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_17,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_18,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_19,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_20,
        type_uint32,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_21,
        type_uint32,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_22,
        type_uint32,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_23,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_24,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_25,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_26,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_27,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_28,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_29,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_30,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_31,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_32,
        type_uint32,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_33,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_34,
        type_uint16,
        NULL,
        NULL
    },
    {
        &g_xj_params.safety_parameters.safety_35,
        type_uint16,
        NULL,
        NULL
    }

};

/********************************************************
  * @Description：获取对应参数类型的参数长度
  * @Arguments	：
                param_type 参数类型
  * @Returns	：
                参数长度
 ********************************************************/
static uint16_t get_params_length(xj_param_type param_type)
{
    uint16_t length = 0;

    switch(param_type)
    {
        case type_int8:
            length = 1;
            break;
        case type_uint8:
            length = 1;
            break;
        case type_int16:
            length = 2;
            break;
        case type_uint16:
            length = 2;
            break;
        case type_int32:
            length = 4;
            break;
        case type_uint32:
            length = 4;
            break;
        case type_int64:
            length = 8;
            break;
        case type_uint64:
            length = 8;
            break;
        case type_float:
            length = 4;
            break;
        case type_double:
            length = 8;
            break;
        case type_string8:
            length = 8;
            break;
        case type_string16:
            length = 16;
            break;
        case type_string32:
            length = 32;
            break;
        case type_string64:
            length = 64;
            break;
        case type_string128:
            length = 128;
            break;
        case type_string130:
            length = 130;
            break;
        case type_string66:
            length = 66;
            break;

    }
    return length;
}

/********************************************************
  * @Description：判断参数设置范围
  * @Arguments	：
                param 参数值
                param_range 范围值
                param_type 参数类型
                range_type 判断类型
  * @Returns	：
                FLASE 设置参数范围错误
                TRUE  设置参数符合范围
 ********************************************************/
static uint8_t judge_param_range(void* param ,void* param_range ,xj_param_type param_type,uint8_t range_type)
{
    uint8_t result = FALSE;

    if(NULL != param && NULL != param_range)
    {
        switch(param_type)
        {
            case type_int8:
                if((RANGE_MIN_TYPE == range_type && (*(int8_t*)param) >= (*(int8_t*)param_range))
                || (RANGE_MAX_TYPE == range_type && (*(int8_t*)param) <= (*(int8_t*)param_range)))
                {
                     result = TRUE;
                }
                break;
            case type_uint8:
                if((RANGE_MIN_TYPE == range_type && (*(uint8_t*)param)  >= (*(uint8_t*)param_range))
                || (RANGE_MAX_TYPE == range_type && (*(uint8_t*)param) <= (*(uint8_t*)param_range)))
                {
                     result = TRUE;
                }
                break;
            case type_int16:
                if((RANGE_MIN_TYPE == range_type && (*(int16_t*)param)  >= (*(int16_t*)param_range))
                || (RANGE_MAX_TYPE == range_type && (*(int16_t*)param) <= (*(int16_t*)param_range)))
                {
                     result = TRUE;
                }
                break;
            case type_uint16:
                if((RANGE_MIN_TYPE == range_type && (*(uint16_t*)param)  >= (*(uint16_t*)param_range))
                || (RANGE_MAX_TYPE == range_type && (*(uint16_t*)param) <= (*(uint16_t*)param_range)))
                {
                     result = TRUE;
                }
                 break;
            case type_int32:
                if((RANGE_MIN_TYPE == range_type && (*(int32_t*)param)  >= (*(int32_t*)param_range))
                || (RANGE_MAX_TYPE == range_type && (*(int32_t*)param) <= (*(int32_t*)param_range)))
                {
                     result = TRUE;
                }
                 break;
            case type_uint32:
                if((RANGE_MIN_TYPE == range_type && (*(uint32_t*)param)  >= (*(uint32_t*)param_range))
                || (RANGE_MAX_TYPE == range_type && (*(uint32_t*)param) <= (*(uint32_t*)param_range)))
                {
                     result = TRUE;
                }
                 break;
            case type_float:
                if((RANGE_MIN_TYPE == range_type && ((*(float*)param)   > (*(float*)param_range) || fabs((*(float*)param)-(*(float*)param_range)) < 1e-8))
                || (RANGE_MAX_TYPE == range_type && ((*(float*)param) < (*(float*)param_range) || fabs((*(float*)param)-(*(float*)param_range)) < 1e-8)))
                {
                     result = TRUE;
                }
                 break;
            case type_double:
                if((RANGE_MIN_TYPE == range_type && ((*(double*)param)   > (*(double*)param_range) || fabs((*(double*)param)-(*(double*)param_range)) < 1e-8))
                || (RANGE_MAX_TYPE == range_type && ((*(double*)param) < (*(double*)param_range) || fabs((*(double*)param)-(*(double*)param_range)) < 1e-8)))
                {
                     result = TRUE;
                }
                 break;
        }
    }
    return result;
}


/********************************************************
  * @Description：更新枪状态
  * @Arguments	：
                p_xj_gun_status：  更新的目标
                p_user_gun_status->：更新的源
                gun_cnt：更新的数量
  * @Returns	：
 ********************************************************/
void sync_gun_status(xj_gun_status* p_xj_gun_status ,st_user_gun_info* p_user_gun_status ,uint16_t gun_cnt )
{
	uint8_t i = 0;
    #if 0
    uint32_t count = 0;
    #endif
    if(NULL != p_xj_gun_status && NULL != p_user_gun_status && 0 != gun_cnt)
    {
        for(i = 0 ;i < gun_cnt ;i++)
        {
        	p_xj_gun_status[i].soc_percent = p_user_gun_status[i].soc_percent;
        	p_xj_gun_status[i].dc_charge_voltage = p_user_gun_status[i].dc_charge_voltage;
        	p_xj_gun_status[i].dc_charge_current = p_user_gun_status[i].dc_charge_current;
        	p_xj_gun_status[i].bms_need_voltage = p_user_gun_status[i].bms_need_voltage;
        	p_xj_gun_status[i].bms_need_current = p_user_gun_status[i].bms_need_current;
        	p_xj_gun_status[i].ac_a_vol = p_user_gun_status[i].ac_a_vol;
        	p_xj_gun_status[i].ac_b_vol = p_user_gun_status[i].ac_b_vol;
        	p_xj_gun_status[i].ac_c_vol = p_user_gun_status[i].ac_c_vol;
        	p_xj_gun_status[i].ac_a_cur = p_user_gun_status[i].ac_a_cur;
        	p_xj_gun_status[i].ac_b_cur = p_user_gun_status[i].ac_b_cur;
        	p_xj_gun_status[i].ac_c_cur = p_user_gun_status[i].ac_c_cur;

        	if(p_user_gun_status[i].meter_kwh_num >= p_xj_gun_status[i].current_meter_kwh_num)
        	{
        	    p_xj_gun_status[i].current_meter_kwh_num = p_user_gun_status[i].meter_kwh_num;
#if 0
                count = 0;
#endif

        	}
        	else
        	{
#if 0
                /* 处理电表读数上限后回退到0的问题*/
                count++;
                if(count >= 30 && ((g_xj_gun_status[i].work_stat == xj_work_stat_type_bist)
                                   || (g_xj_gun_status[i].work_stat == xj_work_stat_type_charging)))
                {
                    count = 0;
                    xj_send_touch_charge_stoped(i,"600C","0000");
                    xj_send_Electric_meter_warning(i);
                }
#else
                xj_send_Electric_meter_warning(i);
#endif
        	}

        	p_xj_gun_status[i].charge_power_kw = p_user_gun_status[i].charge_power_kw;
        }
	}
	else
	{
        xj_pal_print_log(xj_log_Error,"Func：sync_gun_status param error\n");
	}

}


/********************************************************
  * @Description：更新BMS状态
  * @Arguments	：
			    p_xj_bms_status:更新的目标
			    p_user_bms_status：更新的源
			    gun_cnt：更新的数量
  * @Returns	：
 ********************************************************/
void sync_bms_status(xj_bms_status* p_xj_bms_status ,st_user_bms_info* p_user_bms_status ,uint16_t gun_cnt )
{
	int i = 0;

	if(NULL != p_xj_bms_status && NULL != p_user_bms_status && 0 != gun_cnt)
	{
    	for(i = 0 ;i < gun_cnt ;i++ )
        {

            p_xj_bms_status[i].bcl_voltage_need = p_user_bms_status[i].bcl_voltage_need;
            p_xj_bms_status[i].bcl_current_need = p_user_bms_status[i].bcl_current_need;
            p_xj_bms_status[i].bcl_charge_mode = p_user_bms_status[i].bcl_charge_mode;
            p_xj_bms_status[i].bcs_test_voltage = p_user_bms_status[i].bcs_test_voltage;
            p_xj_bms_status[i].bcs_test_current = p_user_bms_status[i].bcs_test_current;
            p_xj_bms_status[i].bcs_max_single_voltage = p_user_bms_status[i].bcs_max_single_voltage;
            p_xj_bms_status[i].bcs_max_single_no = p_user_bms_status[i].bcs_max_single_no;
            p_xj_bms_status[i].bcs_current_soc = p_user_bms_status[i].bcs_current_soc;
            p_xj_bms_status[i].charge_time_left = p_user_bms_status[i].charge_time_left;
            p_xj_bms_status[i].bsm_single_no = p_user_bms_status[i].bsm_single_no;
            p_xj_bms_status[i].bsm_max_temperature = p_user_bms_status[i].bsm_max_temperature;
            p_xj_bms_status[i].bsm_max_temperature_check_no = p_user_bms_status[i].bsm_max_temperature_check_no;
            p_xj_bms_status[i].bsm_min_temperature = p_user_bms_status[i].bsm_min_temperature;
            p_xj_bms_status[i].bsm_min_temperature_check_no = p_user_bms_status[i].bsm_min_temperature_check_no;
            p_xj_bms_status[i].bsm_voltage_too_high_or_too_low = p_user_bms_status[i].bsm_voltage_too_high_or_too_low;
            p_xj_bms_status[i].bsm_car_battery_soc_too_high_or_too_low = p_user_bms_status[i].bsm_car_battery_soc_too_high_or_too_low;
            p_xj_bms_status[i].bsm_car_battery_charge_over_current = p_user_bms_status[i].bsm_car_battery_charge_over_current;
            p_xj_bms_status[i].bsm_battery_temperature_too_high = p_user_bms_status[i].bsm_battery_temperature_too_high;
            p_xj_bms_status[i].bsm_battery_insulation_state = p_user_bms_status[i].bsm_battery_insulation_state;
            p_xj_bms_status[i].bsm_battery_connect_state = p_user_bms_status[i].bsm_battery_connect_state;
            p_xj_bms_status[i].bsm_allow_charge = p_user_bms_status[i].bsm_allow_charge;

        }
	}
	else
	{
        xj_pal_print_log(xj_log_Error,"Func：sync_bms_status param error\n");
	}


 }



/********************************************************
  * @Description：增加 处理事件
  * @Arguments	：
			    info  队列表
			    incident 要增加的事件
  * @Returns	：
                0 成功
                -1 失败
 *******************************************************/
static int8_t xj_add_incident_handle(st_xj_incident_info* info,xj_incident_type incident,uint8_t* data,uint16_t len)
{
    uint8_t point_to_end = 0;
    int8_t result = -1;

    XJ_PAL_MUTEX_LOCK(&info->incident_lock);
    if(info->incident_sum < MAX_XJ_INCIDENT_SUM && ((NULL != data && len <= MAX_XJ_INCIDENT_DATA_LEN) || (NULL == data && 0 == len)))
    {
        point_to_end = info->incident_new + info->incident_sum;
        point_to_end = (point_to_end >= MAX_XJ_INCIDENT_SUM) ? ( point_to_end -= MAX_XJ_INCIDENT_SUM) : (point_to_end);
        info->incident_list[point_to_end].incident_name = incident;
        if(NULL != data)
        {
            memcpy(&info->incident_list[point_to_end].incident_data[0],data,len);
            info->incident_list[point_to_end].incident_len = len;
        }
        info->incident_sum++;
        result = 0;
    }
    else
    {
        xj_pal_print_log(xj_log_Error,"Unable to add incident incident_sum =%d,data addr =%x\n",info->incident_sum,data);
    }

    XJ_PAL_MUTEX_UNLOCK(&info->incident_lock);
    return result;
}

/********************************************************
  * @Description：消除 已处理事件
  * @Arguments	：
			    info  队列表
  * @Returns	：

 *******************************************************/
static void xj_del_recently_incident_handle(st_xj_incident_info* info)
{
    uint8_t point_to_head = info->incident_new;

    XJ_PAL_MUTEX_LOCK(&info->incident_lock);

    if(info->incident_new < MAX_XJ_INCIDENT_SUM && info->incident_sum >= 1)
    {
        info->incident_list[info->incident_new].incident_name = xj_incident_type_null;
        memset(&info->incident_list[info->incident_new].incident_data[0],00,MAX_XJ_INCIDENT_DATA_LEN);
        info->incident_list[info->incident_new].incident_len = 0;
        info->incident_sum--;
        if(info->incident_sum > 0)
        {
            point_to_head++;
            (point_to_head >= MAX_XJ_INCIDENT_SUM) ? ( point_to_head = 0) : (point_to_head);
            info->incident_new = point_to_head;
        }
    }
    else
    {
        xj_pal_print_log(xj_log_Error,"Unable to del recently incident_sum =%d,incident_new =%d\n",info->incident_sum,info->incident_new);

    }

    XJ_PAL_MUTEX_UNLOCK(&info->incident_lock);
}


void _inqueue_mqtt_resp_to_send(_mqtt_resp_to_send *resp)
{
	int i=0;
	for(i=0;i<MAX_BLOCKED_MQTT_RESP_NUM;i++)
	{
		if(_g_mqtt_resp_to_send_queue[i].submited==0)
		{
			memcpy(&(_g_mqtt_resp_to_send_queue[i]),resp,sizeof(_mqtt_resp_to_send));
			_g_mqtt_resp_to_send_queue[i].submited=1;
			return;
		}
	}
    xj_pal_print_log(xj_log_Error,"error! _g_mqtt_resp_to_send_queue exceeds MAX_BLOCKED_MQTT_RESP_NUM\n");
}

void _xj_mqtt_send_resp_wrap(xj_mqtt_cmd_enum cmd_type,char* cmd_value,int32_t size,uint8_t qos)
{
	static _mqtt_resp_to_send _tmp_resp;
	_tmp_resp.qos = qos;
	_tmp_resp.submited = 0;
	_tmp_resp.cmd_type = cmd_type;
	memcpy(_tmp_resp.data,cmd_value,size);
	_inqueue_mqtt_resp_to_send(&_tmp_resp);
}

static void xj_save_charge_order(void)
{
    if(0 != callback_save_bill((char*)&xj_histroy_bill_info,sizeof(xj_histroy_bill_info)))
    {
        xj_pal_print_log(xj_log_Error,"Save bill failure!\n");
    }
}

//更新历史账单
static void xj_updata_history_bill(xj_bill_info new_bill ,xj_bill_status now_status)
{
    static uint32_t before_time = 0;//上一次时间
    uint32_t current_time = 0;//当前时间
    uint8_t i = 0;
    uint8_t new_bill_flag = TRUE;

    for(i = 0;i < MAX_HISTORY_ORDER_CNT;i++)
    {
        if(xj_bill_not_upload == xj_histroy_bill_info[i].status)
        {
            if(0 == strncmp(xj_histroy_bill_info[i].bill.charge_user_id,new_bill.charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
            {
                xj_histroy_bill_info[i].bill = new_bill;
                xj_histroy_bill_info[i].status = now_status;
                new_bill_flag = FALSE;
                break;
            }
        }
    }

    if(TRUE == new_bill_flag)
    {
        for(i = 0;i < MAX_HISTORY_ORDER_CNT;i++)
        {
            if(xj_bill_not_upload != xj_histroy_bill_info[i].status && xj_bill_ready_upload != xj_histroy_bill_info[i].status)
            {
                    //z+++ 找到最旧的账单
                    xj_histroy_bill_info[i].bill = new_bill;
                    xj_histroy_bill_info[i].status = now_status;
                    new_bill_flag = FALSE;
                    break;
            }
        }
    }

    if(TRUE == new_bill_flag)
    {
        xj_pal_print_log(xj_log_Error,"History bill update failure!\n");
    }

    current_time = (uint32_t)xj_pal_time();
    //30S一个周期存储一遍账单，如果遇到结束账单，直接保存
    if((abs(current_time - before_time) >= BILL_SAVE_PERIOD) || (xj_bill_ready_upload == now_status) || (xj_bill_completed_Upload == now_status))
    {
        before_time = current_time;
        xj_save_charge_order();//存储到ROM中
    }
}

/**
 * @brief 银行家算法-四舍五入
 * @param fee
 * @return
 */
static uint32_t bank_round(uint32_t fee)
{
    /* 1.23 32 1 */
#define unit_1      1       /*个位 */
#define unit_10     10      /*十位 */
#define unit_100    100     /*百位 */
#define unit_1000   1000    /*千位 */
#define unit_unpack(n)    ((value / (n)) % 10)
#define unit_pack(n)      (value += (1 * n))

    uint32_t value = fee;

    /*5位小数,删减成2位小数  1.23 32 1*/
    if(unit_unpack(unit_100) >= 6) /* 6进1 */
    {
        unit_pack(unit_1000);
    }
    else if(unit_unpack(unit_100) == 5)  /* 小于5直接舍弃 直接判断==5*/
    {
        /* 需要看前一位和后一位的数据*/

        if(unit_unpack(unit_10) != 0)
        {
            /* 后一位 非0 进1 */
            unit_pack(unit_1000);
        }
        else
        {
            /* 后一位为0 要判断奇偶 */

            if((unit_unpack(unit_1000) % 2) != 0) /*奇数进1 偶数舍弃 */
            {
                unit_pack(unit_1000);
            }
        }
    }

    return value / 1000;
}

/********************************************************
  * @Description： xj账单信息更新
  * @Arguments	：
  * @Returns	：
 ********************************************************/
void _interval_work_for_calculate_bill_info(void)
{
    static _calculate_bill_info_param bill_info = {0};//账单信息参数
    uint8_t i = 0;

	for(i = 0; i<_g_xj_params->gun_cnt ;i++)
	{
	    bill_info.info[i].updata_time++;
	    //首次进入自检或者开始充电 就立刻进入计费，通过  两个状态判断 避免自检过程过快
        if(TRUE != bill_info.info[i].start_bill_flag && (xj_work_stat_type_charging == g_xj_gun_status[i].work_stat || xj_work_stat_type_bist == g_xj_gun_status[i].work_stat))
        {
            memset((char*)&bill_info.info[i],00,sizeof(st_calculate_bill));
            memset(&_g_xj_current_bill_info[i],0,sizeof(xj_bill_info));
            memset(g_xj_gun_status[i].stop_reason,0,XJ_MAX_ERR_NO_LEN);
		    bill_info.info[i].start_bill_flag = TRUE;
			bill_info.info[i].fee_config = _g_xj_params->fee_config;
			/*上一次电表读数*/
			bill_info.info[i].last_kwh = g_xj_gun_status[i].current_meter_kwh_num;
			g_xj_gun_status[i].before_charge_meter_kwh_num = g_xj_gun_status[i].current_meter_kwh_num;
			g_xj_gun_status[i].charged_sec = 0;
			g_xj_gun_status[i].cumulative_charge_fee = 0;
			g_xj_gun_status[i].start_charge_time_year = xj_pal_get_int_year();
			g_xj_gun_status[i].start_charge_time_month = xj_pal_get_int_month();
			g_xj_gun_status[i].start_charge_time_day = xj_pal_get_int_day();
			g_xj_gun_status[i].start_charge_time_hour = xj_pal_get_int_hour();
			g_xj_gun_status[i].start_charge_time_minute = xj_pal_get_int_minute();
			g_xj_gun_status[i].start_charge_time_sec = xj_pal_get_int_sec();
			bill_info.info[i].start_charge_sec = xj_pal_time();
			_g_xj_current_bill_info[i].gun_index = i;
			memcpy(_g_xj_current_bill_info[i].charge_user_id,g_xj_gun_status[i].charge_user_id,32);



			if(TRUE != bill_info.info[i].start_soc_flag && 0 != g_xj_gun_status[i].soc_percent)
			{
			    _g_xj_current_bill_info[i].start_soc = g_xj_gun_status[i].soc_percent;
			    bill_info.info[i].start_soc_flag = TRUE;
			}
			/* 开始充电时的电表读数 */
			_g_xj_current_bill_info[i].start_charge_kwh_meter = g_xj_gun_status[i].current_meter_kwh_num;
			_g_xj_current_bill_info[i].charge_start_time_year = xj_pal_get_int_year();
			_g_xj_current_bill_info[i].charge_start_time_month = xj_pal_get_int_month();
			_g_xj_current_bill_info[i].charge_start_time_day = xj_pal_get_int_day();
			_g_xj_current_bill_info[i].charge_start_time_hour = xj_pal_get_int_hour();
			_g_xj_current_bill_info[i].charge_start_time_minute = xj_pal_get_int_minute();
			_g_xj_current_bill_info[i].charge_start_time_sec = xj_pal_get_int_sec();
            xj_pal_print_log(xj_log_remind,"charge started! start to calc bill. start_meter_kwh:%d\n",_g_xj_current_bill_info[i].start_charge_kwh_meter);
		}

		if(TRUE == bill_info.info[i].start_bill_flag)//更新账单
		{
            if(TRUE != bill_info.info[i].start_soc_flag && 0 != g_xj_gun_status[i].soc_percent)
			{
			    _g_xj_current_bill_info[i].start_soc = g_xj_gun_status[i].soc_percent;
			    bill_info.info[i].start_soc_flag = TRUE;
			}
            /* vin code */
            memcpy(&_g_xj_current_bill_info[i].car_vin[0],&g_xj_bms_status[i].brm_vin[0],17);

			bill_info.info[i].int_hour = xj_pal_get_int_hour();
			bill_info.info[i].int_minute = xj_pal_get_int_minute();
			if(bill_info.info[i].last_kwh > 0)
			{
			    /*某一时刻的累计电量 */
				bill_info.info[i].accum_kwh = (uint32_t)(g_xj_gun_status[i].current_meter_kwh_num - bill_info.info[i].last_kwh);//如需实际电表值 = g_xj_gun_status[i].ammeter_data.Energy - bill_info.last_kwh[i];
			}
			bill_info.info[i].last_kwh = g_xj_gun_status[i].current_meter_kwh_num;

			g_xj_gun_status[i].charged_sec = xj_pal_time() - bill_info.info[i].start_charge_sec;

			bill_info.info[i].index = bill_info.info[i].int_hour*2;
			if(bill_info.info[i].int_minute >= 30)
			{
				bill_info.info[i].index += 1;
			}
			memcpy(_g_xj_current_bill_info[i].charge_user_id,g_xj_gun_status[i].charge_user_id,32);
			/* 充电电量 */
			_g_xj_current_bill_info[i].charge_kwh_amount += bill_info.info[i].accum_kwh;
			/* 分时电量 */
			_g_xj_current_bill_info[i].kwh_amount[ bill_info.info[i].index] += bill_info.info[i].accum_kwh;

			bill_info.info[i].charge_fee += bill_info.info[i].accum_kwh * bill_info.info[i].fee_config.charge_fee[ bill_info.info[i].index];
			bill_info.info[i].service_fee += bill_info.info[i].accum_kwh * bill_info.info[i].fee_config.service_fee[ bill_info.info[i].index];
            bill_info.info[i].demurrage += bill_info.info[i].accum_kwh * bill_info.info[i].fee_config.demurrage[ bill_info.info[i].index];
#if 0
            _g_xj_current_bill_info[i].total_charge_fee  = (bill_info.info[i].charge_fee + 500) / 1000;//进行四舍五入  抹零
            _g_xj_current_bill_info[i].total_service_fee = (bill_info.info[i].service_fee + 500) / 1000;//进行四舍五入  抹零
#endif
            _g_xj_current_bill_info[i].total_charge_fee = bank_round(bill_info.info[i].charge_fee);
            _g_xj_current_bill_info[i].total_service_fee = bank_round(bill_info.info[i].service_fee);

			g_xj_gun_status[i].cumulative_charge_fee = bill_info.info[i].charge_fee + bill_info.info[i].service_fee;

			_g_xj_current_bill_info[i].end_charge_kwh_meter = g_xj_gun_status[i].current_meter_kwh_num;

			_g_xj_current_bill_info[i].charge_end_time_year = xj_pal_get_int_year();
			_g_xj_current_bill_info[i].end_soc = g_xj_gun_status[i].soc_percent;
			_g_xj_current_bill_info[i].charge_time = g_xj_gun_status[i].charged_sec;
			_g_xj_current_bill_info[i].start_type = 1;
			_g_xj_current_bill_info[i].charge_end_time_month = xj_pal_get_int_month();
			_g_xj_current_bill_info[i].charge_end_time_day = xj_pal_get_int_day();
			_g_xj_current_bill_info[i].charge_end_time_hour = xj_pal_get_int_hour();
			_g_xj_current_bill_info[i].charge_end_time_minute = xj_pal_get_int_minute();
			_g_xj_current_bill_info[i].charge_end_time_sec = xj_pal_get_int_sec();

			//判断  账单结束。即不在自检、充电中、停止中 即账单结束
			if(xj_work_stat_type_charging != g_xj_gun_status[i].work_stat && xj_work_stat_type_bist != g_xj_gun_status[i].work_stat && xj_work_stat_type_stoping != g_xj_gun_status[i].work_stat)
			{
				memcpy(_g_xj_current_bill_info[i].err_no,g_xj_gun_status[i].stop_reason,XJ_MAX_ERR_NO_LEN -1);
                /*订单启动失败 上传0元账单 start_flags charge_callback赋值*/
                if (g_xj_gun_status[i].start_flags == 1) {
                    memset(_g_xj_current_bill_info[i].kwh_amount,0,sizeof(_g_xj_current_bill_info[i].kwh_amount));
                    _g_xj_current_bill_info[i].charge_kwh_amount = 0;
                    _g_xj_current_bill_info[i].total_charge_fee = 0;
                    _g_xj_current_bill_info[i].total_service_fee = 0;
                    g_xj_gun_status[i].start_flags = 0;
                }

                xj_updata_history_bill(_g_xj_current_bill_info[i],xj_bill_ready_upload);
                xj_pal_print_log(xj_log_message,"bill gun = %d ,charge_fee =%d,service_fee = %d,kwh =%d\n",i,_g_xj_current_bill_info[i].total_charge_fee,_g_xj_current_bill_info[i].total_service_fee,_g_xj_current_bill_info[i].charge_kwh_amount);

                callback_show_bill(_g_xj_current_bill_info[i],i);//新增账单展示
				memset((char*)&_g_xj_current_bill_info[i],0,sizeof(xj_bill_info));
				memset((char*)&bill_info.info[i],00,sizeof(st_calculate_bill));
                //memset((char*)&g_xj_gun_status[i].soc_percent,00,(_OFFSETOF(xj_gun_status,current_meter_kwh_num) - _OFFSETOF(xj_gun_status,soc_percent)));
                memset((char*)&g_xj_gun_status[i].soc_percent,00,(_OFFSETOF(xj_gun_status,user_tel) - _OFFSETOF(xj_gun_status,soc_percent)));
            }
            else if(bill_info.info[i].updata_time >= 3)//3秒更新一次全局账单
			{
			    xj_updata_history_bill(_g_xj_current_bill_info[i],xj_bill_not_upload);
				bill_info.info[i].updata_time = 0;
			}
		}
	}
}

/********************************************************
  * @Description：xj启动充电
  * @Arguments	：
			    param->gun_index: 枪号
			    param->gun_index:订单号
  * @Returns	：
 *******************************************************/
static void xj_start_charge(const xj_start_charge_param* param)
{
    xj_cmd_8 response;
    char err[XJ_MAX_ERR_NO_LEN];

    memset(err,00,XJ_MAX_ERR_NO_LEN);
    memset((char*)&response,00,sizeof(xj_cmd_8));

    if(param->gun_index >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_warning,"gun_index wrong when start charge! index=%d!\n",param->gun_index);
	    strncpy(err,XJ_SDK_ERR_2002,XJ_ERR_RESULT_LEN);
	}
	else if( xj_work_stat_type_bist == g_xj_gun_status[param->gun_index].work_stat || xj_work_stat_type_charging == g_xj_gun_status[param->gun_index].work_stat)
	{
        xj_pal_print_log(xj_log_warning,"Unable to start while charging!gun = %d!\n",param->gun_index);
	    strncpy(err,XJ_SDK_ERR_2000,XJ_ERR_RESULT_LEN);
	}
    else
    {
        g_xj_gun_status[param->gun_index].start_flags = 0;
        callback_start_charge(param->gun_index,err,(char*)param->charge_user_id,strlen(param->charge_user_id),param->user_tel);

        if(4 == Come_letter_num_len(err) && 0 == strncmp(err,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN))
        {
            memcpy(g_xj_gun_status[param->gun_index].charge_user_id,param->charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN);
            g_xj_gun_status[param->gun_index].user_tel = param->user_tel;
            g_xj_bms_status[param->gun_index].work_stat = g_xj_gun_status[param->gun_index].work_stat = xj_work_stat_type_bist;
            _interval_work_for_calculate_bill_info();
        }
        else
        {
            g_xj_bms_status[param->gun_index].work_stat = g_xj_gun_status[param->gun_index].work_stat = xj_work_stat_type_gun_not_pluged_out;
        }
	}

    strncpy(response.result,err,XJ_ERR_RESULT_LEN);
    memcpy(response.equipment_id,(uint8_t*)_g_xj_params->equipment_id,32);
    response.gun_index = param->gun_index + 1;
    memcpy(response.charge_user_id,param->charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1);
    response.serial_code = param->serial_code;

    _xj_mqtt_send_resp_wrap(xj_cmd_type_8,(char*)&response,sizeof(xj_cmd_8),1);
}


/********************************************************
  * @Description：xj停止充电
  * @Arguments	：
  * @Returns	：
 *******************************************************/
static void xj_stop_charge(xj_cmd_11* param)
{
    xj_cmd_12 response;
    char err[XJ_MAX_ERR_NO_LEN];
    uint8_t gun_cnt = 0;


    memset((char*)&response,0,sizeof(xj_cmd_12));
    memset(err,00,XJ_MAX_ERR_NO_LEN);
    gun_cnt = param->gun_index -1;

    memcpy(response.equipment_id,_g_xj_params->equipment_id,sizeof(response.equipment_id));
    response.serial_code = param->serial_code;
    response.gun_index = gun_cnt + 1;
		memcpy(response.charge_seq,param->charge_seq,sizeof(response.charge_seq));
	if(gun_cnt >= g_xj_params.gun_cnt)
	{
        xj_pal_print_log(xj_log_warning,"gun_index wrong in stop order cmd , gun=%d!\n",gun_cnt);
        strncpy(err,XJ_SDK_ERR_2002,XJ_MAX_ERR_NO_LEN-1);
	}
	else if(xj_work_stat_type_charging != g_xj_gun_status[gun_cnt].work_stat &&
	    xj_work_stat_type_bist != g_xj_gun_status[gun_cnt].work_stat)
	{
        xj_pal_print_log(xj_log_warning,"work_stat wrong in stop order cmd , stat=%d!\n",g_xj_gun_status[gun_cnt].work_stat);
        strncpy(err,XJ_SDK_ERR_OTHER,XJ_MAX_ERR_NO_LEN-1);
	}
	else if(0 != strncmp(g_xj_gun_status[gun_cnt].charge_user_id,param->charge_seq,XJ_MAX_CHAEGE_USER_ID_LEN -1))
	{
        xj_pal_print_log(xj_log_warning,"Order ID wrong in stop order cmd , ID=%s!\n",g_xj_gun_status[gun_cnt].charge_user_id);
        strncpy(err,XJ_SDK_ERR_3021,XJ_MAX_ERR_NO_LEN-1);
	}
	else
	{
	    g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat = xj_work_stat_type_stoping;
        callback_stop_charge(gun_cnt,err);
       // if(0 == strncmp(err,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN))
        {
           // g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat = xj_work_stat_type_stoping;
        }
	}

    memcpy(response.result, err, 4);
	_xj_mqtt_send_resp_wrap(xj_cmd_type_12,(char*)&response,sizeof(xj_cmd_12),1);
}

/********************************************************
  * @Description：xj设置枪二维码  - 根据CODE3命令
  * @Arguments	：
                gun_cnt:设置的枪号，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
                qr_str：二维码指针
                len：长度
  * @Returns	：
 *******************************************************/
static void xj_set_gun_qr(uint8_t gun_cnt,uint8_t*qr_str,uint16_t len)
{
    char err[XJ_MAX_ERR_NO_LEN];
    xj_cmd_4 response;

    memset(err,00,XJ_MAX_ERR_NO_LEN);
    memset((char*)&response,00,sizeof(xj_cmd_4));

    memcpy(response.equipment_id,_g_xj_params->equipment_id,32);
    response.cmd_type = 1;
    response.dataLen = len;
    memcpy(response.data,qr_str,len);
    response.result = 0xFF;//初始失败

    if(gun_cnt < g_xj_params.gun_cnt)
    {
        callback_display_qr(gun_cnt,qr_str,len,err);
        if(0 == strncmp(err,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN))
        {
            response.result = 0;
        }

        if(0 == gun_cnt)
        {
            response.start_addr = 17;
        }
        else if(1 == gun_cnt)
        {
            response.start_addr = 19;
        }
    }
    _xj_mqtt_send_resp_wrap(xj_cmd_type_4,(char*)&response,sizeof(xj_cmd_4),1);
}


/********************************************************
  * @Description：xj设置系统时间- 根据CODE3命令
  * @Arguments	：
                time：时间结构体指针

  * @Returns	：
 *******************************************************/
static void xj_set_sys_time(xj_sync_system_time_param* time)
{
    char err[XJ_MAX_ERR_NO_LEN];
//  xj_cmd_4 response;

    memset(err,00,XJ_MAX_ERR_NO_LEN);
   // memset((char*)&response,00,sizeof(xj_cmd_4));
    callback_set_sys_time(*time,err);
/*
    response.result = (0 == strncmp(err,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN))? (0):(0xFF);
    memcpy(response.equipment_id,_g_xj_params->equipment_id,32);
    response.cmd_type = 1;
    response.start_addr = 2;
    response.dataLen = 8;
    _convert_time(xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec(),(char*)&response.data);
    _xj_mqtt_send_resp_wrap(xj_cmd_type_4,(char*)&response,sizeof(xj_cmd_4),1);
*/
}

/********************************************************
  * @Description：xj下载升级软件- 根据CODE1101命令 异步执行 回调需要异步执行下载 下载完成后调用touch_send_1102_code
  * @Arguments	：
                param->upgrade_type 程序类型
                param->target_type 目标类型
                param->url：供下载的http url 字符串         最大长度128
                param->checksum:文件的MD5校验码   最大长度32
                param->serial_code:code 序列号域

  * @Returns	：
 *******************************************************/
static void xj_software_download(xj_software_upgrade_param *param )
{
    callback_software_download(param);
}

/********************************************************
  * @Description：xj上传设备日志
  * @Arguments	：
                url：http post的地址
  * @Returns	：
                无
 *******************************************************/
static void xj_upload_log(xj_upload_log_param* url)
{
    xj_cmd_410  response;

    memset((char*)&response,00,sizeof(xj_cmd_410));
    if(0 == callback_upload_log(url,(char*)response.log_name,128))
    {
        strncpy((char*)response.equipment_id,_g_xj_params->equipment_id,XJ_MAX_EQUIPMENT_ID_LEN-1);
        _xj_mqtt_send_resp_wrap(xj_cmd_type_410,(char*)&response,sizeof(xj_cmd_410),0);
    }
    else
    {
        xj_pal_print_log(xj_log_warning,"Log upload failed!\n");
    }

}

/********************************************************
  * @Description：xj-code5 控制命令解析
  * @Arguments	：
                param：code5 参数
  * @Returns	：
                无
 *******************************************************/
static void xj_code5_control_handle(xj_control_cmd_param* param)
{
    xj_cmd_6 response;
    char err[XJ_MAX_ERR_NO_LEN];
    uint8_t incident_flag = FALSE;//发送完成后 执行事件

    memset(err,00,XJ_MAX_ERR_NO_LEN);
    memset((char*)&response,0,sizeof(xj_cmd_6));

    memcpy(response.equipment_id,_g_xj_params->equipment_id,XJ_MAX_EQUIPMENT_ID_LEN-1);
    response.gun_index = param->gun_index + 1;
    response.addr = param->cmd_type;
    response.cmd_num = param->cmd_count;
    response.serial_code = param->serial_code;

    if(param->gun_index < g_xj_params.gun_cnt)
    {
        switch((xj_control_cmd_type)param->cmd_type)
        {
            case xj_control_cmd_type_stop_charge:
            {
                if(xj_work_stat_type_charging == g_xj_gun_status[param->gun_index].work_stat ||
                    xj_work_stat_type_bist == g_xj_gun_status[param->gun_index].work_stat)
                {
                    g_xj_bms_status[param->gun_index].work_stat = g_xj_gun_status[param->gun_index].work_stat = xj_work_stat_type_stoping;
                    callback_stop_charge(param->gun_index,err);
                    memcpy(response.result, err, XJ_ERR_RESULT_LEN);
                }
                else
                {
                    xj_pal_print_log(xj_log_remind,"No current charging,The gun=%d is currently stopped!\n",param->gun_index);
                    strncpy(response.result, XJ_SUCCEED_RESULT, XJ_ERR_RESULT_LEN);
                }
                break;
            }
            case xj_control_cmd_type_reboot:
            {
                //xj+++ 待后续实现 预留 目前返回失败
                strncpy(response.result, XJ_SDK_ERR_OTHER, XJ_ERR_RESULT_LEN);
                break;
            }
            case xj_control_cmd_type_reset_factory_settings:
            {
                //xj+++ 待后续实现 预留 目前返回失败
                strncpy(response.result, XJ_SDK_ERR_OTHER, XJ_ERR_RESULT_LEN);
                break;
            }
            case xj_control_cmd_type_gun_unlock:
            {
                //xj+++ 待后续实现 预留 目前返回失败
                strncpy(response.result, XJ_SDK_ERR_OTHER, XJ_ERR_RESULT_LEN);
                break;
            }
            case xj_control_cmd_type_reconnect://重新连接
            {
                uint8_t i = 0;
                uint8_t stat_idle_flag = FALSE;

                //for(i = 0;i < CHARGE_GUN_CNT;i++)
                for(i = 0;i < g_xj_params.gun_cnt;i++)
                {
                    if(xj_work_stat_type_idle != g_xj_gun_status[i].work_stat)
                    {
                        memcpy(response.result, XJ_SDK_ERR_2000, XJ_ERR_RESULT_LEN);
                        stat_idle_flag = TRUE;
                    }
                }
                if(FALSE == stat_idle_flag)
                {
                    memcpy(response.result, XJ_SUCCEED_RESULT, XJ_ERR_RESULT_LEN);
                    incident_flag = TRUE;
                }
                break;
            }
            default:
            {
                strncpy(response.result, XJ_SDK_ERR_OTHER, XJ_ERR_RESULT_LEN);
                break;
            }
        }
    }
    else
    {
        strncpy(response.result, XJ_SDK_ERR_OTHER, XJ_ERR_RESULT_LEN);
        xj_pal_print_log(xj_log_warning,"gun_index wrong in code5 , gun=%d!\n",param->gun_index);
    }
    _xj_mqtt_send_resp_wrap(xj_cmd_type_6,(char*)&response,sizeof(xj_cmd_6),1);

    if(TRUE == incident_flag)
    {
        //不需要等待CODE 6发送完毕，重连前会等待
        switch((xj_control_cmd_type)param->cmd_type)
        {
            case xj_control_cmd_type_reconnect:
            {
                xj_add_incident_handle(&xj_incident_info,xj_incident_type_reconnection,NULL,0);
                break;
            }
        }
    }
}

/********************************************************
  * @Description：xj-code23 电子锁控制
  * @Arguments	：
                param：code23 参数
  * @Returns	：
                无
 *******************************************************/
static void xj_code23_lock_control(xj_cmd_23* param)
{
    char err = CODE23_NO_ERROR;
    uint8_t gun_cnt = 0;
    xj_cmd_24 response;

    gun_cnt = param->gun_index-1;
    memset((char*)&response,0,sizeof(xj_cmd_24));
    memcpy((char*)&response.equipment_id,(char*)&g_xj_params.equipment_id,sizeof(response.equipment_id));
    response.gun_index = param->gun_index;
    response.serial_code = param->serial_code;

    if(gun_cnt >= g_xj_params.gun_cnt)
    {
        xj_pal_print_log(xj_log_warning,"gun_index wrong in code 23 , gun=%d!\n",gun_cnt);
        err =  CODE23_GUN_ERROR;
    }
    else if(0 != strncmp((char*)&g_xj_params.equipment_id,(char*)&param->equipment_id,sizeof(param->equipment_id) ))
    {
        xj_pal_print_log(xj_log_warning,"equipment_id wrong in code 23 !\n");
        err =  CODE23_ID_ERROR;
    }
    else if(xj_work_stat_type_charging == g_xj_gun_status[gun_cnt].work_stat)
    {
        xj_pal_print_log(xj_log_warning,"The current gun state is not supported in code 23 !\n");
        err =  CODE23_STATUS_ERROR;
    }
    else if(0 != param->lock_type && 1 != param->lock_type )
    {
        xj_pal_print_log(xj_log_warning,"lock_type wrong in code 23 !\n");
        err =  CODE23_OTHER_ERROR;
    }
    else
    {
        err = callback_lock_control(gun_cnt,param->lock_type);
    }
    response.result = err;
	_xj_mqtt_send_resp_wrap(xj_cmd_type_24,(char*)&response,sizeof(xj_cmd_24),1);
}





/********************************************************
  * @Description：初始化xj参数
  * @Arguments	：
			    p  xj参数指针
  * @Returns	：

 *******************************************************/
void xj_helper_init_xj_params(xj_params* p,uint8_t gun_cnt)
{
    int ver = 0;
    uint8_t i = 0;
	memset(p, 0, sizeof(xj_params));
	_restore_persisted_xj_params(p);
	if(!(p->sign_in_interval > 0 && p->sign_in_interval < 60))
	{
		p->sign_in_interval = 30;
	}
	if(!(p->network_cmd_timeout > 0 && p->network_cmd_timeout < 10))
	{
		p->network_cmd_timeout = 5;
	}
	if(!(p->upload_gun_status_interval > 0 && p->upload_gun_status_interval < 60))
	{
		p->upload_gun_status_interval = 30;
	}
	if(!(p->upload_bms_status_interval > 0 && p->upload_bms_status_interval < 60))
	{
		p->upload_bms_status_interval = 30;
	}
	if(!(p->heartbeat_interval > 0 && p->heartbeat_interval < 45))
	{
		p->heartbeat_interval = 30;
	}

	if(!(p->heartbeat_timeout_check_cnt > 0 && p->heartbeat_timeout_check_cnt < 5))
	{
		p->heartbeat_timeout_check_cnt = 3;
	}
	if(!(p->logic_svr_upload_log_port > 0))
	{
	    p->logic_svr_upload_log_port = 8089;
	}
	if(!(0 != p->gun_type))
	{
	    p->gun_type = XJ_GUN_TYPE;
	}
	if(!(Come_letter_num_len((uint8_t*)p->equipment_id) > 5))
	{
	    strncpy(p->equipment_id,XJ_EQUIPMENT_ID,XJ_MAX_EQUIPMENT_ID_LEN-1);
	}
	if(!(Come_letter_num_len((uint8_t*)p->center_svr_addr) > 0))
	{
	    strncpy(p->center_svr_addr,XJ_CENTER_SVR_ADDR,XJ_MAX_SVR_ADDR_LEN-1);
	}
	if(!(p->center_svr_port > 0))
	{
    	p->center_svr_port = XJ_CENTER_SVR_PORT;
	}
	//if(!(Come_letter_num_len((uint8_t*)p->logic_svr_addr) > 0))
	if(0 != strncmp((uint8_t*)p->logic_svr_addr,XJ_LOGIC_SVR_ADDR,XJ_MAX_SVR_ADDR_LEN-1))
	{
	    strncpy(p->logic_svr_addr,XJ_LOGIC_SVR_ADDR,XJ_MAX_SVR_ADDR_LEN-1);
	}
	//if(!(p->logic_svr_port > 0))
	if(XJ_LOGIC_SVR_PORT != p->logic_svr_port)
	{
    	p->logic_svr_port = XJ_LOGIC_SVR_PORT;
	}
	if(!(Come_letter_num_len((uint8_t*)p->username) > 10))
	{
	    strncpy(p->username,XJ_MQTT_USERNAME,XJ_MAX_USERNAME_LEN-1);
	}
	if(!(Come_letter_num_len((uint8_t*)p->password) > 10))
	{
	    strncpy(p->password,XJ_MQTT_PASSWORD,XJ_MAX_PASSWORD_LEN-1);
	}
	if(!(p->gun_cnt > 0))
	{
	    //p->gun_cnt = CHARGE_GUN_CNT;
	    p->gun_cnt = (gun_cnt > CHARGE_GUN_CNT_MAX) ? CHARGE_GUN_CNT_DEFAULT : gun_cnt;
	}
	else if(p->gun_cnt > CHARGE_GUN_CNT_MAX)
    {
        p->gun_cnt = CHARGE_GUN_CNT_DEFAULT;
    }
	if(!(Come_letter_num_len((uint8_t*)p->mac_addr) > 0))
	{
        strncpy(p->mac_addr,XJ_MAC_ADDR,XJ_MAX_MAC_ADDR_LEN);
	}
	if(!(p->mqtt_ping_interval >= 1 && p->mqtt_ping_interval <= 30))
	{
	    p->mqtt_ping_interval = 5;
	}
	//for(i = 0;i < CHARGE_GUN_CNT;i++)
	for(i = 0;i < p->gun_cnt;i++)
	{
	    if('\0' == p->gun_qr_code[i][0])
	    {
	        QR_code_String(&p->gun_qr_code[i][0],g_xj_params.equipment_id,i+1);
	    }
	}
	ver = XJ_PAL_HTONL(USER_VERSION);
	memcpy((char*)p->user_version,(char*)&ver,4);
	if(!(LOG_LEVEL_NULL <= p->log_level && LOG_LEVEL_ERROR >= p->log_level))
	{
	    p->log_level = LOG_LEVEL_NULL;
	}
	if(!(LOG_STRATEGY_TIME <= p->log_strategy && LOG_STRATEGY_NULL >= p->log_strategy))
	{
	    p->log_strategy = LOG_STRATEGY_NULL;
	}

	p->software_restart_cnt++;
}

/********************************************************
  * @Description：充电枪和bms 状态初始化
  * @Arguments	：
			    充电枪个数
  * @Returns	：

 ********************************************************/
static void xj_gun_bms_status_init(uint8_t gun_cnt)
{
    int i = 0;

    memset((char*)&g_xj_gun_status,00,sizeof(g_xj_gun_status));
    memset((char*)&g_xj_bms_status,00,sizeof(g_xj_bms_status));

    for(i=0;i<gun_cnt;i++)
    {
        g_xj_bms_status[i].work_stat =g_xj_gun_status[i].work_stat = xj_work_stat_type_idle;
        g_xj_gun_status[i].gun_index = i;
        g_xj_bms_status[i].gun_index = i;
    }
}


static int8_t xj_init(xj_params* params,st_user_gun_info* gun_status, st_user_bms_info* bms_status)
{
	if(params==NULL||gun_status==NULL||bms_status==NULL)
	{
		return -1;
	}
	_g_xj_params= params;
	p_User_gun_info = gun_status;
	p_User_bms_info = bms_status;
	memset(_g_mqtt_resp_to_send_queue,0,sizeof(_g_mqtt_resp_to_send_queue));
	return 0;
}



/*
* 用于充电桩登录服务器
*/

xj_login_err_no xj_login(void)
{
    xj_pal_print_log(xj_log_message,"start to login!\n");
	_g_is_login = 0; // 连接成功全局标志，不要修改，SDK会使用该标志确定是否登录成功
	// 登录中心服务器
	/*
	if(xj_mqtt_connect(_g_xj_params->center_svr_addr,_g_xj_params->center_svr_port,_g_xj_params->username,_g_xj_params->password,_g_xj_params->equipment_id,_on_cmd_received_callback)==0)
	{
         xj_pal_print_log(xj_log_message,"failed to connect to center server!\n");
		return xj_login_err_no_failed_to_connect_center_svr;
	}

	xj_cmd_114 cmd_114;
	memset(&cmd_114,0,sizeof(xj_cmd_114));
	// 获取逻辑服务器地址
	xj_cmd_113* cmd_113 = xj_mqtt_send_cmd(xj_cmd_type_114,&cmd_114,_g_xj_params->network_cmd_timeout,1);

	xj_mqtt_disconnect();

	if(cmd_113==NULL)
	{
        xj_pal_print_log(xj_log_remind,"login failed!failed to get logic server address!\n");
		return xj_login_err_no_failed_to_get_logic_svr_addr;
	}
    xj_pal_print_log(xj_log_message,"logic server address retrieved, disconnect from center server!\n");

    xj_pal_print_log(xj_log_message,"logic url:%s,p:%d\n",cmd_113->url,cmd_113->port);

    if((0 != strncmp(_g_xj_params->logic_svr_addr,cmd_113->url,XJ_MAX_SVR_ADDR_LEN-1)) || (_g_xj_params->logic_svr_port != cmd_113->port))
    {
        strncmp(_g_xj_params->logic_svr_addr,cmd_113->url,XJ_MAX_SVR_ADDR_LEN-1);
	    _g_xj_params->logic_svr_port = cmd_113->port;
        _persist_xj_params();
    }
    */

	// 登录逻辑服务器
	if(xj_mqtt_connect(_g_xj_params->logic_svr_addr,_g_xj_params->logic_svr_port,_g_xj_params->username,_g_xj_params->password,_g_xj_params->equipment_id,_on_cmd_received_callback)==0)
	{
        xj_pal_print_log(xj_log_warning,"failed to connect to logic svr!\n");
		return xj_login_err_no_failed_to_connect_logic_svr;
	}

	_g_is_login = 1;

	return xj_login_err_no_succeed;
}

/********************************************************
  * @Description：小桔首次登陆系统
  * @Arguments	：
  * @Returns	：
 ********************************************************/
void xj_first_login(void)
{
     /*variables declared here to conform to c89 standard.*/
    xj_login_err_no login_ret;

    do{
        xj_pal_print_log(xj_log_message,"xj first login.\n");
        login_ret = xj_login();
        if(login_ret!=xj_login_err_no_succeed)
        {
            xj_pal_print_log(xj_log_remind,"login failed! retry in 5 seconds.ret=%d\n",login_ret);
        }
        else
        {
            break;
        }
        xj_pal_msleep(5*1000);
    }
    while(login_ret!=xj_login_err_no_succeed);
    xj_pal_print_log(xj_log_message,"login xiaoju succeeded!\n");
}



//开机查询历史账单
static void xj_read_history_bill(void)
{
    uint8_t i = 0;

   if(0 == callback_read_bill((char*)&xj_histroy_bill_info,sizeof(xj_histroy_bill_info)))
   {
        for(i = 0;i < MAX_HISTORY_ORDER_CNT;i++)
        {
            if(xj_bill_not_upload == xj_histroy_bill_info[i].status)
            {
                xj_histroy_bill_info[i].status = xj_bill_ready_upload;
                xj_pal_print_log(xj_log_remind,"find not upload histroy bill!\n");
            }
        }
   }
   else
   {
        xj_pal_print_log(xj_log_Error,"Read history failure!\n");
   }


}



_mqtt_resp_to_send* _poll_mqtt_resp_to_send()
{
	int i=0;
	for(i=0;i<MAX_BLOCKED_MQTT_RESP_NUM;i++)
	{
		if(_g_mqtt_resp_to_send_queue[i].submited==1)
		{
			return &(_g_mqtt_resp_to_send_queue[i]);
		}
	}
	return NULL;
}


void _build_xj_cmd_202(xj_cmd_202* cmd_202,xj_bill_info* info){
	memset(cmd_202,0,sizeof(xj_cmd_202));
	strncpy(cmd_202->equipment_id,_g_xj_params->equipment_id,32);
	cmd_202->gun_type = _g_xj_params->gun_type;
	cmd_202->gun_index = info->gun_index+1;
	strncpy(cmd_202->charge_user_id,info->charge_user_id,32);
	char time[8];
	_convert_time(info->charge_start_time_year,info->charge_start_time_month,info->charge_start_time_day,info->charge_start_time_hour,info->charge_start_time_minute,info->charge_start_time_sec,time);
	memcpy(cmd_202->charge_start_time,time,8);
	_convert_time(info->charge_end_time_year,info->charge_end_time_month,info->charge_end_time_day,info->charge_end_time_hour,info->charge_end_time_minute,info->charge_end_time_sec,time);
	memcpy(cmd_202->charge_end_time,time,8);
	cmd_202->charge_time = info->charge_time;
	cmd_202->start_soc = info->start_soc;
	cmd_202->end_soc = info->end_soc;
	strncpy(cmd_202->err_no,info->err_no,4);
	cmd_202->charge_kwh_amount = info->charge_kwh_amount;
	cmd_202->start_charge_kwh_meter = info->start_charge_kwh_meter;
	cmd_202->end_charge_kwh_meter = info->end_charge_kwh_meter;
	cmd_202->total_charge_fee = info->total_charge_fee;
	cmd_202->is_not_stoped_by_card = 0;
	cmd_202->start_card_money = 0;
	cmd_202->end_card_money = 0;
	cmd_202->total_service_fee = info->total_service_fee;
	cmd_202->is_paid_by_offline = 0;
	cmd_202->charge_policy = 0;
	cmd_202->charge_policy_param = 0;
	strncpy(cmd_202->car_vin,info->car_vin,17);
	/*strncpy(cmd_202->car_plate_no,info->car_plate_no,8);*/
	cmd_202->kwh_amount_1 = info->kwh_amount[0];
	cmd_202->kwh_amount_2 = info->kwh_amount[1];
	cmd_202->kwh_amount_3 = info->kwh_amount[2];
	cmd_202->kwh_amount_4 = info->kwh_amount[3];
	cmd_202->kwh_amount_5 = info->kwh_amount[4];
	cmd_202->kwh_amount_6 = info->kwh_amount[5];
	cmd_202->kwh_amount_7 = info->kwh_amount[6];
	cmd_202->kwh_amount_8 = info->kwh_amount[7];
	cmd_202->kwh_amount_9 = info->kwh_amount[8];
	cmd_202->kwh_amount_10 = info->kwh_amount[9];
	cmd_202->kwh_amount_11 = info->kwh_amount[10];
	cmd_202->kwh_amount_12 = info->kwh_amount[11];
	cmd_202->kwh_amount_13 = info->kwh_amount[12];
	cmd_202->kwh_amount_14 = info->kwh_amount[13];
	cmd_202->kwh_amount_15 = info->kwh_amount[14];
	cmd_202->kwh_amount_16 = info->kwh_amount[15];
	cmd_202->kwh_amount_17 = info->kwh_amount[16];
	cmd_202->kwh_amount_18 = info->kwh_amount[17];
	cmd_202->kwh_amount_19 = info->kwh_amount[18];
	cmd_202->kwh_amount_20 = info->kwh_amount[19];
	cmd_202->kwh_amount_21 = info->kwh_amount[20];
	cmd_202->kwh_amount_22 = info->kwh_amount[21];
	cmd_202->kwh_amount_23 = info->kwh_amount[22];
	cmd_202->kwh_amount_24 = info->kwh_amount[23];
	cmd_202->kwh_amount_25 = info->kwh_amount[24];
	cmd_202->kwh_amount_26 = info->kwh_amount[25];
	cmd_202->kwh_amount_27 = info->kwh_amount[26];
	cmd_202->kwh_amount_28 = info->kwh_amount[27];
	cmd_202->kwh_amount_29 = info->kwh_amount[28];
	cmd_202->kwh_amount_30 = info->kwh_amount[29];
	cmd_202->kwh_amount_31 = info->kwh_amount[30];
	cmd_202->kwh_amount_32 = info->kwh_amount[31];
	cmd_202->kwh_amount_33 = info->kwh_amount[32];
	cmd_202->kwh_amount_34 = info->kwh_amount[33];
	cmd_202->kwh_amount_35 = info->kwh_amount[34];
	cmd_202->kwh_amount_36 = info->kwh_amount[35];
	cmd_202->kwh_amount_37 = info->kwh_amount[36];
	cmd_202->kwh_amount_38 = info->kwh_amount[37];
	cmd_202->kwh_amount_39 = info->kwh_amount[38];
	cmd_202->kwh_amount_40 = info->kwh_amount[39];
	cmd_202->kwh_amount_41 = info->kwh_amount[40];
	cmd_202->kwh_amount_42 = info->kwh_amount[41];
	cmd_202->kwh_amount_43 = info->kwh_amount[42];
	cmd_202->kwh_amount_44 = info->kwh_amount[43];
	cmd_202->kwh_amount_45 = info->kwh_amount[44];
	cmd_202->kwh_amount_46 = info->kwh_amount[45];
	cmd_202->kwh_amount_47 = info->kwh_amount[46];
	cmd_202->kwh_amount_48 = info->kwh_amount[47];
	cmd_202->start_charge_type = info->start_type;
	cmd_202->serial_code = ++mqttMessageBase.masterSerialNum;
}

void _build_xj_cmd_104(xj_cmd_104* cmd_104,xj_gun_status* status){
	memset(cmd_104,0,sizeof(xj_cmd_104));
	strncpy(cmd_104->equipment_id,_g_xj_params->equipment_id,32);
	cmd_104->gun_cnt = _g_xj_params->gun_cnt;
	cmd_104->gun_index = status->gun_index+1;
	cmd_104->gun_type = _g_xj_params->gun_type;
	cmd_104->work_stat = status->work_stat;
	cmd_104->soc_percent = status->soc_percent;
	cmd_104->car_connection_stat = status->car_connection_stat;
	cmd_104->cumulative_charge_fee = status->cumulative_charge_fee;
	cmd_104->dc_charge_voltage = status->dc_charge_voltage;
	cmd_104->dc_charge_current = status->dc_charge_current;
	cmd_104->bms_need_voltage = status->bms_need_voltage;
	cmd_104->bms_need_current = status->bms_need_current;
	cmd_104->bms_charge_mode = 0;
	cmd_104->ac_a_vol = status->ac_a_vol;
	cmd_104->ac_b_vol = status->ac_b_vol;
	cmd_104->ac_c_vol = status->ac_c_vol;
	cmd_104->ac_a_cur = status->ac_a_cur;
	cmd_104->ac_b_cur = status->ac_b_cur;
	cmd_104->ac_c_cur = status->ac_c_cur;
	cmd_104->charge_full_time_left = 0;
	cmd_104->charged_sec = status->charged_sec;
	cmd_104->before_charge_meter_kwh_num = status->before_charge_meter_kwh_num;
	cmd_104->now_meter_kwh_num = status->current_meter_kwh_num;

	if(xj_work_stat_type_charging == status->work_stat || xj_work_stat_type_stoping == status->work_stat)
	{
	    cmd_104->cum_charge_kwh_amount = (int32_t)(status->current_meter_kwh_num - status->before_charge_meter_kwh_num);
	}
	else
	{
	    cmd_104->cum_charge_kwh_amount = 0; /*累计充电电量 */
	}
	cmd_104->start_charge_type = 1;
	cmd_104->charge_policy = 0;
	cmd_104->charge_policy_param = 0;
	/*cmd_104->book_flag = status->book_flag;*/
	memcpy(cmd_104->charge_user_id,status->charge_user_id,32);
	/*
 	cmd_104->book_timeout_min = status->book_timeout_min;
	memcpy(cmd_104->book_start_charge_time,status->book_start_charge_time,8);
	cmd_104->before_charge_card_account = status->before_charge_card_account;
	*/
	cmd_104->charge_power_kw = status->charge_power_kw;
}

void _build_xj_cmd_302(xj_cmd_302* cmd_302,xj_bms_status* status)
{
	memset(cmd_302,0,sizeof(xj_cmd_302));
	cmd_302->gun_index = status->gun_index+1;
	strncpy(cmd_302->equipment_id,_g_xj_params->equipment_id,32);
	cmd_302->work_stat = status->work_stat;
	cmd_302->car_connect_stat = status->car_connection_stat;
	memcpy(cmd_302->brm_bms_connect_version,status->brm_bms_connect_version,3);
	cmd_302->brm_battery_type = status->brm_battery_type;
	cmd_302->brm_battery_power = status->brm_battery_power;
	cmd_302->brm_battery_voltage = status->brm_battery_volt;
	cmd_302->brm_battery_supplier = status->brm_battery_supplier;
	cmd_302->brm_battery_seq = status->brm_battery_seq;
	cmd_302->brm_battery_produce_year = status->brm_battery_produce_year;
	cmd_302->brm_battery_produce_month = status->brm_battery_produce_month;
	cmd_302->brm_battery_produce_day = status->brm_battery_produce_day;
	cmd_302->brm_battery_charge_count = status->brm_battery_charge_count;
	cmd_302->brm_battery_property_identification = status->brm_battery_property_identification;
	memcpy(cmd_302->brm_vin,status->brm_vin,17);
	memcpy(cmd_302->brm_BMS_version,status->brm_bms_software_version,8);
	cmd_302->bcp_max_voltage = status->bcp_max_voltage;
	cmd_302->bcp_max_current = status->bcp_max_current;
	cmd_302->bcp_max_power = status->bcp_max_power;
	cmd_302->bcp_total_voltage = status->bcp_total_voltage;
	cmd_302->bcp_max_temperature = status->bcp_max_temperature;
	cmd_302->bcp_battery_soc = status->bcp_battery_soc;
	cmd_302->bcp_battery_soc_current_voltage = status->bcp_battery_soc_current_voltage;
	cmd_302->bcl_voltage_need = status->bcl_voltage_need;
	cmd_302->bcl_current_need = status->bcl_current_need;
	cmd_302->bcl_charge_mode = status->bcl_charge_mode;
	cmd_302->bcs_test_voltage = status->bcs_test_voltage;
	cmd_302->bcs_test_current = status->bcs_test_current;
	cmd_302->bcs_max_single_voltage = status->bcs_max_single_voltage;
	cmd_302->bcs_max_single_no = status->bcs_max_single_no;
	cmd_302->bcs_current_soc = status->bcs_current_soc;
	cmd_302->last_charge_time = status->charge_time_left;
	cmd_302->bsm_single_no = status->bsm_single_no;
	cmd_302->bsm_max_temperature = status->bsm_max_temperature;
	cmd_302->bsm_max_temperature_check_no = status->bsm_max_temperature_check_no;
	cmd_302->bsm_min_temperature = status->bsm_min_temperature;
	cmd_302->bsm_min_temperature_check_no = status->bsm_min_temperature_check_no;
	cmd_302->bsm_voltage_too_high_or_too_low = status->bsm_voltage_too_high_or_too_low;
	cmd_302->bsm_car_battery_soc_too_high_or_too_low = status->bsm_car_battery_soc_too_high_or_too_low;
	cmd_302->bsm_car_battery_charge_over_current = status->bsm_car_battery_charge_over_current;
	cmd_302->bsm_battery_temperature_too_high = status->bsm_battery_temperature_too_high;
	cmd_302->bsm_battery_insulation_state = status->bsm_battery_insulation_state;
	cmd_302->bsm_battery_connect_state = status->bsm_battery_connect_state;
	cmd_302->bsm_allow_charge = status->bsm_allow_charge;
	cmd_302->bst_BMS_soc_target = status->bst_bms_soc_target;
	cmd_302->bst_BMS_voltage_target = status->bst_bms_voltage_target;
	cmd_302->bst_single_voltage_target = status->bst_single_voltage_target;
	cmd_302->bst_finish = status->bst_finish;
	cmd_302->bst_isolation_error = status->bst_isolation_error;
	cmd_302->bst_connect_over_temperature = status->bst_connect_over_temperature;
	cmd_302->bst_BMS_over_temperature = status->bst_over_temperature;
	cmd_302->bst_connect_error = status->bst_connect_error;
	cmd_302->bst_battery_over_temperature = status->bst_battery_over_temperature;
	cmd_302->bst_high_voltage_relay_error = status->bst_high_voltage_relay_error;
	cmd_302->bst_point2_test_error = status->bst_point2_test_error;
	cmd_302->bst_other_error = status->bst_other_error;
	cmd_302->bst_current_too_high = status->bst_current_too_high;
	cmd_302->bst_voltage_too_high = status->bst_voltage_too_high;
	cmd_302->bst_stop_soc = status->bst_stop_soc;
	cmd_302->bsd_battery_low_voltage = status->bsd_battery_low_voltage;
	cmd_302->bsd_battery_high_voltage = status->bsd_battery_high_voltage;
	cmd_302->bsd_battery_low_temperature = status->bsd_battery_low_temperature;
	cmd_302->bsd_battery_high_temperature = status->bsd_battery_high_temperature;
	cmd_302->error_68 = status->bem_error_spn2560_00;
	cmd_302->error_69 = status->bem_error_spn2560_aa;
	cmd_302->error_70 = status->bem_error_time_sync;
	cmd_302->error_71 = status->bem_error_ready_to_charge;
	cmd_302->error_72 = status->bem_error_receive_status;
	cmd_302->error_73 = status->bem_error_receive_stop_charge;
	cmd_302->error_74 = status->bem_error_receive_report;
	cmd_302->error_75 = status->bem_error_other;
}



void _build_xj_cmd_304(xj_cmd_304* cmd_304,xj_bms_status* status,uint8_t*charge_user_id,uint32_t serial_code)
{
    cmd_304->serial_code = serial_code;
	cmd_304->gun_index = status->gun_index+1;
	strncpy(cmd_304->equipment_id,_g_xj_params->equipment_id,32);
	strncpy(cmd_304->charge_user_id,charge_user_id,32);
	cmd_304->work_stat = status->work_stat;

	memcpy(cmd_304->brm_bms_connect_version,status->brm_bms_connect_version,3);
	cmd_304->brm_battery_type = status->brm_battery_type;
	cmd_304->brm_battery_power = status->brm_battery_power;
	cmd_304->brm_battery_voltage = status->brm_battery_volt;
	cmd_304->brm_battery_supplier = status->brm_battery_supplier;
	cmd_304->brm_battery_seq = status->brm_battery_seq;
	cmd_304->brm_battery_produce_year = status->brm_battery_produce_year;
	cmd_304->brm_battery_produce_month = status->brm_battery_produce_month;
	cmd_304->brm_battery_produce_day = status->brm_battery_produce_day;
	cmd_304->brm_battery_charge_count = status->brm_battery_charge_count;
	cmd_304->brm_battery_property_identification = status->brm_battery_property_identification;
	memcpy(cmd_304->brm_vin,status->brm_vin,17);
	memcpy(cmd_304->brm_BMS_version,status->brm_bms_software_version,8);
	cmd_304->bcp_max_voltage = status->bcp_max_voltage;
	cmd_304->bcp_max_current = status->bcp_max_current;
	cmd_304->bcp_max_power = status->bcp_max_power;
	cmd_304->bcp_total_voltage = status->bcp_total_voltage;
	cmd_304->bcp_max_temperature = status->bcp_max_temperature;
	cmd_304->bcp_battery_soc = status->bcp_battery_soc;
	cmd_304->bcp_battery_soc_current_voltage = status->bcp_battery_soc_current_voltage;
	cmd_304->bro_BMS_isReady = status->bro_BMS_isReady;
	cmd_304->CRO_isReady = status->CRO_isReady;
}


void _build_xj_cmd_306(xj_cmd_306* cmd_306,xj_bms_status* status,uint8_t*charge_user_id,uint32_t serial_code)
{
    cmd_306->serial_code = serial_code;
	cmd_306->gun_index = status->gun_index+1;
	strncpy(cmd_306->equipment_id,_g_xj_params->equipment_id,32);
	strncpy(cmd_306->charge_user_id,charge_user_id,32);
	cmd_306->work_stat = status->work_stat;

	cmd_306->bcl_voltage_need = status->bcl_voltage_need;
    cmd_306->bcl_current_need = status->bcl_current_need;
    cmd_306->bcl_charge_mode = status->bcl_charge_mode;
    cmd_306->bcs_test_voltage = status->bcs_test_voltage;
    cmd_306->bcs_test_current = status->bcs_test_current;
    cmd_306->bcs_max_single_voltage = status->bcs_max_single_voltage;
    cmd_306->bcs_max_single_no = status->bcs_max_single_no;
    cmd_306->bcs_current_soc = status->bcs_current_soc;
    cmd_306->last_charge_time = status->charge_time_left;
    cmd_306->bsm_single_no = status->bsm_single_no;
    cmd_306->bsm_max_temperature = status->bsm_max_temperature;
    cmd_306->bsm_max_temperature_check_no = status->bsm_max_temperature_check_no;
    cmd_306->bsm_min_temperature = status->bsm_min_temperature;
    cmd_306->bsm_min_temperature_check_no = status->bsm_min_temperature_check_no;
    cmd_306->bsm_voltage_too_high_or_too_low = status->bsm_voltage_too_high_or_too_low;
    cmd_306->bsm_car_battery_soc_too_high_or_too_low = status->bsm_car_battery_soc_too_high_or_too_low;
    cmd_306->bsm_car_battery_charge_over_current = status->bsm_car_battery_charge_over_current;
    cmd_306->bsm_battery_temperature_too_high = status->bsm_battery_temperature_too_high;
    cmd_306->bsm_battery_insulation_state = status->bsm_battery_insulation_state;
    cmd_306->bsm_battery_connect_state = status->bsm_battery_connect_state;
    cmd_306->bsm_allow_charge = status->bsm_allow_charge;


}

void _build_xj_cmd_308(xj_cmd_308* cmd_308,xj_bms_status* status,uint8_t*charge_user_id,uint32_t serial_code)
{
    cmd_308->serial_code = serial_code;
	cmd_308->gun_index = status->gun_index+1;
	strncpy(cmd_308->equipment_id,_g_xj_params->equipment_id,32);
	strncpy(cmd_308->charge_user_id,charge_user_id,32);
	cmd_308->work_stat = status->work_stat;

    cmd_308->CST_stop_reason = status->CST_stop_reason;
    cmd_308->CST_fault_reason = status->CST_fault_reason;
    cmd_308->CST_error_reason = status->CST_error_reason;
}

void _build_xj_cmd_310(xj_cmd_310* cmd_310,xj_bms_status* status,uint8_t*charge_user_id,uint32_t serial_code)
{
    cmd_310->serial_code = serial_code;
	cmd_310->gun_index = status->gun_index+1;
	strncpy(cmd_310->equipment_id,_g_xj_params->equipment_id,32);
	strncpy(cmd_310->charge_user_id,charge_user_id,32);
	cmd_310->work_stat = status->work_stat;

    cmd_310->BST_stop_reason = status->BST_stop_reason;
    cmd_310->BST_fault_reason = status->BST_fault_reason;
    cmd_310->BST_error_reason = status->BST_error_reason;
}


void _build_xj_cmd_312(xj_cmd_312* cmd_312,xj_bms_status* status,uint8_t*charge_user_id,uint32_t serial_code)
{
	cmd_312->serial_code = serial_code;
	cmd_312->gun_index = status->gun_index+1;
	strncpy((char*)cmd_312->equipment_id,(char*)_g_xj_params->equipment_id,32);
	strncpy((char*)cmd_312->charge_user_id,(char*)charge_user_id,32);
	cmd_312->work_stat = status->work_stat;

	cmd_312->bsd_stop_soc = status->bsd_stop_soc;
	cmd_312->bsd_battery_low_voltage = status->bsd_battery_low_voltage;
	cmd_312->bsd_battery_high_voltage = status->bsd_battery_high_voltage;
	cmd_312->bsd_battery_low_temperature = status->bsd_battery_low_temperature;
	cmd_312->bsd_battery_high_temperature = status->bsd_battery_high_temperature;
	cmd_312->error_68 = status->bem_error_spn2560_00;
	cmd_312->error_69 = status->bem_error_spn2560_aa;
	cmd_312->error_70 = status->bem_error_time_sync;
	cmd_312->error_71 = status->bem_error_ready_to_charge;
	cmd_312->error_72 = status->bem_error_receive_status;
	cmd_312->error_73 = status->bem_error_receive_stop_charge;
	cmd_312->error_74 = status->bem_error_receive_report;
	cmd_312->error_75 = status->bem_error_other;
}


void _build_xj_cmd_106(xj_cmd_106* cmd_106)
{
//	cmd_106->charge_mode_num = _g_xj_params->charge_module_cnt;
//	cmd_106->charge_mode_rate = _g_xj_params->charge_module_kw;
	strncpy((char*)cmd_106->equipment_id,(char*)_g_xj_params->equipment_id,32);
/*	int flag = 01;

	if(_g_xj_params->allow_charge==1){
		flag += 10;
	}
	if(_g_xj_params->allow_offline_charge==1){
		flag += 1;
	}*/

	cmd_106->offline_charge_flag = 01;
	cmd_106->stake_version=(_g_xj_params->user_version[3]<<24|_g_xj_params->user_version[2]<<16|_g_xj_params->user_version[1]<<8|_g_xj_params->user_version[0]);
	cmd_106->stake_type = _g_xj_params->gun_type-1;
	cmd_106->stake_start_times = _g_xj_params->software_restart_cnt;
	cmd_106->data_up_mode = 0;
	cmd_106->sign_interval = _g_xj_params->sign_in_interval;
	cmd_106->gun_index = _g_xj_params->gun_cnt;
	cmd_106->heartInterval = _g_xj_params->heartbeat_interval;
	cmd_106->heart_out_times = _g_xj_params->heartbeat_timeout_check_cnt;
	cmd_106->stake_charge_record_num = _g_xj_params->gun_cnt;
	cmd_106->ccu_version = 0;//TCU才有
	/*cmd_106->stake_systime = 0;*/

	char start_charge_time[8];
	memset(start_charge_time,0,8);
	int i=0;
	int max_year=0;
	int max_month=0;
	int max_day=0;
	int max_hour=0;
	int max_minute=0;
	int max_sec=0;
	int j=0;
	for(j=0;j<_g_xj_params->gun_cnt;j++)
	{
		if(g_xj_gun_status[j].start_charge_time_year>max_year||
			g_xj_gun_status[j].start_charge_time_month>max_month||
			g_xj_gun_status[j].start_charge_time_day>max_day||
			g_xj_gun_status[j].start_charge_time_hour>max_hour||
			g_xj_gun_status[j].start_charge_time_minute>max_minute||
			g_xj_gun_status[j].start_charge_time_sec>max_sec)
		{
			i = j;
			max_year = g_xj_gun_status[j].start_charge_time_year;
			max_month = g_xj_gun_status[j].start_charge_time_month;
			max_day = g_xj_gun_status[j].start_charge_time_day;
			max_hour = g_xj_gun_status[j].start_charge_time_hour;
			max_minute = g_xj_gun_status[j].start_charge_time_minute;
			max_sec = g_xj_gun_status[j].start_charge_time_sec;
		}
	}
	_convert_time(g_xj_gun_status[i].start_charge_time_year,g_xj_gun_status[i].start_charge_time_month,g_xj_gun_status[i].start_charge_time_day,g_xj_gun_status[i].start_charge_time_hour,g_xj_gun_status[i].start_charge_time_minute,g_xj_gun_status[i].start_charge_time_sec,start_charge_time);
	memcpy(cmd_106->stake_last_charge_time,start_charge_time,8);
	char software_start_time[8];
//	_convert_time(_g_xj_params->software_start_year,_g_xj_params->software_start_month,_g_xj_params->software_start_day,_g_xj_params->software_start_hour,_g_xj_params->software_start_minute,_g_xj_params->software_start_sec,software_start_time);
	memcpy(cmd_106->stake_last_start_time,software_start_time,8);
	char system_time[8];
	_convert_time(xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec(),system_time);
	memcpy(cmd_106->stake_systime,system_time,8);
	memcpy(cmd_106->mac , _g_xj_params->mac_addr,32);
}

/********************************************************
  * @Description： xj消息应答
  * @Arguments	：null
  * @Returns	：null
 ********************************************************/
void _interval_work_for_send_mqtt_resp(void)
{
	_mqtt_resp_to_send* resp = _poll_mqtt_resp_to_send();//根据响应标志位，判断是否有消息需要去回复
	if(NULL != resp)
	{
		xj_mqtt_send_resp(resp->cmd_type,resp->data,resp->qos);
		resp->submited = 0;
	}
}


int8_t _xj_send_304(xj_bms_status *status,uint8_t*charge_user_id)
{
    int8_t result = -1;
    xj_cmd_304 send_buff;

    memset((char*)&send_buff,00,sizeof(xj_cmd_304));
	_build_xj_cmd_304(&send_buff,status,charge_user_id,++mqttMessageBase.masterSerialNum);

    result = xj_add_incident_handle(&xj_incident_info,xj_incident_type_send_304,(uint8_t*)&send_buff,sizeof(send_buff));

    return (0 == result)? TRUE:FALSE;
}

int8_t _xj_send_306(xj_bms_status *status,uint8_t*charge_user_id)
{
    xj_cmd_306 send_buff;
    memset((char*)&send_buff,00,sizeof(xj_cmd_306));
	_build_xj_cmd_306(&send_buff,status,charge_user_id,++mqttMessageBase.masterSerialNum);

	xj_cmd_305* cmd_305 = xj_mqtt_send_cmd(xj_cmd_type_306,&send_buff,_g_xj_params->network_cmd_timeout,1);
	if(NULL == cmd_305 || cmd_305->serial_code != send_buff.serial_code)
	{
        xj_pal_print_log(xj_log_warning,"code 306 Failed to reply!\n");
		return FALSE;//发送失败
	}
	return TRUE;
}


int8_t _xj_send_308(xj_bms_status *status,uint8_t*charge_user_id)
{
    xj_cmd_308 send_buff;
    memset((char*)&send_buff,00,sizeof(xj_cmd_308));
	_build_xj_cmd_308(&send_buff,status,charge_user_id,++mqttMessageBase.masterSerialNum);

	xj_cmd_307* cmd_307 = xj_mqtt_send_cmd(xj_cmd_type_308,&send_buff,_g_xj_params->network_cmd_timeout,1);
	if(NULL == cmd_307 || cmd_307->serial_code != send_buff.serial_code)
	{
        xj_pal_print_log(xj_log_warning,"code 308 Failed to reply!\n");
		return FALSE;//发送失败
	}
	return TRUE;
}

int8_t _xj_send_310(xj_bms_status *status,uint8_t*charge_user_id)
{
    xj_cmd_310 send_buff;
    memset((char*)&send_buff,00,sizeof(xj_cmd_310));
	_build_xj_cmd_310(&send_buff,status,charge_user_id,++mqttMessageBase.masterSerialNum);

	xj_cmd_309* cmd_309 = xj_mqtt_send_cmd(xj_cmd_type_310,&send_buff,_g_xj_params->network_cmd_timeout,1);
	if(NULL == cmd_309 || cmd_309->serial_code != send_buff.serial_code)
	{
        xj_pal_print_log(xj_log_warning,"code 310 Failed to reply!\n");
		return FALSE;//发送失败
	}
	return TRUE;
}


int8_t _xj_send_312(xj_bms_status *status,uint8_t*charge_user_id)
{
    int8_t result = -1;
    xj_cmd_312 send_buff;

    memset((char*)&send_buff,00,sizeof(xj_cmd_312));
	_build_xj_cmd_312(&send_buff,status,charge_user_id,++mqttMessageBase.masterSerialNum);

    result = xj_add_incident_handle(&xj_incident_info,xj_incident_type_send_312,(uint8_t*)&send_buff,sizeof(send_buff));

    return (0 == result)? TRUE:FALSE;
}


static xj_bool _xj_send_gun_status(xj_gun_status *status)
{
	_build_xj_cmd_104((xj_cmd_104*)(&_g_stack_var_for_interval_work_thread),status);

	xj_cmd_103* cmd_103 = xj_mqtt_send_cmd(xj_cmd_type_104,(xj_cmd_104*)(&_g_stack_var_for_interval_work_thread),_g_xj_params->network_cmd_timeout,1);
	if(NULL == cmd_103)
	{
		return xj_bool_false;
	}
	return xj_bool_true;
}

static xj_bool _xj_send_bms_status(xj_bms_status *status)
{
	_build_xj_cmd_302((xj_cmd_302*)(&_g_stack_var_for_interval_work_thread),status);

	xj_cmd_301* cmd_301 = xj_mqtt_send_cmd(xj_cmd_type_302,(xj_cmd_302*)(&_g_stack_var_for_interval_work_thread),_g_xj_params->network_cmd_timeout,1);
	if(NULL == cmd_301)
	{
		return xj_bool_false;
	}
	return xj_bool_true;
}



/********************************************************
  * @Description： xj发送106签到信息
  * @Arguments	：clean_flag :用于重连时触发106标志位
  * @Returns	：null
 ********************************************************/
void _interval_work_for_sign_in(uint8_t clean_flag)
{
    static uint32_t before_time = 0;
    uint32_t current_time = 0;
    current_time = (uint32_t)xj_pal_time();

    if(TRUE == clean_flag)
    {
        before_time = 0;
    }
    else
    {
        if((current_time - before_time) >=  _g_xj_params->sign_in_interval * 60)
        {
            xj_cmd_106 cmd_106;

            before_time = current_time;
            _build_xj_cmd_106(&cmd_106);
            xj_mqtt_send_cmd(xj_cmd_type_106,&cmd_106,_g_xj_params->network_cmd_timeout,1);
        }
    }

}


/********************************************************
  * @Description： xj发送mqtt_ping包 用于保持连接
  * @Arguments	：null
  * @Returns	：null
 ********************************************************/
void _interval_work_for_mqtt_ping(void)
{
	static uint32_t before_time = 0;
    uint32_t current_time = 0;
    current_time = (uint32_t)xj_pal_time();

    if((current_time - before_time) >= _g_xj_params->mqtt_ping_interval)
    {
        before_time = current_time;
	    mqtt_ping(&mqttMessageBase.client);//190815新增mqtt心跳发送 用于服务端取代102
	}
}


/********************************************************
  * @Description： xj发送102 xj心跳包
  * @Arguments	：null
  * @Returns	：null
 ********************************************************/
void _interval_work_for_heartbeat(void)
{
    static _heartbeat_param  heartbeat_param = {0,0,0};
    uint32_t current_time = 0;
    current_time = (uint32_t)xj_pal_time();

    if((current_time - heartbeat_param.sleeped_sec) >= _g_xj_params->heartbeat_interval)
    {
        xj_cmd_102 cmd_102;

        heartbeat_param.sleeped_sec = current_time;
        memset(&cmd_102,0,sizeof(xj_cmd_102));
        strncpy(cmd_102.equipment_id,_g_xj_params->equipment_id,XJ_MAX_EQUIPMENT_ID_LEN-1);
        cmd_102.heart_index = heartbeat_param.index;
        heartbeat_param.index++;

        xj_cmd_101* cmd_101 = xj_mqtt_send_cmd(xj_cmd_type_102,&cmd_102,_g_xj_params->network_cmd_timeout,0);
    	if(NULL == cmd_101)
        {
        	heartbeat_param.timeout_cnt++;

        	//if(heartbeat_param.timeout_cnt >= _g_xj_params->heartbeat_timeout_check_cnt)
        	if(heartbeat_param.timeout_cnt >= 3)
        	{
        	    heartbeat_param.index = 0;
        	    xj_add_incident_handle(&xj_incident_info,xj_incident_type_reconnection,NULL,0);
        		return;
        	}
        }
        else
        {
            //xj_pal_print_log(xj_log_message,"Received heartbeat resp!\n");
		    heartbeat_param.timeout_cnt = 0;
        }

    }

}

/********************************************************
  * @Description： xj发送104 枪状态
  * @Arguments	：null
  * @Returns	：null
 ********************************************************/
void _interval_work_for_send_gun_status(void)
{
    static uint32_t before_time = 0;
    uint32_t current_time = 0;
    uint8_t i = 0;

    current_time = (uint32_t)xj_pal_time();

    if((current_time - before_time) >= _g_xj_params->upload_gun_status_interval)
    {
        before_time = current_time;
        //for(i = 0;i < CHARGE_GUN_CNT;i++)
        for(i = 0;i < _g_xj_params->gun_cnt;i++)
        {
            g_xj_gun_status[i].car_connection_stat = g_xj_bms_status[i].car_connection_stat;
		    _xj_send_gun_status(&g_xj_gun_status[i]);
        }

    }
}

/********************************************************
  * @Description： xj发送306 bms状态
  * @Arguments	：null
  * @Returns	：null
 ********************************************************/
void _interval_work_for_send_bms_status(void)
{
    static uint32_t before_time = 0;
    uint32_t current_time = 0;
    uint8_t i = 0;

    current_time = (uint32_t)xj_pal_time();

    if((current_time - before_time) >= _g_xj_params->upload_bms_status_interval)
    {
        before_time = current_time;

        //for(i = 0;i < CHARGE_GUN_CNT;i++)
        for(i = 0;i < _g_xj_params->gun_cnt;i++)
        {
            if(xj_work_stat_type_charging == g_xj_gun_status[i].work_stat)
            {
                _xj_send_306(&g_xj_bms_status[i],g_xj_gun_status[i].charge_user_id);
            }
        }
    }
}


/********************************************************
  * @Description： xj发送202账单信息     ---5秒检查一次是否需要发送
  * @Arguments	：null
  * @Returns	：null

 ********************************************************/
void _interval_work_for_bill_send(void)
{
    static uint32_t before_time = 0;
    uint32_t current_time = 0;
    uint8_t i = 0;

    current_time = (uint32_t)xj_pal_time();

    if((current_time - before_time) >= 5)
    {
        uint8_t save_flag = FALSE;

        before_time = current_time;

        for(i = 0;i < MAX_HISTORY_ORDER_CNT;i++)
        {
            if(xj_bill_ready_upload == xj_histroy_bill_info[i].status)
            {
                xj_cmd_202 cmd_202;

                _build_xj_cmd_202(&cmd_202,&xj_histroy_bill_info[i].bill);
                xj_cmd_201* cmd_201 = xj_mqtt_send_cmd(xj_cmd_type_202,&cmd_202,_g_xj_params->network_cmd_timeout,1);
                if(NULL != cmd_201)
                {
                    if(0 != strncmp(xj_histroy_bill_info[i].bill.charge_user_id,cmd_201->user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
                    {
                        xj_pal_print_log(xj_log_warning,"Bill CODE 201 user id error！\n");
                    }
                    else if(cmd_202.serial_code != cmd_201->serial_code)
                    {
                        xj_pal_print_log(xj_log_warning,"Bill CODE 201 serial_code error！send = %d,rece = %d\n",cmd_202.serial_code, cmd_201->serial_code);
                    }
                    else
                    {
                        xj_histroy_bill_info[i].status = xj_bill_completed_Upload;
                        save_flag = TRUE;
                    }
                }
            }
        }
        if(TRUE == save_flag)
        {
            xj_save_charge_order();
        }
    }
}


/********************************************************
  * @Description：主动上传消息和应答消息处理                100ms进来一次
  * @Arguments	：
			    NULL
  * @Returns	：
 ********************************************************/
void _interval_work(void)
{

    static uint16_t sleep = 0;

    if(1 == _g_is_login)
    {
        _interval_work_for_send_mqtt_resp();//主动应答消息
        _interval_work_for_sign_in(FALSE);//106签到
        _interval_work_for_heartbeat();//xj心跳
        _interval_work_for_send_gun_status();
        _interval_work_for_send_bms_status();
        _interval_work_for_bill_send();//账单发送
    }

	if(sleep++ >= 10)//大约1.5秒更新一次账单信息
	{
	    sleep = 0;
		_interval_work_for_calculate_bill_info();//账单信息更新
	}
}




/*
*	 用于充电卡鉴权，返回值用于判断指令是否收到，鉴权结果请根据reslut判断
*	 参数：auth_arg 用于鉴权的参数，reslut 为鉴权结果
*	 返回值：xj_bool_false没有收到服务端鉴权反馈，xj_bool_true 收到服务端反馈
*/
static xj_bool xj_send_card_auth(xj_card_auth *auth_arg,uint16_t* reslut)
{
	xj_cmd_34 cmd_34;
	xj_cmd_33* cmd_33 = NULL;
	memset(&cmd_34,0,sizeof(xj_cmd_34));
	memcpy(cmd_34.equipment_id,_g_xj_params->equipment_id,XJ_MAX_EQUIPMENT_ID_LEN-1);
	cmd_34.gun_index = auth_arg->gun_index+1;
	memcpy(cmd_34.card_id,auth_arg->card_id,16);
	memcpy(cmd_34.random_id,auth_arg->random_id,48);
	memcpy(cmd_34.phy_card_id,auth_arg->phy_id,4);

	cmd_33 = xj_mqtt_send_cmd(xj_cmd_type_34,&cmd_34,_g_xj_params->network_cmd_timeout,1);

	if(NULL != cmd_33)
	{
	    if(auth_arg->gun_index+1 == cmd_33->gun_index)
	    {
            xj_pal_print_log(xj_log_remind,"Auth succeed,Card num %.16s balance %d.\n",auth_arg->card_id,cmd_33->card_money);
	        *reslut = cmd_33->auth_result;
	        return xj_bool_true;
	    }
	    else
	    {
            xj_pal_print_log(xj_log_warning,"Card auth result gun num discrepancy!");
	    }

	}
	return xj_bool_false;
}

/*
*	 用于刷卡请求充电，返回值用于判断指令是否收到，请求充电结果请根据reslut判断
*	 参数：auth_arg 用于请求充电的参数，reslut 为请求充电结果
*	 返回值：xj_bool_false没有收到服务端鉴权反馈，xj_bool_true 收到服务端反馈
*/

static xj_bool xj_send_card_charge(xj_card_charge *charge_arg,uint16_t* reslut)
{
	xj_cmd_36 cmd_36;
	xj_cmd_35* cmd_35 = NULL;

	memset(&cmd_36,0,sizeof(xj_cmd_36));
	memcpy(cmd_36.equipment_id,_g_xj_params->equipment_id,32);
	cmd_36.gun_index = charge_arg->gun_index+1;
	memcpy(cmd_36.card_id,charge_arg->card_id,16);

	cmd_35 = xj_mqtt_send_cmd(xj_cmd_type_36,&cmd_36,_g_xj_params->network_cmd_timeout,1);
	if(NULL != cmd_35)
	{
	    if(charge_arg->gun_index+1 == cmd_35->gun_index)
	    {
	        *reslut = cmd_35->result;
	        return xj_bool_true;
	    }
	    else
	    {
            xj_pal_print_log(xj_log_warning,"Card charge result gun num discrepancy!");
	    }
	}
    return xj_bool_false;
}



/********************************************************
  * @Description： xj发送事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    type：事件类型
			    event_param 事件参数
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
static int8_t xj_send_event_occured(uint8_t gun_cnt, xj_event_type type,uint32_t event_param)
{
    int8_t result = -1;
    st_108_cmd data;

    memset((char*)&data,00,sizeof(data));
	//if(gun_cnt < CHARGE_GUN_CNT)
	if(gun_cnt < _g_xj_params->gun_cnt)
    {
        data.gun_cnt = gun_cnt;
        data.type = type;
        data.event_param = event_param;
        result = xj_add_incident_handle(&xj_incident_info,xj_incident_type_send_108,(uint8_t*)&data,sizeof(data));
	}
	return result ;
}



/********************************************************
  * @Description：插枪后 触发xj发送事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_gun_pluged_in(uint8_t gun_cnt)
{
    uint32_t event_param = 0;


    //if(gun_cnt >= CHARGE_GUN_CNT)
    if(gun_cnt >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_gun_pluged_in  gun_cnt-param error  gun_cnt =%d\n",gun_cnt);
    }
    else
    {
        g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat = xj_work_stat_type_gun_pluged_in;
        g_xj_gun_status[gun_cnt].car_connection_stat = g_xj_bms_status[gun_cnt].car_connection_stat = xj_car_connection_stat_type_connected;
        memcpy((char*)&event_param,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN);
        if(1 != _g_is_login)
        {
            xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_gun_pluged_in\n");
        }
        else
        {
            if(xj_bool_true == xj_send_event_occured(gun_cnt,xj_event_gun_pluged_in,event_param))
        	{
        	    return 0;
        	}
        }


    }
    return -1;

}



/********************************************************
  * @Description：拔枪后 xj发送事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_gun_pluged_out(uint8_t gun_cnt)
{
    uint32_t event_param = 0;


    //if(gun_cnt >= CHARGE_GUN_CNT)
    if(gun_cnt >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_gun_pluged_out  gun_cnt-param error  gun_cnt =%d\n",gun_cnt);
    }
    else
    {
        g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat = xj_work_stat_type_idle;
        g_xj_gun_status[gun_cnt].car_connection_stat = g_xj_bms_status[gun_cnt].car_connection_stat = xj_car_connection_stat_type_disconnected;
        memcpy((char*)&event_param,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN);
        if(1 != _g_is_login)
        {
            xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_gun_pluged_out\n");
        }
        else
        {
            if(xj_bool_true == xj_send_event_occured(gun_cnt,xj_event_gun_pluged_out,event_param))
        	{
        	    return 0;
        	}
        }


    }
    return -1;

}

/********************************************************
  * @Description：触发开始充电 xj发送108事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    param 事件参数 4字节ASCII码 "0000"表示正常
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_charge_start(uint8_t gun_cnt,uint8_t* param)
{
    uint32_t event_param = 0;


    xj_pal_print_log(xj_log_remind,"xj_send_touch_charge_start gun_cnt=%02d , param = %s\n",gun_cnt,param);

    //if(gun_cnt >= CHARGE_GUN_CNT)
    if(gun_cnt >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_charge_start failure  gun_cnt =%d \n",gun_cnt);
    }
    else if(xj_work_stat_type_bist != g_xj_gun_status[gun_cnt].work_stat)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_charge_start failure  gun_cnt =%d,work stat =%d\n",gun_cnt,g_xj_gun_status[gun_cnt].work_stat);
    }
    else if(NULL == param)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_charge_start failure param error!!!\n");
    }
    else
    {
        memset((char*)&g_xj_gun_status[gun_cnt].stop_reason,00,XJ_MAX_ERR_NO_LEN);
        memcpy((char*)&g_xj_gun_status[gun_cnt].stop_reason,(char*)param,XJ_ERR_RESULT_LEN);
        memcpy((char*)&event_param,(char*)param,XJ_ERR_RESULT_LEN);
        if(0 == strncmp((char*)&event_param,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN))
        {
            g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat =  xj_work_stat_type_charging;
             sync_gun_status(&g_xj_gun_status[0],p_User_gun_info,g_xj_params.gun_cnt);

            if(1 != _g_is_login)
            {
                xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_charge_start\n");
            }
            else
            {
                xj_send_event_occured(gun_cnt,xj_event_charge_started,event_param);//发送108充电事件
                return 0;
            }
        }
        else
        {
            g_xj_gun_status[gun_cnt].start_flags = 1;
            g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat =  xj_work_stat_type_gun_not_pluged_out;
             sync_gun_status(&g_xj_gun_status[0],p_User_gun_info,g_xj_params.gun_cnt);
            if(1 != _g_is_login)
            {
                xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_charge_start\n");
            }
            else
            {
                xj_send_event_occured(gun_cnt,xj_event_charge_started,event_param);//发送108充电事件
                return 0;
            }
        }
    }
    return -1;
}




/********************************************************
  * @Description：停充后 xj发送事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    stop_reason: 停止原因 4字节ASCII码
			    param 事件参数4字节ASCII码 "0000"表示正常
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_charge_stoped(uint8_t gun_cnt, uint8_t*stop_reason,uint8_t* param)
{
    uint32_t event_param = 0;


    xj_pal_print_log(xj_log_remind,"xj_send_touch_charge_stoped gun_cnt=%02d , stop_reason = %s\n",gun_cnt,stop_reason);


    //if(gun_cnt >= CHARGE_GUN_CNT || NULL == stop_reason || NULL == param)
    if(gun_cnt >= _g_xj_params->gun_cnt || NULL == stop_reason || NULL == param)
    {
       xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_charge_stoped  gun_cnt =%d,len-stop_reason =%d\n",gun_cnt,Come_letter_num_len(stop_reason));
    }
    else if(xj_work_stat_type_bist != g_xj_gun_status[gun_cnt].work_stat && xj_work_stat_type_charging != g_xj_gun_status[gun_cnt].work_stat && xj_work_stat_type_stoping != g_xj_gun_status[gun_cnt].work_stat)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_charge_stoped failure  gun_cnt =%d,work stat =%d\n",gun_cnt,g_xj_gun_status[gun_cnt].work_stat);
    }
    else
    {
    	strncpy(g_xj_gun_status[gun_cnt].stop_reason,stop_reason,XJ_MAX_ERR_NO_LEN - 1);
        sync_gun_status(&g_xj_gun_status[0],p_User_gun_info,g_xj_params.gun_cnt);
    	g_xj_bms_status[gun_cnt].work_stat = g_xj_gun_status[gun_cnt].work_stat = xj_work_stat_type_gun_not_pluged_out;
    	memcpy((char*)&event_param,(char*)param,XJ_ERR_RESULT_LEN);
    	if(1 != _g_is_login)
        {
           xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_charge_stoped\n");
        }
        else
        {
            if(xj_bool_true == xj_send_event_occured(gun_cnt,xj_event_charge_stoped,event_param))
        	{
        	    return 0;
        	}
        }

	}
    return -1;
}

int8_t xj_send_warning(uint8_t gun_cnt, uint8_t *warning, uint32_t threshold)
{
    xj_cmd_120 cmd_120;
    int8_t result = -1;

    memset((char*)&cmd_120,00,sizeof(cmd_120));

    if(gun_cnt >= _g_xj_params->gun_cnt || NULL == warning)
    {
       xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_warning_occured  gun_cnt =%d,warning addr =%d\n",gun_cnt,warning);
    }
    else
    {
        strncpy((char*)cmd_120.equipment_id,(char*)g_xj_params.equipment_id,XJ_MAX_EQUIPMENT_ID_LEN-1);
        cmd_120.gun_index = gun_cnt + 1;
        memcpy((char*)cmd_120.warning_code,(char*)warning,XJ_ERR_RESULT_LEN);
        strncpy((char*)cmd_120.charge_user_id,(char*)g_xj_gun_status[gun_cnt].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1);
        cmd_120.type = 1;
        memcpy(cmd_120.threshold,&threshold,sizeof(cmd_120.threshold));

        if(1 != _g_is_login)
        {
           xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_warning_occured\n");
        }
        else
        {
            result = xj_add_incident_handle(&xj_incident_info,xj_incident_type_send_120,(uint8_t*)&cmd_120,sizeof(cmd_120));
        }
    }
    return result;
}

/********************************************************
  * @Description：xj发送告警码
  * @Author     ：周大侠
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    warning: 告警码    4字节ASCII码
  * @Returns	：
                成功：0
                失败：-1
 *******************************************************/
int8_t xj_send_touch_warning_occured(uint8_t gun_cnt, uint8_t* warning)
{
#if 0
    xj_cmd_120 cmd_120;
    int8_t result = -1;

    memset((char*)&cmd_120,00,sizeof(cmd_120));
    //if(gun_cnt >= CHARGE_GUN_CNT || NULL == warning)
    if(gun_cnt >= _g_xj_params->gun_cnt || NULL == warning)
    {
       xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_warning_occured  gun_cnt =%d,warning addr =%d\n",gun_cnt,warning);
    }
    else
    {
        strncpy((char*)cmd_120.equipment_id,(char*)g_xj_params.equipment_id,XJ_MAX_EQUIPMENT_ID_LEN-1);
        cmd_120.gun_index = gun_cnt + 1;
        memcpy((char*)cmd_120.warning_code,(char*)warning,XJ_ERR_RESULT_LEN);
        strncpy((char*)cmd_120.charge_user_id,(char*)g_xj_gun_status[gun_cnt].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1);
        cmd_120.type = 1;

        if(1 != _g_is_login)
        {
           xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_warning_occured\n");
        }
        else
        {
            result = xj_add_incident_handle(&xj_incident_info,xj_incident_type_send_120,(uint8_t*)&cmd_120,sizeof(cmd_120));
        }
    }
    return result;
#endif
    uint32_t th = 0x0000; /* 兼容,值为0,非ascii*/
    return xj_send_warning(gun_cnt,warning,th);
}




/********************************************************
  * @Description：报错后 xj发送事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    err_no: 报错码    4字节ASCII码
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_error_occured(uint8_t gun_cnt, uint8_t* err_no)
{
	xj_error error;

	memset((char*)&error,00,sizeof(error));
	//if(gun_cnt >= CHARGE_GUN_CNT || NULL == err_no)
	if(gun_cnt >= _g_xj_params->gun_cnt || NULL == err_no)
    {
       xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_error_occured  gun_cnt =%d,err_addr =%d\n",gun_cnt,err_no);
    }
    else
    {
        error.gun_index = gun_cnt;
        strncpy(error.err_no,err_no,XJ_MAX_ERR_NO_LEN - 1);
        error.err_flag = 0x00;

        /* input queue */
        #if defined(XJ_ERR_CACHE_USED)
        err_event_info_push(0,gun_cnt,err_no);
        #endif // defined

        if(1 != _g_is_login)
        {
           xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_error_occured\n");
        }
        else
        {
            if(xj_bool_true == xj_send_error(&error))
            {
                return 0;
            }
        }


    }
    return -1;

}


/********************************************************
  * @Description：报错解除后 xj发送事件
  * @Arguments	：
			    gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    err_no: 报错码    4字节ASCII码
  * @Returns	：
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_error_recovered(uint8_t gun_cnt, uint8_t* err_no)
{
	xj_error error;

	memset((char*)&error,00,sizeof(error));
	//if(gun_cnt >= CHARGE_GUN_CNT || NULL == err_no)
	if(gun_cnt >= _g_xj_params->gun_cnt || NULL == err_no)
    {
       xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_error_recovered  gun_cnt =%d,len-err_no =%d\n",gun_cnt,Come_letter_num_len(err_no));
    }
    else
    {
        error.gun_index = gun_cnt;
        strncpy(error.err_no,(char*)err_no,XJ_MAX_ERR_NO_LEN - 1);
        error.err_flag = 0x01;

        /* input queue */
        #if defined(XJ_ERR_CACHE_USED)
        err_event_info_push(1,gun_cnt,err_no);
        #endif // defined

        if(1 != _g_is_login)
        {
           xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_touch_error_recovered\n");
        }
        else
        {
            if(xj_bool_true == xj_send_error(&error))
            {
                return 0;
            }
        }
    }
    return -1;
}

/********************************************************
  * @Description：设置桩号
  * @Arguments	：
			    ID_str：桩号的字符串指针
			    长度不可超过宏XJ_MAX_EQUIPMENT_ID_LEN-1
  * @Returns	：
                设置成功：0
                设置失败：-1
 ********************************************************/
int8_t xj_touch_set_equipment_id(uint8_t* ID_str)
{
    uint8_t gun = 0;

    for(gun = 0;gun < g_xj_params.gun_cnt;gun++)
    {
        if(xj_work_stat_type_idle != g_xj_gun_status[gun].work_stat)
        {
            xj_pal_print_log(xj_log_warning,"gun:%d ,ID setting is not supported in the current state",gun+1);
            return -1;
        }
    }
    if(strlen(ID_str) < XJ_MAX_EQUIPMENT_ID_LEN)
    {
        strncpy(g_xj_params.equipment_id,ID_str,XJ_MAX_EQUIPMENT_ID_LEN-1);

        for(gun = 0;gun < g_xj_params.gun_cnt;gun++)
        {
            QR_code_String((char*)&g_xj_params.gun_qr_code[gun][0],g_xj_params.equipment_id,gun+1);
        }

        _persist_xj_params();
        if(1 ==_g_is_login)
        {
            xj_add_incident_handle(&xj_incident_info,xj_incident_type_reconnection,NULL,0);
        }
        return 0;
    }
    else
    {
        xj_pal_print_log(xj_log_warning,"Setup failed, equipment id length of the overrun ,max len:%d\n",XJ_MAX_EQUIPMENT_ID_LEN-1);
    }
    return -1;
}


/********************************************************
  * @Description：设置设备唯一编码
  * @Arguments	：
			    mac_str：充电桩Mac地址或者IMEI码的字符串指针
			    长度不可超过宏XJ_MAX_MAC_ADDR_LEN-1
  * @Returns	：
                设置成功：0
                设置失败：-1
 ********************************************************/
int8_t xj_touch_set_mac_addr(uint8_t* mac_str)
{
    if(strlen(mac_str) < XJ_MAX_MAC_ADDR_LEN)
    {
        strncpy(g_xj_params.mac_addr,mac_str,XJ_MAX_MAC_ADDR_LEN-1);
        _persist_xj_params();
        return 0;
    }
    else
    {
        xj_pal_print_log(xj_log_warning,"Setup failed,mac_addr length of the overrun ,max len:%d\n",XJ_MAX_MAC_ADDR_LEN-1);
        return -1;
    }
}

/********************************************************
  * @Description：设置mqtt登录信息
  * @Arguments	：
			    UserName : mqtt登录用户名
			    password ：mqtt登录密码
  * @Returns	：
                设置成功：0
                设置失败：-1
 ********************************************************/
int8_t xj_touch_set_mqtt_info(uint8_t* UserName ,uint8_t* password)
{
    uint8_t gun = 0;

    for(gun = 0;gun < g_xj_params.gun_cnt;gun++)
    {
        if(xj_work_stat_type_idle != g_xj_gun_status[gun].work_stat)
        {
            xj_pal_print_log(xj_log_warning,"gun:%d ,mqtt setting is not supported in the current state",gun+1);
            return -1;
        }
    }

    strncpy((uint8_t*)g_xj_params.username,UserName,XJ_MAX_USERNAME_LEN-1);
    strncpy((uint8_t*)g_xj_params.password,password,XJ_MAX_PASSWORD_LEN-1);

    _persist_xj_params();
    if(1 ==_g_is_login)
    {
        xj_add_incident_handle(&xj_incident_info,xj_incident_type_reconnection,NULL,0);
    }
    return 0;

}
#if 0
int8_t xj_touch_set_gun_cnt(uint8_t gun_cnt)
{
    uint8_t gun = 0;
    xj_params params;

    if((_g_xj_params->gun_cnt == 0) && (1 != _g_is_login))
    {
        /* maybe first set    before App_Start*/
        xj_helper_init_xj_params(&g_xj_params,gun_cnt);
        return 0;
    }
    /* running status */
    for(gun = 0;gun < g_xj_params.gun_cnt;gun++)
    {
        if(xj_work_stat_type_idle != g_xj_gun_status[gun].work_stat)
        {
            return -1;
        }
    }

    /* 禁止其他进程使用枪信息。防止数据不对*/
    g_xj_params.gun_cnt = 0;

    _restore_persisted_xj_params(&params);

    params.gun_cnt = (gun_cnt > CHARGE_GUN_CNT_MAX) ? CHARGE_GUN_CNT_DEFAULT : gun_cnt;

    xj_pal_write_persist_params((char*)&params,sizeof(xj_params));

    return 0;
    /* action 需要重启 */
}
#endif

/********************************************************
  * @Description：刷卡请求启动充电
  * @Arguments	：
			    info->gun_index:请求的枪号
			    0表示枪1 ,1表示枪2,以此类推
			    info->card_id:扇区2块0的虚拟卡号
			    info->random_id:扇区5内的随机数
			    info->phy_id:扇区0块0的物理卡号
  * @Returns	：
                执行结果
                成功：0
                失败：-1
 ********************************************************/
int8_t xj_send_touch_card_start_charge(xj_card_auth *info)
{
    uint16_t auth_result = 0xFF;//初始成一个不存在的结果
    uint16_t charge_result = 0xFF;//初始成一个不存在的结果
    xj_card_charge charge_info;

    if(info->gun_index < g_xj_params.gun_cnt)
    {
        if(xj_bool_true == xj_send_card_auth(info,&auth_result))
        {
            if(0 == auth_result)
            {
                memset((char*)&charge_info,00,sizeof(xj_card_charge));
                memcpy(charge_info.card_id,info->card_id,16);
                charge_info.gun_index = info->gun_index;
                if(xj_bool_true == xj_send_card_charge(&charge_info,&charge_result))
                {
                    if(0 == charge_result)
                    {
                        xj_pal_print_log(xj_log_remind,"Successfully request charge by card!\n");
                        return 0;
                    }
                    else
                    {
                        xj_pal_print_log(xj_log_remind,"Card charge failure,The reason %d\n",charge_result);
                    }
                }
                else
                {
                    xj_pal_print_log(xj_log_warning,"Card start charge failure! Card charge timeout\n");
                }
            }
            else
            {
                xj_pal_print_log(xj_log_remind,"Auth failure,The reason %d\n",auth_result);
            }
        }
        else
        {
            xj_pal_print_log(xj_log_warning,"Card start charge failure! Auth timeout\n");
        }
    }
    else
    {
        xj_pal_print_log(xj_log_Error,"Card start charge failure! gun_cnt =%d\n",info->gun_index);
    }
    return -1;
}



/********************************************************
  * @Description：VIN鉴权请求启动充电
  * @Arguments	：
			    gun:请求的枪号
			    0表示枪1 ,1表示枪2,以此类推
			    vin:请求车辆vin码字符串
  * @Returns	：
 ********************************************************/
void xj_send_touch_vin_charge_reques(uint8_t gun,char *vin)
{
    xj_cmd_40 send_buff;
    st_user_vin_send_result user_param;
    xj_cmd_41* p_cmd_41 = NULL;

    xj_cmd_8 cmd_8;

    memset((char*)&send_buff,00,sizeof(send_buff));
    memset((char*)&user_param,00,sizeof(user_param));

    user_param.gun_index = gun;
    user_param.Request_result = 1;
    user_param.failure_reasons = 0xFF;//初始成失败

    if(gun >= g_xj_params.gun_cnt ||  NULL == vin)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_vin_charge_reques  param error!\n");
    }
    else if(xj_work_stat_type_gun_pluged_in != g_xj_gun_status[gun].work_stat)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_vin_charge_reques failure ,work stat =%d\n",g_xj_gun_status[gun].work_stat);
    }
    else
    {
        strncpy(send_buff.equipment_id,_g_xj_params->equipment_id,32);
        send_buff.gun_index  = gun + 1;
        strncpy(send_buff.vin,vin,17);
        send_buff.serial_code = ++mqttMessageBase.masterSerialNum;

        p_cmd_41 = xj_mqtt_send_cmd(xj_cmd_type_40,&send_buff,_g_xj_params->network_cmd_timeout + 5,1);
        if(NULL == p_cmd_41 )
        {
            xj_pal_print_log(xj_log_warning,"code 41 Failed to reply!\n");
        }
        else if(send_buff.gun_index != p_cmd_41->gun_index || 0 != strncmp(vin,p_cmd_41->vin,17) || 0 != strncmp(g_xj_params.equipment_id,p_cmd_41->equipment_id,32))
        {
             xj_pal_print_log(xj_log_warning,"code 41 reply parameter does not match!\n");
        }
        else if(xj_work_stat_type_gun_pluged_in != g_xj_gun_status[gun].work_stat)
        {
            xj_pal_print_log(xj_log_Error,"Func:xj_send_touch_vin_charge_reques failure ,work stat =%d\n",g_xj_gun_status[gun].work_stat);
        }
        else
        {
            memcpy(user_param.charge_user_id,p_cmd_41->charge_user_id,32);
            user_param.balance = p_cmd_41->balance;
            user_param.Request_result = p_cmd_41->Request_result;
            user_param.failure_reasons = p_cmd_41->failure_reasons;
            user_param.remainkon = p_cmd_41->remainkon;
            user_param.dump_energy = p_cmd_41->dump_energy;
            user_param.residue_degree = p_cmd_41->residue_degree;
            user_param.phone = p_cmd_41->phone;
        }
    }

    /* 鉴权成功发送 code=8 */
    if((p_cmd_41 != NULL) && (0 == p_cmd_41->Request_result))
    {
        /*发送code=8 */
        memset(&cmd_8,0x0,sizeof(cmd_8));
        memcpy(&cmd_8.charge_user_id[0],&p_cmd_41->charge_user_id[0],32);
        memcpy(&cmd_8.equipment_id[0],&g_xj_params.equipment_id[0],sizeof(cmd_8.equipment_id));
        cmd_8.gun_index = gun;
        cmd_8.serial_code = mqttMessageBase.masterSerialNum;
        memcpy(&cmd_8.result[0],"0000",4);

        _xj_mqtt_send_resp_wrap(xj_cmd_type_8,(char*)&cmd_8,sizeof(xj_cmd_8),1);
    }

    if(0 == callback_vin_start_charge_result(user_param) && 0 == user_param.Request_result)
    {
        memcpy(g_xj_gun_status[gun].charge_user_id,user_param.charge_user_id,32);
        g_xj_gun_status[gun].user_tel = user_param.phone;
        g_xj_bms_status[gun].work_stat = g_xj_gun_status[gun].work_stat = xj_work_stat_type_bist;
        _interval_work_for_calculate_bill_info();
        xj_pal_print_log(xj_log_message,"vin start charge succeed!\n");
    }
    else
    {
        xj_pal_print_log(xj_log_remind,"vin start charge failure!\n");
    }

}


/********************************************************
  * @Description：触发发送bms 基础信息  在充电启动后上传一次，  1笔订单发送1次(无论启动成功或失败都需上传)
  * @Arguments	：
                gun :对应更新的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    charge_user_id：订单号字符串指针 字符长度不要超过XJ_MAX_CHAEGE_USER_ID_LEN-1
			    data:要更新的bms基础信息结构指针
  * @Returns	：
                0 发送成功
                -1 发送失败
 *******************************************************/
int8_t touch_send_bms_basic_info (uint8_t gun,char*charge_user_id,st_bms_basic_info *data)
{
   // if(gun < CHARGE_GUN_CNT && NULL != charge_user_id && NULL != data)
    if(gun < _g_xj_params->gun_cnt && NULL != charge_user_id && NULL != data)
    {
        if(0 == strncmp(charge_user_id,(char*)g_xj_gun_status[gun].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
        {
            memcpy(g_xj_bms_status[gun].brm_bms_connect_version,data->brm_bms_connect_version,XJ_MAX_BRM_BMS_CONNECT_VERSION_LEN);
            g_xj_bms_status[gun].brm_battery_type = data->brm_battery_type;
            g_xj_bms_status[gun].brm_battery_power = data->brm_battery_power;
            g_xj_bms_status[gun].brm_battery_volt = data->brm_battery_volt;
            g_xj_bms_status[gun].brm_battery_supplier = data->brm_battery_supplier;
            g_xj_bms_status[gun].brm_battery_seq = data->brm_battery_seq;
            g_xj_bms_status[gun].brm_battery_produce_year = data->brm_battery_produce_year;
            g_xj_bms_status[gun].brm_battery_produce_month = data->brm_battery_produce_month;
            g_xj_bms_status[gun].brm_battery_produce_day = data->brm_battery_produce_day;
            g_xj_bms_status[gun].brm_battery_charge_count = data->brm_battery_charge_count;
            g_xj_bms_status[gun].brm_battery_property_identification = data->brm_battery_property_identification;
            memcpy(g_xj_bms_status[gun].brm_vin,data->brm_vin,XJ_MAX_BRM_VIN_LEN);
            memcpy(g_xj_bms_status[gun].brm_bms_software_version,data->brm_bms_software_version,XJ_MAX_BRM_BMS_SOFTWARE_VERSION_LEN);
            g_xj_bms_status[gun].bcp_max_voltage = data->bcp_max_voltage;
            g_xj_bms_status[gun].bcp_max_current = data->bcp_max_current;
            g_xj_bms_status[gun].bcp_max_power = data->bcp_max_power;
            g_xj_bms_status[gun].bcp_total_voltage = data->bcp_total_voltage;
            g_xj_bms_status[gun].bcp_max_temperature = data->bcp_max_temperature;
            g_xj_bms_status[gun].bcp_battery_soc = data->bcp_battery_soc;
            g_xj_bms_status[gun].bcp_battery_soc_current_voltage = data->bcp_battery_soc_current_voltage;
            g_xj_bms_status[gun].bro_BMS_isReady = data->bro_bms_is_ready;
            g_xj_bms_status[gun].CRO_isReady = data->CRO_isReady;

            if(TRUE ==  _xj_send_304(&g_xj_bms_status[gun],(uint8_t*)g_xj_gun_status[gun].charge_user_id))
            {
                return 0;
            }
        }
    }

    return -1;

}




/********************************************************
  * @Description：触发发送bms 中止CST数据     ，  1笔订单发送1次
  * @Arguments	：
                gun :对应更新的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    charge_user_id：订单号字符串指针 字符长度不要超过XJ_MAX_CHAEGE_USER_ID_LEN-1
			    data:要更新的bms 中止CST数据结构指针
  * @Returns	：
                0 发送成功
                -1 发送失败

 *******************************************************/
int8_t touch_send_bms_cst_data (uint8_t gun,char*charge_user_id,st_off_bms_cst_data* data)
{
    //if(gun < CHARGE_GUN_CNT && NULL != charge_user_id && NULL != data)
    if(gun < _g_xj_params->gun_cnt && NULL != charge_user_id && NULL != data)
    {
        if(0 == strncmp(charge_user_id,(char*)g_xj_gun_status[gun].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
        {
            g_xj_bms_status[gun].CST_stop_reason = data->CST_stop_reason;
            g_xj_bms_status[gun].CST_fault_reason = data->CST_fault_reason;
            g_xj_bms_status[gun].CST_error_reason = data->CST_error_reason;

            if(TRUE ==  _xj_send_308(&g_xj_bms_status[gun],(uint8_t*)g_xj_gun_status[gun].charge_user_id))
            {
                return 0;
            }
        }
    }

    return -1;

}

/********************************************************
  * @Description：触发发送bms 中止BST数据     ，  1笔订单发送1次
  * @Arguments	：
                gun :对应更新的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    charge_user_id：订单号字符串指针 字符长度不要超过XJ_MAX_CHAEGE_USER_ID_LEN-1
			    data:要更新的bms 中止BST数据结构指针
  * @Returns	：
                0 发送成功
                -1 发送失败
 *******************************************************/
int8_t touch_send_bms_bst_data (uint8_t gun,char*charge_user_id,st_off_bms_bst_data* data)
{
    //if(gun < CHARGE_GUN_CNT && NULL != charge_user_id && NULL != data)
    if(gun < _g_xj_params->gun_cnt && NULL != charge_user_id && NULL != data)
    {
        if(0 == strncmp(charge_user_id,(char*)g_xj_gun_status[gun].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
        {
            g_xj_bms_status[gun].BST_stop_reason = data->BST_stop_reason;
            g_xj_bms_status[gun].BST_fault_reason = data->BST_fault_reason;
            g_xj_bms_status[gun].BST_error_reason = data->BST_error_reason;

            if(TRUE ==  _xj_send_310(&g_xj_bms_status[gun],(uint8_t*)g_xj_gun_status[gun].charge_user_id))
            {
                return 0;
            }
        }
    }
    return -1;
}



/********************************************************
  * @Description：触发发送bms 结束统计数据     ，  1笔订单发送1次  ，充电结束时发送
  * @Arguments	：
                gun :对应更新的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    charge_user_id：订单号字符串指针 字符长度不要超过XJ_MAX_CHAEGE_USER_ID_LEN-1
			    data:要更新的bms 结束统计数据结构指针
  * @Returns	：
                0 发送成功
                -1 发送失败

 *******************************************************/
int8_t touch_send_bms_inish_statistical_data (uint8_t gun ,char*charge_user_id,st_bms_finish_statistical_data* data)
{
   // if(gun < CHARGE_GUN_CNT && NULL != charge_user_id && NULL != data)
    if(gun < _g_xj_params->gun_cnt && NULL != charge_user_id && NULL != data)
    {
        if(0 == strncmp(charge_user_id,(char*)g_xj_gun_status[gun].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
        {
            g_xj_bms_status[gun].bsd_stop_soc = data->bsd_stop_soc;
            g_xj_bms_status[gun].bsd_battery_low_voltage = data->bsd_battery_low_voltage;
            g_xj_bms_status[gun].bsd_battery_high_voltage = data->bsd_battery_high_voltage;
            g_xj_bms_status[gun].bsd_battery_low_temperature = data->bsd_battery_low_temperature;
            g_xj_bms_status[gun].bsd_battery_high_temperature = data->bsd_battery_high_temperature;
            g_xj_bms_status[gun].bem_error_spn2560_00 = data->bem_error_spn2560_00;
            g_xj_bms_status[gun].bem_error_spn2560_aa = data->bem_error_spn2560_aa;
            g_xj_bms_status[gun].bem_error_time_sync = data->bem_error_time_sync;
            g_xj_bms_status[gun].bem_error_ready_to_charge = data->bem_error_ready_to_charge;
            g_xj_bms_status[gun].bem_error_receive_status = data->bem_error_receive_status;
            g_xj_bms_status[gun].bem_error_receive_stop_charge = data->bem_error_receive_stop_charge;
            g_xj_bms_status[gun].bem_error_receive_report = data->bem_error_receive_report;
            g_xj_bms_status[gun].bem_error_other = data->bem_error_other;

            if(TRUE ==  _xj_send_312(&g_xj_bms_status[gun],(uint8_t*)g_xj_gun_status[gun].charge_user_id))
            {
                return 0;
            }
        }
    }
        return -1;

}



/********************************************************
  * @Description：触发发送1102 用于异步下载应答
  * @Arguments	：
                update_result:下载结果
			    md5：下载计算所得md5校验指针 最大32字符
			    serial_code：callback_software_download 1101的命令序列号域
  * @Returns	：
                无
 *******************************************************/
void touch_send_1102_code(uint8_t update_result ,uint8_t *md5 ,uint32_t serial_code)
{
    xj_cmd_1102 response;

    memset((char*)&response,00,sizeof(response));
    response.update_result = update_result;
    strncpy(response.md5,md5,32);
    response.serial_code = serial_code;

    _xj_mqtt_send_resp_wrap(xj_cmd_type_1102,(char*)&response,sizeof(xj_cmd_1102),1);
}





void xj_incident_send_304(xj_cmd_304* send_buff)
{
    if(NULL != send_buff)
    {
        xj_cmd_303* cmd_303 = xj_mqtt_send_cmd(xj_cmd_type_304,send_buff,_g_xj_params->network_cmd_timeout,1);
    	if(NULL == cmd_303 || cmd_303->serial_code != send_buff->serial_code)
    	{
            xj_pal_print_log(xj_log_warning,"code 304 Failed to reply!\n");
    	}
    }
}


void xj_incident_send_312(xj_cmd_312* send_buff)
{
    if(NULL != send_buff)
    {
        xj_cmd_311* cmd_311 = xj_mqtt_send_cmd(xj_cmd_type_312,send_buff,_g_xj_params->network_cmd_timeout,1);
        if(NULL == cmd_311 || cmd_311->serial_code != send_buff->serial_code)
    	{
            xj_pal_print_log(xj_log_warning,"code 312 Failed to reply!\n");
    	}
    }
}

void xj_incident_send_120(xj_cmd_120* send_buff)
{
    xj_cmd_119* cmd_119 = NULL;
    if(NULL != send_buff)
    {
        cmd_119 = xj_mqtt_send_cmd(xj_cmd_type_120,send_buff,_g_xj_params->network_cmd_timeout,1);
    	if(NULL == cmd_119)
    	{
    		xj_pal_print_log(xj_log_warning,"code 120 Failed to reply!\n");
    	}
    }
}





/********************************************************
  * @Description： xj对电表异常时，主动上报告警
  * @Arguments	：gun  枪号
  * @Returns	：null
 ********************************************************/
void xj_send_Electric_meter_warning(uint8_t gun)
{
    static uint32_t before_time = 0;
    uint32_t current_time = 0;
    uint8_t i = 0;

    current_time = (uint32_t)xj_pal_time();

    if((current_time - before_time) >= 10)
    {
        before_time = current_time;
        xj_send_touch_warning_occured(gun,XJ_SDK_ERR_300E);
    }
}




xj_bool xj_send_event(uint8_t gun_cnt, xj_event_type type,uint32_t	event_param)
{
    xj_pal_print_log(xj_log_message,"Event gun_cnt %d event %d\n", gun_cnt, type);

    xj_cmd_108 cmd_108;
	memset(&cmd_108,0,sizeof(xj_cmd_108));
	cmd_108.gun_index = gun_cnt + 1;
	cmd_108.event_addr = type;
	cmd_108.event_param = event_param;
	memcpy((char*)cmd_108.charge_user_id,(char*)g_xj_gun_status[gun_cnt].charge_user_id,sizeof(cmd_108.charge_user_id));

	xj_cmd_107* cmd_107 = xj_mqtt_send_cmd(xj_cmd_type_108,&cmd_108,_g_xj_params->network_cmd_timeout,1);
	if(cmd_107==NULL)
	{
		return xj_bool_false;
	}
	return xj_bool_true;
}

xj_bool xj_send_error(xj_error* error){
	xj_cmd_118 cmd_118;
	memset(&cmd_118,0,sizeof(xj_cmd_118));
	memcpy(cmd_118.equipment_id,_g_xj_params->equipment_id,32);
	cmd_118.gun_index = error->gun_index+1;
	memcpy(cmd_118.err_code,error->err_no,4);
	cmd_118.err_status = error->err_flag;
	xj_pal_print_log(xj_log_remind,"xj_send_error gun =%d,error=%s,flag=%d.\n",error->gun_index,error->err_no,error->err_flag);
	xj_cmd_117* cmd_117 = xj_mqtt_send_cmd(xj_cmd_type_118,&cmd_118,_g_xj_params->network_cmd_timeout,1);
	if(NULL == cmd_117)
	{
		return xj_bool_false;
	}
	return xj_bool_true;
}







void _cmd_callback_start_charge(void* arg)
{
	xj_cmd_7* cmd_7 = (xj_cmd_7*)arg;
	xj_start_charge_param param;

	memset((char*)&param,00,sizeof(xj_start_charge_param));
	param.serial_code = cmd_7->serial_code;
	param.gun_index = cmd_7->gun_index-1;
	param.user_tel = cmd_7->user_tel;
	memcpy(param.charge_user_id,cmd_7->charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN);
    xj_add_incident_handle(&xj_incident_info,xj_incident_type_start_charge,(uint8_t*)&param,sizeof(xj_start_charge_param));
}


uint8_t* get_xj_param_data_addr_for_cmd_1(int addr,uint16_t* param_len)
{
    uint8_t* p_param = NULL;

	switch(addr)
	{
		case 1:
			p_param = (uint8_t*)&_g_xj_params->sign_in_interval;
			*param_len = sizeof(_g_xj_params->sign_in_interval);
			break;
		case 3:
		//	p_param = (uint8_t*)&_g_xj_params->gun_cnt;
		//	*param_len = sizeof(_g_xj_params->gun_cnt);
			break;
/*
		case 5:
			return &_g_xj_params->max_charge_volt;
		case 6:
			return &_g_xj_params->max_charge_curr;
		case 8:
			return &_g_xj_params->max_charge_power_kw;
		case 9:
			return &_g_xj_params->min_charge_power_kw;
		case 17:
			return &_g_xj_params->bms_unit_protect_volt;
		case 18:
			return &_g_xj_params->bms_unit_protect_temperature;
		case 19:
			return &_g_xj_params->allow_charge;
		case 20:
			return &_g_xj_params->allow_offline_charge;
		case 26:
			return &_g_xj_params->allow_gun_lock;
		case 29:
			return &_g_xj_params->upload_gun_status_interval;
		case 30:
			return &_g_xj_params->heartbeat_interval;
		case 31:
			return &_g_xj_params->heartbeat_timeout_check_cnt;
		case 32:
			return &_g_xj_params->gun_status_sync_interval;
		case 38:
			return &_g_xj_params->show_charge_price;
		case 51:
			return &_g_xj_params->charge_duration_after_offline;
*/
        case 52:
            p_param = (uint8_t*)&_g_xj_params->mqtt_ping_interval;
			*param_len = sizeof(_g_xj_params->mqtt_ping_interval);
			break;
		case 53:
		    p_param = (uint8_t*)&_g_xj_params->logic_svr_addr;
			*param_len = XJ_MAX_SVR_ADDR_LEN;
			break;
		case 54:
		    p_param = (uint8_t*)&_g_xj_params->logic_svr_port;
			*param_len = sizeof(_g_xj_params->logic_svr_port);
			break;
		default:
		    p_param = NULL;
		    break;
	}
	return p_param;
}

void _cmd_callback_cmd_1(void* arg)
{
	xj_cmd_1* cmd_1 = (xj_cmd_1*)arg;
	uint8_t* p_param = NULL;
	uint16_t param_len = 0;

	p_param = get_xj_param_data_addr_for_cmd_1(cmd_1->start_addr,&param_len);

	if(NULL != p_param && ((1 == cmd_1->cmd_type) || (0 == cmd_1->cmd_type)))//&& param_len < cmd_len
	{
	    if(1 == cmd_1->cmd_type)
	    {
            uint16_t i = 0;
            xj_pal_print_log(xj_log_remind,"Setting value: addr:%d ,value: ",cmd_1->start_addr);
            for(i = 0 ;i < cmd_1->cmd_len ;i++)
            {
                xj_pal_print_log(xj_log_Null,"%X ",cmd_1->data[i]);
            }
            xj_pal_print_log(xj_log_Null,"\n");

            memcpy(p_param,cmd_1->data,param_len);
			_persist_xj_params();
            xj_add_incident_handle(&xj_incident_info,xj_incident_type_params_updated,NULL,0); //暂时没用  后续这一块觉得CODE1要重写
	    }
	    else
	    {
	        memcpy(((xj_cmd_2*)_g_stack_var_for_callback_thread)->data,p_param,param_len);
	        ((xj_cmd_2*)_g_stack_var_for_callback_thread)->data_len = param_len;
	    }

	    ((xj_cmd_2*)_g_stack_var_for_callback_thread)->result = 0;

	}
	else
	{
	    ((xj_cmd_2*)_g_stack_var_for_callback_thread)->result = 0xff;
	}

	memcpy(((xj_cmd_2*)_g_stack_var_for_callback_thread)->equipment_id,_g_xj_params->equipment_id,32);
	((xj_cmd_2*)_g_stack_var_for_callback_thread)->cmd_type = cmd_1->cmd_type;
	((xj_cmd_2*)_g_stack_var_for_callback_thread)->start_addr = cmd_1->start_addr;
	((xj_cmd_2*)_g_stack_var_for_callback_thread)->cmd_num = 1;
    ((xj_cmd_2*)_g_stack_var_for_callback_thread)->serial_code = cmd_1->serial_code;

	_xj_mqtt_send_resp_wrap(xj_cmd_type_2,_g_stack_var_for_callback_thread,sizeof(xj_cmd_2),1);
}




void _cmd_callback_cmd_3(void* arg)
{
/*
	xj_cmd_3* cmd_3 = (xj_cmd_3*)arg;
	_string_data_addr data_addr;
	memset(&data_addr,0,sizeof(_string_data_addr));
	int addr= cmd_3->start_addr;
	data_addr = get_xj_param_data_addr_for_cmd_3(addr);
	xj_sync_system_time_param time_param;
	if(cmd_3->cmd_type == 1)
	{
		if(cmd_3->start_addr == 2)
		{
			_convert_time_back(cmd_3->data,&time_param);
            xj_add_incident_handle(&xj_incident_info,xj_incident_type_set_time,(uint8_t*)&time_param,sizeof(xj_sync_system_time_param));
			return;
		}

		if(cmd_3->start_addr == 17)
		{
		    xj_add_incident_handle(&xj_incident_info,xj_incident_type_set_left_qr,cmd_3->data,strlen(cmd_3->data));
			return;
		}

		if(cmd_3->start_addr == 19)
		{
		    xj_add_incident_handle(&xj_incident_info,xj_incident_type_set_righ_qr,cmd_3->data,strlen(cmd_3->data));
			return;
		}

		if(data_addr.addr!=NULL)
		{
			if(strncmp(data_addr.addr,cmd_3->data,data_addr.len)!=0)
			{
				memset(data_addr.addr,0,data_addr.len);
				strncpy(data_addr.addr,cmd_3->data,data_addr.len);
                xj_pal_print_log(xj_log_remind,"setting str value. addr:%d value:%s!\n",cmd_3->start_addr,cmd_3->data);
				_persist_xj_params();

                xj_add_incident_handle(&xj_incident_info,xj_incident_type_params_updated,NULL,0);
			}
		}
	}
	memcpy(((xj_cmd_4*)_g_stack_var_for_callback_thread)->equipment_id,_g_xj_params->equipment_id,32);
	((xj_cmd_4*)_g_stack_var_for_callback_thread)->cmd_type = cmd_3->cmd_type;
	((xj_cmd_4*)_g_stack_var_for_callback_thread)->start_addr = cmd_3->start_addr;
	((xj_cmd_4*)_g_stack_var_for_callback_thread)->result = 0;
	if(data_addr.addr!=NULL){
		memcpy(((xj_cmd_4*)_g_stack_var_for_callback_thread)->data,data_addr.addr,strlen(data_addr.addr));
		((xj_cmd_4*)_g_stack_var_for_callback_thread)->dataLen = strlen(data_addr.addr);
	}

	_xj_mqtt_send_resp_wrap(xj_cmd_type_4,_g_stack_var_for_callback_thread,sizeof(xj_cmd_4),1);
*/
}


void* _cmd_callback_control_cmd(void* arg)
{

	xj_cmd_5* cmd_5 = (xj_cmd_5*)arg;
	xj_control_cmd_param param;

	memset((char*)&param,00,sizeof(xj_control_cmd_param));

	param.gun_index = cmd_5->gun_index-1;
	param.cmd_type  = cmd_5->addr;
	param.cmd_count = cmd_5->cmd_num;
	param.cmd_len   = cmd_5->cmd_len;
	param.serial_code = cmd_5->serial_code;

	if(param.cmd_len <= 128)
	{
	   memcpy(param.cmd_data, (uint8_t*)cmd_5->cmd_param,param.cmd_len);
	}
	else
	{
        xj_pal_print_log(xj_log_warning,"CODE 5 ,cmd param Len over upper limit ,len=%d\n",param.cmd_len);
	    memcpy(param.cmd_data, (uint8_t*)cmd_5->cmd_param,128);
	}

    xj_add_incident_handle(&xj_incident_info,xj_incident_type_control_cmd,(uint8_t*)&param,sizeof(xj_control_cmd_param));

	return NULL;
}

void _cmd_callback_recent_bill_info(void* arg)
{
	xj_cmd_205* cmd_205 = (xj_cmd_205*)arg;
	uint8_t i=0;

	memset(_g_stack_var_for_callback_thread,0,sizeof(xj_cmd_206));

	for(i = 0;i < MAX_HISTORY_ORDER_CNT;i++)
	{
	    if(xj_bill_ready_upload == xj_histroy_bill_info[i].status || xj_bill_completed_Upload == xj_histroy_bill_info[i].status)
	    {
	        if(0 == strncmp(xj_histroy_bill_info[i].bill.charge_user_id,cmd_205->user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1))
	        {
	             _build_xj_cmd_202((xj_cmd_202*)_g_stack_var_for_callback_thread,&xj_histroy_bill_info[i].bill);
	        }
	    }
	}
	_xj_mqtt_send_resp_wrap(xj_cmd_type_206,_g_stack_var_for_callback_thread,sizeof(xj_cmd_206),1);
}

void _cmd_callback_stop_order(void* arg)
{
   	xj_cmd_11* cmd_11 = (xj_cmd_11*)arg;

    xj_add_incident_handle(&xj_incident_info,xj_incident_type_stop_charge,(uint8_t*)cmd_11,sizeof(xj_cmd_11));

}


void _cmd_callback_lock_control_order(void* arg)
{
    xj_add_incident_handle(&xj_incident_info,xj_incident_type_lock_control,(uint8_t*)arg,sizeof(xj_cmd_23));
}


void _cmd_callback_105_set_time(void* arg)
{
    xj_cmd_105* cmd_105 = (xj_cmd_105*)arg;
    xj_sync_system_time_param time_param;

    memset((char*)&time_param,00,sizeof(time_param));
    _convert_time_back((char*)cmd_105->time,&time_param);
    xj_add_incident_handle(&xj_incident_info,xj_incident_type_set_time,(uint8_t*)&time_param,sizeof(xj_sync_system_time_param));
}





void _cmd_callback_cmd_501(void* arg)
{
    xj_cmd_502 revert;
	xj_cmd_501* cmd_501 = (xj_cmd_501*)arg;
    uint16_t p_data = 0;//当前指向
    uint16_t param_num = 0;//设置参数地址
    uint8_t Legal_flag = CODE502_NO_ERROR;//数据合法标志
   // uint8_t reconne_flag = FALSE;//重连标志  5/7/8设置完要重启，通过平台code5命令控制

    memset((char*)&revert,00,sizeof(revert));
    revert.serial_code = cmd_501->serial_code;
    memcpy(revert.equipment_id,g_xj_params.equipment_id,XJ_MAX_EQUIPMENT_ID_LEN -1);

    for(p_data = 4 ;(cmd_501->cmd_len - p_data) > 2;p_data += (2 + get_params_length(set_501_param_list[param_num].param_type)))
    {
        param_num = cmd_501->data[p_data] + ((uint16_t)cmd_501->data[p_data + 1] >> 8);

        if(0 == param_num || param_num >= ARRAY_SIZE(set_501_param_list))
        {
            xj_pal_print_log(xj_log_remind,"set 501 code param addr %d error!\n",param_num);
            Legal_flag = CODE502_ADDR_ERROR;
            break;//参数地址不在列表
        }

        if((cmd_501->cmd_len - p_data) >= 2 + get_params_length(set_501_param_list[param_num].param_type))//长度包含参数地址长度和参数长度
        {
            if(NULL != set_501_param_list[param_num].p_param_max)//最大值判断
            {
                if(TRUE != judge_param_range(&cmd_501->data[p_data + 2],set_501_param_list[param_num].p_param_max,set_501_param_list[param_num].param_type,RANGE_MAX_TYPE))
                {
                    xj_pal_print_log(xj_log_remind,"set 501 code param out of max range!\n");
                    Legal_flag = CODE502_MAX_RANGE_ERROR;
                    break;
                }
            }

            if(NULL != set_501_param_list[param_num].p_param_min)//最小值判断
            {
                if(TRUE != judge_param_range(&cmd_501->data[p_data + 2],set_501_param_list[param_num].p_param_min,set_501_param_list[param_num].param_type,RANGE_MIN_TYPE))
                {
                    xj_pal_print_log(xj_log_remind,"set 501 code param out of min range!\n");
                    Legal_flag = CODE502_MIN_RANGE_ERROR;
                    break;
                }
            }

            if( g_xj_params.gun_qr_code == set_501_param_list[param_num].p_param)//设置二维码地址由枪号+二维码组成
            {
                uint16_t gun_num = cmd_501->data[p_data + 2] + ((uint16_t)cmd_501->data[p_data + 3] >> 8) -1;
                //if(gun_num > CHARGE_GUN_CNT)
                if(gun_num > _g_xj_params->gun_cnt)
                {
                    xj_pal_print_log(xj_log_remind,"set 501 qr code gun error，set gun = %d!\n",gun_num);
                    Legal_flag = CODE502_OTHER_ERROR;
                    break;
                }

            }
            else if( g_xj_params.equipment_id == set_501_param_list[param_num].p_param)
            {
                uint8_t gun = 0;

                for(gun = 0;gun < g_xj_params.gun_cnt;gun++)
                {
                    if(xj_work_stat_type_idle != g_xj_gun_status[gun].work_stat)
                    {
                        xj_pal_print_log(xj_log_warning,"gun:%d ,ID setting is not supported in the current state",gun+1);
                        Legal_flag = CODE502_OTHER_ERROR;
                        break;
                    }
                }
            }
            else if(g_xj_params.logic_svr_addr == set_501_param_list[param_num].p_param)
            {
                /*屏蔽,防止断线重连出错*/
                continue;
            }
            else if((void *)(&g_xj_params.logic_svr_port) == set_501_param_list[param_num].p_param)
            {
                continue;
            }

            if(CODE502_NO_ERROR == Legal_flag)//数据检查合法 //写入数据
        	{

                if( g_xj_params.gun_qr_code == set_501_param_list[param_num].p_param)//设置二维码地址由枪号+二维码组成
                {
                    uint16_t gun_num = cmd_501->data[p_data + 2] + ((uint16_t)cmd_501->data[p_data + 3] >> 8) - 1;
                    memcpy((char*)&g_xj_params.gun_qr_code[gun_num][0],(char*)&cmd_501->data[p_data+4],get_params_length(set_501_param_list[param_num].param_type)-2);
                }
                else
                {
                    memcpy((char*)set_501_param_list[param_num].p_param,(char*)&cmd_501->data[p_data + 2],get_params_length(set_501_param_list[param_num].param_type));
                }
                revert.success_number++;
        	}
        }
        else
        {
            xj_pal_print_log(xj_log_remind,"set 501 code param length error!\n");
            Legal_flag = CODE502_LEN_ERROR;
            break;//长度不符

        }

    }

	if(CODE502_NO_ERROR == Legal_flag && 0 != (cmd_501->cmd_len - p_data))//多了1个字节情况
	{
	    Legal_flag = CODE502_LEN_ERROR;
	}

    revert.set_result = Legal_flag;

    if(revert.success_number > 0)
    {
        _persist_xj_params();//写入FLASH
    }

  	_xj_mqtt_send_resp_wrap(xj_cmd_type_502,(char*)&revert,sizeof(xj_cmd_502),1);


}


void _cmd_callback_cmd_503(void* arg)
{
    xj_cmd_504 revert;
	xj_cmd_503* cmd_503 = (xj_cmd_503*)arg;
    uint16_t p_data = 0;//当前指向

    revert.set_result = CODE503_OTHER_ERROR;
	revert.serial_code = cmd_503->serial_code;
	memcpy(revert.equipment_id,g_xj_params.equipment_id,XJ_MAX_EQUIPMENT_ID_LEN -1);

    for(p_data = 0;(cmd_503->data_len - p_data) > 1 ;p_data += 5)
    {
        //if((cmd_503->data[p_data]-1) < CHARGE_GUN_CNT)
        if((cmd_503->data[p_data]-1) < _g_xj_params->gun_cnt)
        {
            if((cmd_503->data_len - p_data) >= 5)//参数必须4个以上
            {
                revert.set_result = CODE503_NO_ERROR;
                continue;
            }
            else
            {
                 xj_pal_print_log(xj_log_remind,"set 503 code len error!\n");
                revert.set_result = CODE503_LEN_ERROR;
                break;//长度错误
            }
        }
        else
        {
            xj_pal_print_log(xj_log_remind,"set 503 code gun number error!\n");
            revert.set_result = CODE503_GUN_ERROR;
            break;//枪号错误
        }
    }

    if(CODE503_NO_ERROR == revert.set_result)
    {
        for(p_data = 0;(cmd_503->data_len - p_data) > 1 ;p_data += 5)
        {
            g_xj_params.max_power[cmd_503->data[p_data] - 1] = cmd_503->data[p_data+1] + ((uint32_t)cmd_503->data[p_data+2] <<8) + ((uint32_t)cmd_503->data[p_data+3]<<16)+ ((uint32_t)cmd_503->data[p_data+4]<<24);
        }
        _persist_xj_params();//写入FLASH
    }

	_xj_mqtt_send_resp_wrap(xj_cmd_type_504,(char*)&revert,sizeof(xj_cmd_504),1);

}

void _cmd_callback_software_upgrade(void* arg)
{
	xj_cmd_1101* cmd_1101 = (xj_cmd_1101*)arg;
    xj_software_upgrade_param   param;

    memset((char*)&param,00,sizeof(xj_software_upgrade_param));
    param.serial_code = cmd_1101->serial_code;
	param.upgrade_type = cmd_1101->soft_type;
	param.target_type = cmd_1101->soft_param;
	memcpy(param.url, cmd_1101->download_url,128) ;
	memcpy(param.md5, cmd_1101->md5,32) ;

    xj_add_incident_handle(&xj_incident_info,xj_incident_type_software_upgrade,(uint8_t*)&param,sizeof(xj_software_upgrade_param));
}

void _cmd_callback_splited_charge_fee_config(void* arg){
	int i=0;
	xj_cmd_1303* cmd_1303 = (xj_cmd_1303*)arg;
	memset(_g_stack_var_for_callback_thread,0,sizeof(xj_cmd_1304));
	if(cmd_1303->cmd_type==1)
	{
		for(i=0;i<48;i++)
		{
			_g_xj_params->fee_config.charge_fee[i]=cmd_1303->fee_data[i];
		}
		_persist_xj_params();
	}

	for(i=0;i<48;i++){
		((xj_cmd_1304*)(_g_stack_var_for_callback_thread))->fee_data[i] = _g_xj_params->fee_config.charge_fee[i];
	}
	_xj_mqtt_send_resp_wrap(xj_cmd_type_1304,_g_stack_var_for_callback_thread,sizeof(xj_cmd_1304),1);
}

void _cmd_callback_splited_service_fee_config(void* arg){
	xj_cmd_1305* cmd_1305 = (xj_cmd_1305*)arg;
	int i=0;
	memset(_g_stack_var_for_callback_thread,0,sizeof(xj_cmd_1306));
	if(cmd_1305->cmd_type==1){
		for(i=0;i<48;i++){
			_g_xj_params->fee_config.service_fee[i]=cmd_1305->fee_data[i];
		}
		_persist_xj_params();
	}

	((xj_cmd_1306*)(_g_stack_var_for_callback_thread))->gun_index = cmd_1305->gun_index;
	for(i=0;i<48;i++){
		((xj_cmd_1306*)(_g_stack_var_for_callback_thread))->fee_data[i] = _g_xj_params->fee_config.service_fee[i];
	}
	_xj_mqtt_send_resp_wrap(xj_cmd_type_1306,_g_stack_var_for_callback_thread,sizeof(xj_cmd_1306),1);
}


void _cmd_callback_1309_cmd(void* arg)
{
    uint8_t i = 0;
	uint32_t sum_class = 0;
	xj_cmd_1310 revert;
	xj_cmd_1309* cmd_1309 = (xj_cmd_1309*)arg;

	memset((char*)&revert,0,sizeof(xj_cmd_1310));
    revert.serial_code = cmd_1309->serial_code;

	if(cmd_1309->class_num * 16 == cmd_1309->data_len)
	{
	    if(0 != cmd_1309->class_num)
	    {
	        for(i = 0;i < cmd_1309->class_num ;i++)
	        {
	            sum_class += cmd_1309->data[16*i + 2] + ((uint16_t)cmd_1309->data[16*i + 3] << 8);
	        }
	        if(ARRAY_SIZE(_g_xj_params->fee_config.charge_fee) == sum_class )
	        {
	            revert.set_result = CODE1301_NO_ERROR;
	        }
	        else
	        {
	            revert.set_result = CODE1301_CLASS_ERROR;
	        }
	    }
	    else
	    {
	        revert.set_result = CODE1301_CLASS_NUM_ERROR;
	    }
	}
	else
	{
	    revert.set_result = CODE1301_LEN_ERROR;
	}


    if(CODE1301_NO_ERROR == revert.set_result)
    {
        uint8_t same_number = 0;
        uint8_t location = 0;
        for(i = 0;i < cmd_1309->class_num ;i++)
        {
            same_number += cmd_1309->data[16*i + 2] + ((uint16_t)cmd_1309->data[16*i + 3] << 8);
            for( ;location < same_number;location++)
            {
                _g_xj_params->fee_config.charge_fee[location] = cmd_1309->data[16*i + 4] + ((uint32_t)cmd_1309->data[16*i + 5] << 8) + ((uint32_t)cmd_1309->data[16*i + 6] << 16) + ((uint32_t)cmd_1309->data[16*i + 7] << 24);
                _g_xj_params->fee_config.service_fee[location] = cmd_1309->data[16*i + 8] + ((uint32_t)cmd_1309->data[16*i + 9] << 8) + ((uint32_t)cmd_1309->data[16*i + 10] << 16) +((uint32_t)cmd_1309->data[16*i + 11] << 24);
                _g_xj_params->fee_config.demurrage[location] = cmd_1309->data[16*i + 12] + ((uint32_t)cmd_1309->data[16*i + 13] << 8) + ((uint32_t)cmd_1309->data[16*i + 14] << 16) +((uint32_t)cmd_1309->data[16*i + 15] << 24);
            }
        }
        _persist_xj_params();
        xj_pal_print_log(xj_log_remind,"Updata fee config!\n");
    }
	_xj_mqtt_send_resp_wrap(xj_cmd_type_1310,(char*)&revert,sizeof(xj_cmd_1310),1);
}

void _cmd_callback_upload_log(void* arg)
{
	xj_cmd_409* cmd_409 = (xj_cmd_409*)arg;
	xj_upload_log_param param;

    memset((char*)&param,00,sizeof(xj_upload_log_param));
    snprintf(param.upload_url, XJ_UPLOAD_LOG_URL_LEN, "http://unicronlogic-rest.didichuxing.com/logUpload") ;
    xj_add_incident_handle(&xj_incident_info,xj_incident_type_upload_log,(char*)&param,sizeof(xj_upload_log_param));
	return;
}

void* _on_cmd_received_callback(void* cmd,xj_mqtt_cmd_enum cmd_type)
{
    xj_pal_print_log(xj_log_message,"Handle with cmd:%d!\n",cmd_type);

	switch(cmd_type)
	{
		case xj_cmd_type_1:
        	//_cmd_callback_cmd_1(cmd);
			break;
		case xj_cmd_type_3:
			//_cmd_callback_cmd_3(cmd);
			break;
		case xj_cmd_type_7:
			_cmd_callback_start_charge(cmd);
			break;
		case xj_cmd_type_5:
            _cmd_callback_control_cmd(cmd);
			break;
		case xj_cmd_type_11:
        	_cmd_callback_stop_order(cmd);
			break;
	    case xj_cmd_type_23:
        	_cmd_callback_lock_control_order(cmd);
			break;
	    case xj_cmd_type_105:
        	_cmd_callback_105_set_time(cmd);
			break;
		case xj_cmd_type_205:
            _cmd_callback_recent_bill_info(cmd);
        	break;
		case xj_cmd_type_409:
        	_cmd_callback_upload_log(cmd);
			break;
		case xj_cmd_type_501:
        	_cmd_callback_cmd_501(cmd);
			break;
		case xj_cmd_type_503:
        	_cmd_callback_cmd_503(cmd);
			break;
		case xj_cmd_type_1101:
        	_cmd_callback_software_upgrade(cmd);
			break;
		case xj_cmd_type_1303:
        	//_cmd_callback_splited_charge_fee_config(cmd);  使用1309
			break;
		case xj_cmd_type_1305:
        	//_cmd_callback_splited_service_fee_config(cmd);
			break;
		case xj_cmd_type_1309:
        	_cmd_callback_1309_cmd(cmd);
			break;
		default:
			break;
	}
	return NULL;
}



/********************************************************
  * @Description：发送vin码启动充电请求
  * @Arguments	：
                gun_cnt:触发的枪号 ，必须小于枪总数
			    0表示枪1 ,1表示枪2,以此类推
			    vin:车辆vin字符串，最大17位
  * @Returns	：
                0:请求成功，将在最长10秒内收到鉴权结果回调
               -1:请求失败
 *******************************************************/
int8_t xj_send_vin_start_charge_request(uint8_t gun_cnt,uint8_t* vin)
{
    int8_t result = -1;
    xj_cmd_40 data;

    //if(gun_cnt >= CHARGE_GUN_CNT)
    if(gun_cnt >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_vin_start_charge_request  gun_cnt-param error  gun_cnt =%d\n",gun_cnt);
    }
    else if(NULL == vin)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_vin_start_charge_request  vin-param error NULL\n");
    }
    else if(xj_work_stat_type_gun_pluged_in != g_xj_gun_status[gun_cnt].work_stat)
    {
        xj_pal_print_log(xj_log_Error,"Func:xj_send_vin_start_charge_request .The gun is not currently inserted!\n");
    }
    else if(1 != _g_is_login)
    {
        xj_pal_print_log(xj_log_warning,"Unconnected server! Unable to trigger--xj_send_vin_start_charge_request\n");
    }
    else
    {
        memset((char*)&data,00,sizeof(data));
        data.gun_index = gun_cnt;
        strncpy(data.vin,vin,sizeof(data.vin));
        result = xj_add_incident_handle(&xj_incident_info,xj_incident_type_vin_start_charg,(uint8_t*)&data,sizeof(data));
    }
    return result;
}



void _convert_time(int32_t year,int32_t month,int32_t day,int32_t hour,int32_t minute,int32_t sec, char* bcd_time){
	char tmp[4];
	uint8_t hex = 0;
	int year_high = year/100;
	int year_low = year%100;
	memset(tmp,0,4);
	snprintf(tmp,4,"%d",year_high);
	hex = strtol(tmp, NULL, 16);
	bcd_time[0]=hex;

	memset(tmp,0,4);
	snprintf(tmp,4,"%d",year_low);
	hex = strtol(tmp, NULL, 16);
	bcd_time[1]=hex;

	memset(tmp,0,4);
	snprintf(tmp,4,"%d",month);
	hex = strtol(tmp, NULL, 16);
	bcd_time[2]=hex;

	memset(tmp,0,4);
	snprintf(tmp,4,"%d",day);
	hex = strtol(tmp, NULL, 16);
	bcd_time[3]=hex;

	memset(tmp,0,4);
	snprintf(tmp,4,"%d",hour);
	hex = strtol(tmp, NULL, 16);
	bcd_time[4]=hex;

	memset(tmp,0,4);
	snprintf(tmp,4,"%d",minute);
	hex = strtol(tmp, NULL, 16);
	bcd_time[5]=hex;

	memset(tmp,0,4);
	snprintf(tmp,4,"%d",sec);
	hex = strtol(tmp, NULL, 16);
	bcd_time[6]=hex;

	memset(tmp,0,4);
	strncpy(tmp,"ff",2);
	hex = strtol(tmp, NULL, 16);
	bcd_time[7]=hex;
}

void _convert_time_back(char* bcd_time,xj_sync_system_time_param* time_param){
	char tmp[3];
	int year_high = 0;
	int year_low = 0;
	/*year*/
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[0]);
	year_high = strtol(tmp,NULL,10);
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[1]);
	year_low = strtol(tmp,NULL,10);
	time_param->year = year_high*100+year_low;
	/*month*/
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[2]);
	time_param->month = strtol(tmp,NULL,10);
	/*day*/
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[3]);
	time_param->day = strtol(tmp,NULL,10);
	/*hour*/
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[4]);
	time_param->hour = strtol(tmp,NULL,10);
	/*minute*/
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[5]);
	time_param->minute = strtol(tmp,NULL,10);
	/*minute*/
	memset(tmp,0,3);
	snprintf(tmp,3,"%x",bcd_time[6]);
	time_param->sec = strtol(tmp,NULL,10);
}

void _persist_xj_params()
{
	xj_pal_write_persist_params((char*)_g_xj_params,sizeof(xj_params));
}

void _restore_persisted_xj_params(xj_params* p)
{
	if(0 != xj_pal_read_persist_params((char*)p,sizeof(xj_params)))
	{
        xj_pal_print_log(xj_log_warning,"Read xj_params failure.Use default values!\n");
	}
}





/********************************************************
  * @Description：获取指定时段的电费
  * @Arguments	：
                time：要获取的时段。将24小时分成48个时段，每半小时为1段。
                    取值为0-47

  * @Returns	： 该时段电费   ，单位分
 *******************************************************/
uint16_t xj_get_charge_fee(uint8_t time)
{
    uint16_t free = 0;

    if(time < 48)
    {
        free = g_xj_params.fee_config.charge_fee[time];
    }
    return free;
}


/********************************************************
  * @Description：获取指定时段的服务费
  * @Arguments	：
                time：要获取的时段。将24小时分成48个时段，每半小时为1段。
                    取值为0-47

  * @Returns	： 该时段服务费，单位分
 *******************************************************/
uint16_t xj_get_service_fee(uint8_t time)
{
    uint16_t free = 0;

    if(time < 48)
    {
        free = g_xj_params.fee_config.service_fee[time];
    }
    return free;
}


/********************************************************
  * @Description：获取指定时段的延误费
  * @Arguments	：
                time：要获取的时段。将24小时分成48个时段，每半小时为1段。
                    取值为0-47

  * @Returns	： 该时段延误费，单位分
 *******************************************************/
uint16_t xj_get_demurrage(uint8_t time)
{
    uint16_t free = 0;

    if(time < 48)
    {
        free = g_xj_params.fee_config.demurrage[time];
    }
    return free;
}

/********************************************************
  * @Description：获取指定枪的最大功率
  * @Arguments	：
                gun:枪号

  * @Returns	： 该枪的最大功率
 *******************************************************/
 uint32_t xj_get_maxpower(uint8_t gun)
 {
    if(gun < g_xj_params.gun_cnt)
    {
        return g_xj_params.max_power[gun];
    }
    else
    {
        return  0;
    }
 }

/********************************************************
  * @Description：根据桩号设置二维码
  * @Arguments	：

  * @Returns	：
 *******************************************************/
static void set_qr(void)
{
    uint8_t gun = 0;
    uint8_t QR[128];/* 50 overflow */
    char err[XJ_MAX_ERR_NO_LEN];

    for(gun = 0;gun < g_xj_params.gun_cnt;gun++)
    {
        memset(QR,00,sizeof(QR));
        memset(err,00,XJ_MAX_ERR_NO_LEN);
        QR_code_String(QR,g_xj_params.equipment_id,gun+1);
        callback_display_qr(gun,QR,strlen(QR),err);
        if(0 != strncmp(err,XJ_SUCCEED_RESULT,XJ_ERR_RESULT_LEN))
        {
             xj_pal_print_log(xj_log_Error,"gun:%d Qr code setting failed\n",gun+1);
        }
    }
}


/********************************************************
  * @Description：二维码字符生成
  * @Arguments	：
			    QR          存储字符的指针
			    charge      桩号
			    gun_num     枪号

  * @Returns	：
                错误:         0
                正确  ：       长度

 *******************************************************/
uint8_t QR_code_String(char* QR ,const char*charge,const uint8_t gun_num)
{
    if(NULL != QR && NULL != charge && gun_num < 100)
    {
        //sprintf(QR,"hlht://%s%02d.101437000/",charge,gun_num);
        sprintf(QR,"https://epower.xiaojukeji.com/epower/static/resources/xcxconf/XIAOJU.101437000.%s%02d",charge,gun_num);
        return strlen(QR);
    }
    return 0;
}



/********************************************************
  * @Description：判断大小端
  * @Arguments	：
  * @Returns	：
                0:小端
                1大端
 *******************************************************/
char Check_cpu(void)
{
    int a = 1;
    char *p = (char *)&a;
    if(*p == 1)
    {
        return _LITTLE_MODE_;
    }
    else
    {
        return _BIG_MODE_;
    }
}


/********************************************************
  * @Description：计算字母或数字的长度
  * @Arguments	：
                data：计算的字符指针
  * @Returns	：
                -1:计算错误
                其它：长度
 *******************************************************/
int Come_letter_num_len(uint8_t*data)
{
    uint32_t len = 0;
    if(NULL != data)
    {
        while(1)
        {
            if( ('0' <= *(data+len) && '9' >= *(data+len))
            || ('a' <= *(data+len) && 'z' >= *(data+len))
            || ('A' <= *(data+len) && 'Z' >= *(data+len)))
            {
                len++;
            }
            else
            {
                break;
            }
        }
        return len;
    }
    return -1;
}


/********************************************************
  * @Description：日志输出
  * @Arguments	：
			    type  日志的类型
			    可变参数
  * @Returns	：

 *******************************************************/
void xj_pal_print_log(xj_log_type type, char* format, ... )
{
#if XJ_LOG_RECORD_SWITCH
    char temp_str[TEMP_LOG_SIZE];
    int i = 0;
    int out_size = 0;
    va_list vArgList; //定义一个va_list型的变量,这个变量是指向参数的指针.

    memset(temp_str,00,TEMP_LOG_SIZE);

    switch(type)
    {
        case xj_log_message:
        {
            snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:信息:",
            xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
            xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec());
            break;
        }
        case xj_log_remind:
        {
            snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:提示:",
            xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
            xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec());
            break;
        }
        case xj_log_warning:
        {
            snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:警告:",
            xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
            xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec());
            break;
        }
        case xj_log_Error:
        {
            snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:错误:",
            xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
            xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec());
            break;
        }
        default:
        {
            break;
        }
   }
    i = strlen(temp_str);
    va_start(vArgList, format);//用va_start宏初始化变量,这个宏的第二个参数是第一个可变参数的前一个参数,是一个固定的参数
    out_size = vsnprintf((temp_str + i),(TEMP_LOG_SIZE - i), format, vArgList); //注意,不要漏掉前面的_
    va_end(vArgList); //用va_end宏结束可变参数的获取

    if(out_size < 0)
    {
        snprintf(temp_str,TEMP_LOG_SIZE,"Log output error!!!");
        out_size = strlen(temp_str);
    }

    User_log_export(temp_str,out_size);
  //  MQTT_PAL_MUTEX_UNLOCK(&print_log_lock);
#endif
}

/********************************************************
  * @Description：日志输出
  * @Arguments	：
			    type  日志的类型
			    module_name 模块名称
			    status 模块状态
			    可变参数
  * @Returns	：

 *******************************************************/
void xj_print_log(uint8_t type,uint8_t * module_name,uint8_t status,char* format, ... )
{
#if XJ_LOG_RECORD_SWITCH
    char temp_str[TEMP_LOG_SIZE];
    int i = 0;
    int out_size = 0;
    va_list vArgList; //定义一个va_list型的变量,这个变量是指向参数的指针.

    memset(temp_str,00,TEMP_LOG_SIZE);

    switch(g_xj_params.log_level)
    {
        case LOG_LEVEL_NULL:
        case LOG_LEVEL_MESSAGE:
        {
            if(LOG_LEVEL_MESSAGE == type)
            {
                snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:Message:%s %d ",
                xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
                xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec(),module_name,status);
            }
        }
        case LOG_LEVEL_DEBUG:
        {
            if(LOG_LEVEL_DEBUG == type)
            {
                snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:Debug:%s %d ",
                xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
                xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec(),module_name,status);
            }
        }
        case LOG_LEVEL_WARING:
        {
            if(LOG_LEVEL_WARING == type)
            {
                snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:Waring:%s %d ",
                xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
                xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec(),module_name,status);
            }
        }
        case LOG_LEVEL_ERROR:
        {
            if(LOG_LEVEL_ERROR == type)
            {
                snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:Error:%s %d ",
                xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
                xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec(),module_name,status);
            }
            break;
        }
        default:
        {
            snprintf(temp_str,TEMP_LOG_SIZE,"%04d-%02d-%02d %02d:%02d %02d:log level error:",
                xj_pal_get_int_year(),xj_pal_get_int_month(),xj_pal_get_int_day(),
                xj_pal_get_int_hour(),xj_pal_get_int_minute(),xj_pal_get_int_sec());
        }
   }
    i = strlen(temp_str);
    va_start(vArgList, format);//用va_start宏初始化变量,这个宏的第二个参数是第一个可变参数的前一个参数,是一个固定的参数
    out_size = vsnprintf((temp_str + i),(TEMP_LOG_SIZE - i), format, vArgList); //注意,不要漏掉前面的_
    va_end(vArgList); //用va_end宏结束可变参数的获取

    if(out_size < 0)
    {
        snprintf(temp_str,TEMP_LOG_SIZE,"Log output error!!!");
        out_size = strlen(temp_str);
    }

    User_log_export(temp_str,out_size);

#endif
}


/********************************************************
  * @Description：TCP报文打印
  * @Arguments	：
			    buff  数据
			    len   长度
			    type  数据方向
  * @Returns	：

 *******************************************************/
void Printf_TCP_Log(char *buff,int len,char type)
{
    int i = 0;
	char temp_buff[64];
	memset(temp_buff,00,sizeof(temp_buff));

    if(len > 0 && (_TCP_SEND_ == type ||  _TCP_RECEIVE_ == type))
    {
        if(_TCP_SEND_ == type)
        {
            sprintf(temp_buff,"TCP Send size = %d,HEX data :\n",len);
        }
        else
        {
            sprintf(temp_buff,"TCP Receive size = %d,HEX data :\n",len);
        }

        xj_pal_print_log(xj_log_message,temp_buff);
        for(i = 0;i<len;i++)
        {
            sprintf(temp_buff,"%02X ",*(uint8_t*)(buff + i));
            xj_pal_print_log(xj_log_Null,temp_buff);
        }
        sprintf(temp_buff,"\r\n");
        xj_pal_print_log(xj_log_Null,temp_buff);
    }
}

/********************************************************
  * @Description：计算最大公约数
  * @Arguments	：
			    a  数a
			    b  数b
  * @Returns	：
                a，b最大公约数
 *******************************************************/
uint32_t GCD(uint32_t a, uint32_t b)
{
    if(b == 0)
        return a;
    else
        return GCD(b, a%b);
}

/********************************************************
  * @Description：计算最小公倍数
  * @Arguments	：
			    a  数a
			    b  数b
  * @Returns	：
                a，b最小公倍数
 *******************************************************/
uint32_t LCM(uint32_t a, uint32_t b)
{
    if(a * b == 0)
        return 0;
    return (a*b)/GCD(a,b);//使用公式
}

/********************************************************
  * @Description：参数初始化
  * @Arguments	：
			    gun_status  用户枪状态结构指针
			    bms_status  用户枪状态结构指针
  * @Returns	：
 *******************************************************/
static void System_param_init(st_user_gun_info* gun_status, st_user_bms_info* bms_status,uint8_t gun_cnt)
{
    xj_helper_init_xj_params(&g_xj_params,gun_cnt);
#if defined(XJ_ERR_CACHE_USED)
    err_event_info_init();
#endif // defined
    xj_gun_bms_status_init(g_xj_params.gun_cnt);
    xj_init(&g_xj_params,gun_status,bms_status);
	_persist_xj_params();
	xj_read_history_bill();
}


/********************************************************
  * @Description：同步小桔数据
  * @Arguments	：
  * @Returns	：
 ********************************************************/
void* sync_xiaoju_status(void* args)
{
    for(;;)
    {
        sync_gun_status(&g_xj_gun_status[0],p_User_gun_info,g_xj_params.gun_cnt);
        sync_bms_status(&g_xj_bms_status[0],p_User_bms_info,g_xj_params.gun_cnt);
        xj_pal_msleep(1000);
    }

    return NULL;
}


/********************************************************
  * @Description：重连事件处理
  * @Arguments	：
			    param  参数
  * @Returns	：

 *******************************************************/
static void reconnection_handle(xj_params* param)
{
    if(NULL != param)
    {
        char Connect_start = ECO_FALSE;
        xj_mqtt_disconnect();

        xj_pal_print_log(xj_log_remind,"disconncted! retry login in 10 seconds.\n");
    	do{
    		xj_pal_msleep(10 * 1000 );
    		Connect_start = xj_mqtt_connect(param->logic_svr_addr,param->logic_svr_port,param->username,param->password,param->equipment_id,_on_cmd_received_callback);
    		if(ECO_FALSE == Connect_start)
        	{
                xj_pal_print_log(xj_log_warning,"failed to connect to logic svr!,retry in 10 seconds.\n");
        	}
    		else
    		{
    		    _interval_work_for_sign_in(TRUE);
    		}
    	}
    	while(ECO_TRUE != Connect_start);
    	_g_is_login = 1;
        xj_pal_print_log(xj_log_remind,"re-login xiaoju succeeded!\n");
    }
    else
    {
        xj_pal_print_log(xj_log_Error,"Reconnection handle.param addr error!!!\n");
    }
    #if defined(XJ_ERR_CACHE_USED)
    err_event_info_clr();
    #endif // defined
}

/********************************************************
  * @Description：处理事件
  * @Arguments	：
			    info  队列表
  * @Returns	：

 *******************************************************/
void xj_incident_info_handle(st_xj_incident_info* info)
{
    uint8_t* param = info->incident_list[info->incident_new].incident_data;
    uint16_t param_len = info->incident_list[info->incident_new].incident_len;

    if(info->incident_sum > 0)
    {
        xj_pal_print_log(xj_log_message,"Start handle incident=%d\n",info->incident_list[info->incident_new].incident_name);
        switch(info->incident_list[info->incident_new].incident_name)
        {
            case xj_incident_type_reconnection:
            {
                reconnection_handle(&g_xj_params);
                break;
            }
            case xj_incident_type_set_left_qr://code 3 cmd 17
            {
                xj_set_gun_qr(0,param,param_len);
                break;
            }
            case xj_incident_type_set_righ_qr://code 3 cmd 19
            {
                xj_set_gun_qr(1,param,param_len);
                break;
            }
            case xj_incident_type_start_charge://code 7
            {
                xj_start_charge((xj_start_charge_param*)param);
                break;
            }
            case xj_incident_type_stop_charge://code 11
            {
                xj_stop_charge((xj_cmd_11*)param);
                break;
            }
            case xj_incident_type_set_time://code 105
            {
                xj_set_sys_time((xj_sync_system_time_param*)param);
                break;
            }
            case xj_incident_type_software_upgrade://code 1101
            {
                xj_software_download((xj_software_upgrade_param*)param);
                break;
            }
            case xj_incident_type_params_updated://code 1 or 3
            {
                //xj+++ 已取消code1 code3
                break;
            }
            case xj_incident_type_upload_log://cpde 409
            {
                xj_upload_log((xj_upload_log_param*)param);
                break;
            }
            case xj_incident_type_control_cmd://code 5
            {
                xj_code5_control_handle((xj_control_cmd_param*)param);
                break;
            }
            case xj_incident_type_lock_control:
            {
                xj_code23_lock_control((xj_cmd_23*)param);
                break;
            }
            case xj_incident_type_vin_start_charg:
            {
                xj_send_touch_vin_charge_reques(((xj_cmd_40*)param)->gun_index,((xj_cmd_40*)param)->vin);
                break;
            }
            case xj_incident_type_send_108:
            {
                xj_send_event(((st_108_cmd*)param)->gun_cnt,((st_108_cmd*)param)->type,((st_108_cmd*)param)->event_param);
                break;
            }
            case xj_incident_type_send_304:
            {
                xj_incident_send_304((xj_cmd_304*)param);
                break;
            }
            case xj_incident_type_send_312:
            {
                xj_incident_send_312((xj_cmd_312*)param);
                break;
            }
            case xj_incident_type_send_120:
            {
                xj_incident_send_120((xj_cmd_120*)param);
                break;
            }
            default:
            {
                xj_pal_print_log(xj_log_warning,"No way to handle the incident,incident_name = %d\n",info->incident_list[info->incident_new].incident_name);
                break;
            }
        }
        xj_pal_print_log(xj_log_message,"Incident=%d,resolved\n",info->incident_list[info->incident_new].incident_name);
        xj_del_recently_incident_handle(info);
    }

}

/********************************************************
  * @Description：xj事件处理线程
  * @Arguments	：
  * @Returns	：
 ********************************************************/
void* xj_incident_handle(void* args)
{
    XJ_PAL_MUTEX_INIT(&xj_incident_info.incident_lock);

    for(;;)
    {
        xj_incident_info_handle(&xj_incident_info);
        xj_pal_msleep(10);
    }

    XJ_PAL_MUTEX_DESTORY(&xj_incident_info.incident_lock);

    return NULL;
}



/********************************************************
  * @Description：消息处理                100ms
  * @Arguments	：
			    NULL
  * @Returns	：
 ********************************************************/
void* xj_message_handle(void* args)
{
    for(;;)
    {
        _interval_work();
        xj_pal_msleep(100);
    }

    return NULL;
}






/********************************************************
  * @Description：获取指定枪的订单号
  * @Arguments	：
			    gun 枪号
			    id  要存储订单号的指针    ，可存储长度要大于等于 XJ_MAX_CHAEGE_USER_ID_LEN
  * @Returns	：
                TRUE 获取成功
                FALSE 获取失败
 *******************************************************/
int8_t get_current_order_number(uint8_t gun,int8_t* id)
{
    //if(gun < CHARGE_GUN_CNT && NULL != id )
    if(gun < _g_xj_params->gun_cnt && NULL != id)
    {
        strncpy(id,g_xj_gun_status[gun].charge_user_id,XJ_MAX_CHAEGE_USER_ID_LEN-1);
        return TRUE;
    }
    else
    {
        return  FALSE;
    }
}


/********************************************************
  * @Description：获取指定枪的当前费用
  * @Arguments	：
			    gun 枪号
  * @Returns	：
                当前费用
 *******************************************************/
uint32_t get_current_fee(uint8_t gun)
{
    //if(gun < CHARGE_GUN_CNT)
    if(gun < _g_xj_params->gun_cnt)
    {
        return g_xj_gun_status[gun].cumulative_charge_fee;
    }
    else
    {
        return  0;
    }
}


/********************************************************
  * @Description：获取指定枪的当前电量
  * @Arguments	：
			    gun 枪号
  * @Returns	：
                -1 获取失败
                其它为当前电量
 *******************************************************/
int get_current_Kwh(uint8_t gun)
{
    //if(gun < CHARGE_GUN_CNT)
    if(gun < _g_xj_params->gun_cnt)
    {
        return _g_xj_current_bill_info[gun].charge_kwh_amount ;
    }
    else
    {
        return  -1;
    }
}



/********************************************************
  * @Description：获取安全围栏参数
  * @Arguments	：
  * @Returns	：
                st_safety_parameters:围栏参数指针
 *******************************************************/
const st_safety_parameters* get_xj_safety_parameters(void)
{
    return &g_xj_params.safety_parameters ;
}



/********************************************************
  * @Description：获取指定枪对应的二维码
  * @Arguments	：
			    gun 枪号
			    QR  要存储二维码字符串的指针   ，可存储长度要大于等于XJ_MAX_BAR_CODE_LEN
  * @Returns	：
                0 获取成功
                -1 获取失败
 *******************************************************/
int8_t get_gun_qr_code(uint8_t gun, int8_t* QR)
{
    //if(gun < CHARGE_GUN_CNT && NULL != QR)
    if(gun < _g_xj_params->gun_cnt && NULL != QR)
    {
        strncpy(QR,g_xj_params.gun_qr_code[gun],XJ_MAX_BAR_CODE_LEN-1);
        return 0;
    }
    else
    {
        return  -1;
    }
}


/*******************************************************
			    gun_status：用户枪状态结构指针
			    bms_status：用户BMS状态结构指针
  * @Returns	：
                0：启动成功
                -1 启动失败
 *******************************************************/

int8_t xj_APP_start(st_user_gun_info* gun_status, st_user_bms_info* bms_status,uint8_t gun_cnt)
{
  if(NULL != gun_status && NULL != bms_status)
	{
        //System_Storage = Check_cpu();
        System_param_init(gun_status,bms_status,gun_cnt);

        if(0 == xj_pal_thread_create("xjsdk-send/rcv",&mqttMessageBase.client_daemon,client_refresher,&mqttMessageBase.client,xj_osPriorityAboveNormal,XJ_OS_STACK_SIZE_LARGE))
        {
            xj_first_login();
            set_qr();
            if(0 == xj_pal_thread_create("xjsdk-incident",&xj_thread_incident,xj_incident_handle, NULL,xj_osPriorityNormal,XJ_OS_STACK_SIZE_MEDIUM))
            {
                if(0 ==xj_pal_thread_create("xjsdk-sync",&xj_thread_sync_status,sync_xiaoju_status, NULL,xj_osPriorityNormal,XJ_OS_STACK_SIZE_LITTLE))
                {
                    if(0 ==  xj_pal_thread_create("xjsdk-message",&xj_thread_message_handle,xj_message_handle, NULL,xj_osPriorityNormal,XJ_OS_STACK_SIZE_MEDIUM))
                    {
                        return 0;
                    }
                }
           }
        }
    }
    xj_pal_print_log(xj_log_Error,"xj_APP_start ERROR!!!\n");
    return -1;
}


/* add err code event to g_err_occurred_info */
void err_event_info_init(void)
{
    uint8_t pos = 0;
    uint8_t ii = 0;
    for(pos = 0;pos < (sizeof(g_err_event_info.cache) / sizeof(g_err_event_info.cache[0]));pos++)
    {
        g_err_event_info.cache[pos].get_index = 0;
        g_err_event_info.cache[pos].put_index = 0;


        for(ii = 0;ii < (sizeof(g_err_event_info.cache[pos].error_item) / sizeof(g_err_event_info.cache[pos].error_item[0]));ii++)
        {
            memset(&g_err_event_info.cache[pos].error_item[ii].err_code[0],'\0',sizeof(g_err_event_info.cache[pos].error_item[ii].err_code));
            g_err_event_info.cache[pos].error_item[ii].flag = 1;
        }
    }
    XJ_PAL_MUTEX_INIT(&g_err_event_info.errlist_lock);
}

int8_t err_event_info_push(int8_t flag,uint8_t gun_index,char *err_code)
{
    uint8_t item_count;
    uint8_t cur_pos;
    uint8_t *cur_errcode;
    uint8_t cur_flag = 0;

    if(gun_index >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_Error,"gun num error \n");
        return -1;
    }

    XJ_PAL_MUTEX_LOCK(&g_err_event_info.errlist_lock);

    item_count = sizeof(g_err_event_info.cache[gun_index].error_item) / sizeof(g_err_event_info.cache[gun_index].error_item[0]);

    /* is full */
    if(((g_err_event_info.cache[gun_index].put_index + 1) % item_count) == (g_err_event_info.cache[gun_index].get_index))
    {
        xj_pal_print_log(xj_log_Error,"queue is full\n");

        XJ_PAL_MUTEX_UNLOCK(&g_err_event_info.errlist_lock);

        return -1;
    }
#if 1
    /* is empty */
    if(g_err_event_info.cache[gun_index].get_index == g_err_event_info.cache[gun_index].put_index)
    {
        g_err_event_info.cache[gun_index].put_index = (g_err_event_info.cache[gun_index].put_index + 1) % item_count;

        memcpy(&g_err_event_info.cache[gun_index].error_item[g_err_event_info.cache[gun_index].put_index].err_code[0],err_code,4);

        g_err_event_info.cache[gun_index].error_item[g_err_event_info.cache[gun_index].put_index].flag = flag;

        XJ_PAL_MUTEX_UNLOCK(&g_err_event_info.errlist_lock);
        return 0;
    }
#endif
    /* find */
    cur_pos = g_err_event_info.cache[gun_index].get_index;
    /* cur_pos < put_index */

    /* is not full */
    for(;(cur_pos % item_count) != (g_err_event_info.cache[gun_index].put_index + 1) % item_count;cur_pos++)
    {
        cur_errcode = &g_err_event_info.cache[gun_index].error_item[cur_pos % item_count].err_code[0];

        if(cur_errcode[0] != '\0' && (strncmp(cur_errcode,err_code,4) == 0))
        {
            g_err_event_info.cache[gun_index].error_item[cur_pos % item_count].flag = flag;
            cur_flag = 1;

            break;
        }
    }


    if(cur_flag == 0)
    {
        /* no same err code */
        g_err_event_info.cache[gun_index].put_index = (g_err_event_info.cache[gun_index].put_index + 1) % item_count;

        memcpy(&g_err_event_info.cache[gun_index].error_item[g_err_event_info.cache[gun_index].put_index].err_code[0],err_code,4);


        g_err_event_info.cache[gun_index].error_item[g_err_event_info.cache[gun_index].put_index].flag = flag;
    }

    XJ_PAL_MUTEX_UNLOCK(&g_err_event_info.errlist_lock);

    return 0;
}

int8_t err_event_info_pop(int8_t flag,uint8_t gun_index,char *err_code)
{
    uint8_t item_count = 0;
    uint8_t *cur_errcode;
    uint8_t cur_pos;

    if(gun_index >= _g_xj_params->gun_cnt)
    {
        xj_pal_print_log(xj_log_Error,"gun num error \n");
        return -1;
    }

    XJ_PAL_MUTEX_LOCK(&g_err_event_info.errlist_lock);

    item_count = sizeof(g_err_event_info.cache[gun_index].error_item) / sizeof(g_err_event_info.cache[gun_index].error_item[0]);

    if(g_err_event_info.cache[gun_index].get_index == g_err_event_info.cache[gun_index].put_index)
    {
        /* is empty */
        xj_pal_print_log(xj_log_message,"queue is empty\n");

        XJ_PAL_MUTEX_UNLOCK(&g_err_event_info.errlist_lock);

        return -1;
    }

    cur_errcode = &g_err_event_info.cache[gun_index].error_item[((g_err_event_info.cache[gun_index].get_index + 1) % item_count)].err_code[0];
    if(cur_errcode[0] != '\0')
    {
        if(strncmp(cur_errcode,err_code,4) == 0)
        {
            memset(cur_errcode,'\0',4);

            g_err_event_info.cache[gun_index].error_item[((g_err_event_info.cache[gun_index].get_index + 1) % item_count)].flag = 1;

            g_err_event_info.cache[gun_index].get_index = (g_err_event_info.cache[gun_index].get_index + 1) % item_count;

        }
        else
        {
            /*收到的117和发送的不对应。？？？*/
            cur_pos = g_err_event_info.cache[gun_index].get_index;
            for(;(cur_pos % item_count) != g_err_event_info.cache[gun_index].put_index;cur_pos++)
            {
                cur_errcode = &g_err_event_info.cache[gun_index].error_item[cur_pos].err_code[0];
                if(cur_errcode && (strncmp(cur_errcode,err_code,4) == 0))
                {
                    memset(&g_err_event_info.cache[gun_index].error_item[cur_pos].err_code[0],'\0',sizeof(g_err_event_info.cache[gun_index].error_item[cur_pos].err_code[0]));
                }
            }
        }
    }

    XJ_PAL_MUTEX_UNLOCK(&g_err_event_info.errlist_lock);

    return 0;
}

void err_event_info_clr(void)
{
    uint8_t item_count = 0;
    uint8_t pos = 0;
    uint8_t ii = 0;
    uint8_t *current_err_code = NULL;
    xj_error error;

    /* occurred */
    item_count = sizeof(g_err_event_info.cache[0].error_item) / sizeof(g_err_event_info.cache[0].error_item[0]);
    for(pos = 0;pos < _g_xj_params->gun_cnt;pos++)
    {
        //XJ_PAL_MUTEX_LOCK(&g_err_event_info.errlist_lock);

        /* 防止while 进入死循环 */
        for(ii = 0;ii < item_count;ii++)
        {
            if(g_err_event_info.cache[pos].get_index == g_err_event_info.cache[pos].put_index)
            {
                /* is empty */
                xj_pal_print_log(xj_log_message,"[clr] queue is empty\n");
                break;
            }
            current_err_code = &g_err_event_info.cache[pos].error_item[((g_err_event_info.cache[pos].get_index + 1) % item_count)].err_code[0];
            if((current_err_code[0] != '\0') && (1 == _g_is_login))
            {
                error.err_flag = g_err_event_info.cache[pos].error_item[((g_err_event_info.cache[pos].get_index + 1) % item_count)].flag;
                memcpy(&error.err_no[0],current_err_code,4);
                error.gun_index = pos;

                if(xj_bool_true == xj_send_error(&error))
                {
                    #if 0
                    memset(current_err_code,'\0',4);
                    printf("current_error_code = %x %x %x %x \n",current_err_code[0],current_err_code[1],current_err_code[2],current_err_code[3]);

                    g_err_event_info.cache[pos].get_index = (g_err_event_info.cache[pos].get_index + 1) % item_count;
                    printf("get_index = %d \n",g_err_event_info.cache[pos].get_index);
                    #endif
                }
            }
        }

        //XJ_PAL_MUTEX_UNLOCK(&g_err_event_info.errlist_lock);
    }
}

