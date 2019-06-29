#ifndef APP_MAIN_H
#define APP_MAIN_H
#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "srv.h"
#include "app_event.h"

#define APP_TASK_PRIORITY   (1)
#define APP_TASK_STACK_SIZE (1024)
#define APP_QUEUE_SIZE      (30)

/**
    *  @brief Define for the device role.
*/
#define APP_DEVICE_MASTER  (0x00)
#define APP_DEVICE_SLAVE       (0x01)
typedef uint8_t app_device_role_t;

typedef struct {
    QueueHandle_t       queue_handle;
    srv_state_t         state;
    app_event_node_t    dynamic_callback_header;
    srv_event_t         invoking;
    app_device_role_t   device_role;
    uint8_t             battery_level;
    srv_features_config_t feature_config;
    bool notify_connection_state;
} app_context_t;

extern app_context_t app_context;

void app_task_create(void);
void app_task_main(void *arg);
app_device_role_t bt_sink_app_get_device_role(void);
void bt_sink_app_set_device_role(app_device_role_t role);
void bt_sink_app_key_action_handler(srv_key_value_t key_value, srv_key_action_t key_action);
void bt_sink_app_reset_hf_battery(void);
void bt_sink_app_battery_level_handler(int32_t charger_exist, uint8_t capacity);
#endif
