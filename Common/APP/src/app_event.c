#include "FreeRTOS.h"
#include "task.h"
#include "bt_sink_app_main.h"
//
static void app_event_node_init(app_event_node_t *event_node)
{
    event_node->previous = event_node;
    event_node->next = event_node;
}

static void app_event_node_inset(app_event_node_t *head, app_event_node_t *node)
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
