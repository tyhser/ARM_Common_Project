#ifndef APP_EVENT_H
#define APP_EVENT_H
#include <stdbool.h>
#include <stdint.h>
#include "srv.h"

#define BT_SINK_EVENT_APP_BASE                (SRV_EVENT_USER + 30 )
#define BT_SINK_EVENT_APP_EXT_COMMAND         (BT_SINK_EVENT_APP_BASE + 1)
#define BT_SINK_EVENT_APP_SYS_LOG_ON          (BT_SINK_EVENT_APP_BASE + 2)
#define BT_SINK_EVENT_APP_SYS_LOG_OFF         (BT_SINK_EVENT_APP_BASE + 3)
#define BT_SINK_EVENT_APP_KEY_INPUT           (BT_SINK_EVENT_APP_BASE + 4)
#define BT_SINK_EVENT_APP_NOTIFICATION        (BT_SINK_EVENT_APP_BASE + 5)
#define BT_SINK_EVENT_APP_LED_NOTIFICATION    (BT_SINK_EVENT_APP_BASE + 6)
#define BT_SINK_EVENT_APP_BATTERY_NOTIFICATION    (BT_SINK_EVENT_APP_BASE + 7)

#define APP_ACTION_PLAY            (SRV_ACTION_USER_START)
#define APP_ACTION_PAUSE           (SRV_ACTION_USER_START + 1)
#define APP_ACTION_NEXT_TRACK      (SRV_ACTION_USER_START + 2)
#define APP_ACTION_PRE_TRACK       (SRV_ACTION_USER_START + 3)

typedef struct {
    srv_key_value_t key_value;
    srv_key_action_t key_action;
} app_ext_cmd_t;

typedef struct {
    int32_t charger_exist;
    uint8_t capacity;
} app_battery_info_t;

typedef srv_status_t (*app_event_callback_t)(srv_event_t event_id, void *parameters);
typedef void (*app_event_post_result_t)(srv_event_t event_id, srv_status_t result, void *parameters);

typedef struct sink_event_node_t {
    struct sink_event_node_t *previous;
    struct sink_event_node_t *next;
} app_event_node_t;

typedef struct {
    srv_event_t event_id;
    app_event_callback_t callback;
} app_event_callback_table_t;

typedef struct {
    srv_event_t event_id;
    void *parameters;
    app_event_post_result_t post_callback;
} app_event_t;

typedef struct {
    app_event_node_t pointer;
    srv_event_t event_id;
    app_event_callback_t callback;
    bool dirty;
} app_event_callback_node_t;

void app_event_init(void);
void app_event_post(srv_event_t event_id, void *parameters, app_event_post_result_t callback);
void app_event_register_callback(srv_event_t event_id, app_event_callback_t callback);
void app_event_deregister_callback(srv_event_t event_id, app_event_callback_t callback);
void app_event_process(app_event_t *event);
srv_status_t app_event_handler(srv_event_t event_id, void *parameters);
void app_event_post_callback(srv_event_t event_id, srv_status_t result, void *parameters);

#endif
