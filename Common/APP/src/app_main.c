#include "FreeRTOS.h"
#include "task.h"
#include "app_main.h"
#include "app_event.h"
#include "srv.h"
app_context_t app_context;
static void bt_sink_app_init_device_role(void);
#ifdef APP_NO_ACTION_AUTO_POWER_OFF
//TODO
#endif
void srv_event_callback(srv_event_t event_id, srv_event_param_t *param)
{
    void *event_params = NULL;
    if (NULL != param) {
        event_params = (srv_event_param_t *)pvPortMalloc(sizeof((*param)));
        if (NULL != event_params) {
            memcpy(event_params, param, sizeof(*param));
        } else {
            app_report("malloc fail");
        }
    }
    app_event_post(event_id, event_params, app_event_post_callback);
}
void app_task_main(void *arg)
{
    app_event_t event;
    //srv_features_config_t config;
    app_report("enter main");
    memset(&app_context, 0, sizeof(app_context_t));
    //initialize event
    app_event_init();
#ifdef APP_NO_ACTION_AUTO_POWER_OFF
    //TODO
#endif
    app_context.queue_handle = xQueueCreate(APP_QUEUE_SIZE, sizeof(app_event_t));
    //app_event_register_callback(EVENT_APP_KEY_INPUT, app_keypad_event_handler);
    app_event_register_callback(SRV_EVENT_ALL, app_event_handler);
    //app_atci_init();
    //app_keypad_init();
    // init sink app role
    app_init_device_role();
    app_context.feature_config.features = NULL;
    srv_init(&(app_context.feature_config));
    
    while (1) {
        if (pdPASS == xQueueReceive(app_context.queue_handle, &event, portMAX_DELAY)) {
            app_event_process(&event);
        }
    }
}
void app_task_create(void)
{
    xTaskCreate(app_task_main,
                APP_TASK_NAME,
                APP_TASK_STACKSIZE / ((uint32_t)sizeof(StackType_t)),
                NULL,
                APP_TASK_PRIO,
                NULL);
}
static void app_init_device_role(void)
{
    app_device_role_t role = APP_DEVICE_MASTER;
    uint32_t size = 1;
#ifdef __CFW_CONFIG_MODE__
    role = (bt_sink_app_device_role_t)(CFW_CFG_ITEM_VALUE(bt_device_role));
#else
    nvdm_status_t nvdm_status = nvdm_read_data_item("BT_SINK", "role", (uint8_t *)&role, &size);
    app_report("read role result:%d", nvdm_status);
#endif
    app_context.device_role  = role;
    app_report("init role:%d", app_context.device_role);
    return;
}
app_device_role_t app_get_device_role(void)
{
    //app_report("[Sink][APP] get device role:%d", bt_sink_app_context.device_role);   
    return app_context.device_role;
}
void app_set_device_role(app_device_role_t role)
{
    app_report("[Sink][APP] set device role:%d", role);   
    #ifndef __CFW_CONFIG_MODE__
    nvdm_status_t result =
        nvdm_write_data_item("BT_SINK",
                             "role",
                             NVDM_DATA_ITEM_TYPE_RAW_DATA,
                             (const uint8_t *)&role,
                             1);
    app_context.device_role = role;
    app_report("write role result:%d", result);
    #endif
    return;
}
void app_battery_report_post_callback(srv_event_t event_id, srv_status_t result, void *parameters)
{
    if (NULL != parameters) {
        vPortFree(parameters);
    }
}
void app_battery_report_handler(int32_t charger_exist, uint8_t capacity)
{
    if (app_context.queue_handle != NULL) {
        app_battery_info_t *battery_info_p = (app_battery_info_t *)pvPortMalloc(sizeof(*battery_info_p));
        battery_info_p->charger_exist = charger_exist;
        battery_info_p->capacity = capacity;
        app_event_post((srv_event_t)BT_SINK_EVENT_APP_BATTERY_NOTIFICATION,
                                       (void *)battery_info_p,
                                       app_battery_report_post_callback);
    }
    return;
}

