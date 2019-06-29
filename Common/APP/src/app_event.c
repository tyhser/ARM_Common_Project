#include "FreeRTOS.h"
#include "task.h"
#include "bt_sink_app_main.h"
//
static void app_event_node_init(app_event_node_t *event_node)
{
    event_node->previous = event_node;
    event_node->next = event_node;
}
static void app_event_node_insert(app_event_node_t *head, app_event_node_t *node)
{
    node->next = head;
    node->previous = head->previous;
    head->previous->next = node;
    head->previous = node;
}
static void app_event_node_remove(app_event_node_t *node)
{
    node->previous->next = node->next;
    node->next->previous = node->previous;
}
static app_event_callback_node_t *app_event_node_find_callback(app_event_node_t *head,
        srv_event_t event_id,
        app_event_callback_t callback)
{
    app_event_node_t *current_node = head->next;
    app_event_callback_node_t *result = NULL;
    while (current_node != head) {
        if (((app_event_callback_node_t *)current_node)->event_id == event_id
                && ((app_event_callback_node_t *)current_node)->callback == callback) {
            result = (app_event_callback_node_t *)current_node;
            break;
        }
        current_node = current_node->next;
    }
    return result;
}
void app_event_init(void)
{
    app_context.invoking =  SRV_EVENT_ALL;
    app_event_node_init(&   app_context.dynamic_callback_header);
}
void app_event_post(srv_event_t event_id, void *parameters, app_event_post_result_t callback)
{
    app_event_t event;
    app_report("[Sink] bt_sink_app_event_post, event:%x", event_id);
    memset(&event, 0, sizeof(bt_sink_app_event_t));
    event.event_id = event_id;
    event.parameters = parameters;
    event.post_callback = callback;
    if (app_context.queue_handle == NULL) {
        app_report("[Sink] queue is not ready.");
        return;
    }
    if (pdPASS != xQueueSend(app_context.queue_handle, &event, 0)) {
        if (NULL != callback) {
            callback(event_id, SRV_STATUS_FAIL, parameters);
        }
        app_report("[Sink][Fatal Error] event lost:0x%x", event_id);
    }
}
void app_event_register_callback(srv_event_t event_id, app_event_callback_t callback)
{
    app_event_callback_node_t *callback_node =
        app_event_node_find_callback(&app_context.dynamic_callback_header, event_id, callback);
    if (NULL == callback_node) {
        callback_node = (app_event_callback_node_t *)pvPortMalloc(sizeof(*callback_node));
        if (NULL != callback_node) {
            memset(callback_node, 0, sizeof(app_event_callback_node_t));
            callback_node->event_id = event_id;
            callback_node->callback = callback;
            app_event_node_insert(&app_context.dynamic_callback_header, &callback_node->pointer);
        }
    } else {
        callback_node->dirty = false;
    }
}
void app_event_deregister_callback(srv_event_t event_id, app_event_callback_t callback)
{
    app_event_callback_node_t *callback_node =
        app_event_node_find_callback(&app_context.dynamic_callback_header, event_id, callback);
    if (NULL != callback_node) {
        if ((event_id == SRV_EVENT_ALL && SRV_EVENT_ALL != app_context.invoking)
                || event_id == app_context.invoking) {
            callback_node->dirty = true;
        } else {
            app_event_node_remove(&callback_node->pointer);
            vPortFree((void *)callback_node);
        }
    }
}
static srv_status_t app_event_invoke(srv_event_t event, void *parameters)
{
    srv_status_t result = SRV_STATUS_SUCCESS;
    app_event_node_t *dynamic_callback = &app_context.dynamic_callback_header;
    app_context.invoking = event;
    while ((dynamic_callback = dynamic_callback->next) != &app_context.dynamic_callback_header) {
        if (SRV_EVENT_ALL == ((app_event_callback_node_t *)dynamic_callback)->event_id) {
            result = ((app_event_callback_node_t *)dynamic_callback)->callback(event, parameters);
            if (SRV_STATUS_EVENT_STOP == result) {
                // TRACE
                break;
            }
        } else if (event == ((app_event_callback_node_t *)dynamic_callback)->event_id) {
            result = ((app_event_callback_node_t *)dynamic_callback)->callback(event, parameters);
            if (SRV_STATUS_EVENT_STOP == result) {
                // TRACE
                break;
            }
        } else {
            // TRACE
        }
    }
    app_context.invoking = SRV_EVENT_ALL;
    dynamic_callback = app_context.dynamic_callback_header.next;
    while (dynamic_callback != &app_context.dynamic_callback_header) {
        if (((app_event_callback_node_t *)dynamic_callback)->dirty) {
            app_event_node_t *dirty_node = dynamic_callback;
            dynamic_callback = dynamic_callback->next;
            app_event_node_remove(dirty_node);
            vPortFree((void *)dirty_node);
            continue;
        }
        dynamic_callback = dynamic_callback->next;
    }
    return result;
}
void app_event_process(app_event_t *event)
{
    srv_status_t result;
    if (NULL != event) {
        app_report("[Sink] app_event_process:0x%x" , event->event_id);
        result = app_event_invoke(event->event_id, event->parameters);
        if (event->post_callback) {
            event->post_callback(event->event_id, result, event->parameters);
        }
    }
}
void app_event_post_callback(srv_event_t event_id, srv_status_t result, void *parameters)
{
    app_report("[Sink] free event:0x%x params:0x%x", event_id, parameters);
    if (NULL != parameters) {
        vPortFree(parameters);
        parameters = NULL;
    }
}
srv_status_t app_event_handler(srv_event_t event_id, void *parameters)
{
    srv_event_param_t *event = (srv_event_param_t *)parameters;
    app_report("[Sink] event:0x%x", event_id);
    switch (event_id) {
        case SRV_EVENT_STATE_CHANGE:
            app_report("[Sink] state change, previous:0x%x, now:0x%x", event->state_change.previous, event->state_change.now);
            app_context.state = event->state_change.now;
        #ifdef APP_NO_ACTION_AUTO_POWER_OFF
            app_auto_power_off_by_state(event->state_change.now);
        #endif
            //app_atci_indicate(BT_SINK_APP_IND_TYPE_STATE, (uint32_t)event->state_change.now);
            break;
        case SRV_EVENT_CONNECTION_INFO_UPDATE:
            break;
        case BT_SINK_EVENT_APP_EXT_COMMAND: {
            app_ext_cmd_t *ext_cmd_p = (app_ext_cmd_t *)parameters;
            //app_key_action_handler(ext_cmd_p->key_value, ext_cmd_p->key_action);
        }
        break;
        case BT_SINK_EVENT_APP_BATTERY_NOTIFICATION: {
            app_battery_info_t *battery_info_p = (app_battery_info_t *)parameters;
            //app_report("[Sink] battery level, charger_exist:%d, capacity:%d", battery_info_p->charger_exist, battery_info_p->capacity);
            //app_battery_level_handler(battery_info_p->charger_exist, battery_info_p->capacity);
            //extern void ble_bas_app_update_battery_capacity(int32_t capacity);
            //app_update_battery_capacity((int32_t)(battery_info_p->capacity));
        }
        break;
        case BT_SINK_EVENT_APP_SYS_LOG_ON:
            break;
        case BT_SINK_EVENT_APP_SYS_LOG_OFF:
            break;
        default:
            break;
    }
#ifdef MTK_PROMPT_SOUND_ENABLE
    app_voice_prompt_by_sink_event(event_id, parameters);
#endif /*MTK_PROMPT_SOUND_ENABLE*/
    return SRV_STATUS_SUCCESS;
}
