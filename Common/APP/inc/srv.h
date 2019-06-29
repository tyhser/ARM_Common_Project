#ifndef __SRV_H_
#define __SRV_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/**
 @section srv_api_usage How to use this module
 *  - Step1: Mandatory, implement #srv_get_mapping_table() to get the mapping relation of user input and sink service status
 *   - Sample code:
 *    @code
 *       static const srv_table_t g_bt_sink_app_mapping_table[] = {
 *            // Call control
 *           {
 *                // ANSWER INCOMING
 *                SRV_KEY_FUNC,
 *                SRV_KEY_ACT_PRESS_UP,
 *                SRV_STATE_INCOMING,
 *                SRV_ACTION_ANSWER
 *            },
 *            {
 *                // REJECT INCOMING
 *                SRV_KEY_FUNC,
 *                SRV_KEY_ACT_LONG_PRESS_UP,
 *                SRV_STATE_INCOMING,
 *                SRV_ACTION_REJECT
 *            }
 *       };
 *
 *       const srv_table_t *srv_get_mapping_table(void)
 *       {
 *            return g_bt_sink_app_mapping_table;
 *       }
 *    @endcode
 *
 *  - Step2: Mandatory, implement #srv_event_callback() to handle the sink events, such as status changed, connection information, caller information etc.
 *   - Sample code:
 *    @code
 *       void srv_event_callback(srv_event_t event_id, srv_event_param_t *param)
 *       {
 *           switch (event_id)
 *           {
 *              case SRV_EVENT_STATE_CHANGE:
 *                  printf("State changed, previous:0x%x, now:0x%x", param->state_change.previous, param->state_change.now);
 *                  break;
 *
 *              case SRV_EVENT_CONNECTION_INFO_UPDATE:
 *                  printf("Connection information, profile:0x%x", event->connection_info.profile_type);
 *                  break;
 *
 *              case SRV_EVENT_HF_CALLER_INFORMATION:
 *                  bt_sink_app_report("Caller information, number:%s, name:%s",
 *                              event->caller_info.number,
 *                              event->caller_info.name);
 *                  break;
 *
 *              case SRV_EVENT_HF_MISSED_CALL:
 *                  bt_sink_app_report("Missed call, number:%s, name:%s",
 *                              event->caller_info.number,
 *                              event->caller_info.name);
 *                  break;
 *
 *              case SRV_EVENT_HF_SPEAKER_VOLUME_CHANGE:
 *                  bt_sink_app_report("Speaker volume, %d", event->volume_change.current_volume);
 *                  break;
 *
 *              default:
 *                  break;
 *            }
 *        }
 *    @endcode
 *
 *  - Step3: Mandatory, receive Bluetooth profile event from stack callback or Bluetooth callback manager.
 *   - Sample code:
 *    @code
 *       const bt_gap_config_t* bt_gap_get_local_configuration(void)
 *       {
 *           return srv_gap_config();
 *       }
 *
 *       void bt_gap_get_link_key(bt_gap_link_key_notification_ind_t* key_information)
 *       {
 *           srv_cm_reply_link_key(key_information);
 *       }
 *
 *       bt_status_t bt_hfp_get_init_params(bt_hfp_init_params_t *init_params)
 *       {
 *           return srv_hf_get_init_params(init_params);
 *       }
 *
 *       bt_status_t bt_a2dp_get_init_params(bt_a2dp_init_params_t *param)
 *       {
 *           return srv_a2dp_get_init_params(param);
 *       }
 *
 *       bt_status_t bt_app_event_callback(bt_msg_type_t msg, bt_status_t status, void *buff)
 *       {
 *           return srv_common_callback(msg, status, buff);
 *       }
 *    @endcode
 *
 *  - Step4: Mandatory, initialize Bluetooth sink service when system initialization.
 *   - Sample code:
 *    @code
 *           srv_features_config_t config;
 *
 *           config.features = SRV_FEATURE_POWER_ON_RECONNECT;
 *           srv_init(&config);
 *    @endcode
 *
 *  - Step5: Optional, according to mapping table and current sink service status, send user event to control remote device.
 *   - Sample code:
 *    @code
 *           // if current sink status is SRV_STATE_INCOMING
 *           // and mapping table which get from srv_get_mapping_table() include following item.
 *           // {
 *           //     SRV_KEY_FUNC,
 *           //     SRV_KEY_ACT_PRESS_UP,
 *           //     SRV_STATE_INCOMING,
 *           //     SRV_ACTION_ANSWER
 *           // }
 *           srv_key_action(SRV_KEY_FUNC, SRV_KEY_ACT_PRESS_UP);
 *    @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NULL
#define NULL ((void *)0)       /**<Default value of the pointer.*/
#endif

/**
 *  @brief This structure define service actions.
 */
typedef uint32_t srv_action_t;
#define SRV_ACTION_BASE 0x40000000                                                                           /**< Sink service actions beginning. */

#define SRV_ACTION_CM_RANGE      0x100                                                                       /**< Connection manager action range. */
#define SRV_ACTION_CM_START      (SRV_ACTION_BASE + 1)                                       /**< Connection manager action beginning. */
#define SRV_ACTION_CM_END        (SRV_ACTION_CM_START + SRV_ACTION_CM_RANGE - 1)             /**< Connection manager action end. */

#define SRV_ACTION_USER_RANGE   0x100                                                                        /**< User defined action range. */
#define SRV_ACTION_USER_START   (SRV_ACTION_CM_END + 1)                                           /**< User defined action beginning. */
#define SRV_ACTION_USER_END     (SRV_ACTION_USER_START + SRV_ACTION_USER_RANGE - 1)          /**< User defined action end. */

/* Internal use start */
#define SRV_ACTION_COMMUNICATION_INIT       (SRV_ACTION_CM_START)     /**< This action indicates the profile to initialize. */
#define SRV_ACTION_COMMUNICATION_CONNECT    (SRV_ACTION_CM_START + 1) /**< This action indicates the profile to create connection. */
#define SRV_ACTION_COMMUNICATION__DISCONNECT (SRV_ACTION_CM_START + 2) /**< This action indicates the profile to disconnect remote device. */
#define SRV_ACTION_COMMUNICATION__DEINIT     (SRV_ACTION_CM_START + 3) /**< This action indicates the profile to de-initialize. */
#define SRV_ACTION_RESET_COMMUNICATION_LIST (SRV_ACTION_CM_START + 4) /**< This action indicates the profile to reset trusted device list. */
/* Internal use end */

#define SRV_ACTION_ALL (SRV_ACTION_USER_END + 1)            /**< This action means the end of action and receiver want to receive any actions. */

/**
 *  @brief This structure define service events.
 */
typedef uint32_t srv_event_t;
#define SRV_EVENT_BASE          0x50000000          

#define SRV_EVENT_COMMON_RANGE  0x100                                                                 /**< Common event range. */
#define SRV_EVENT_COMMON_START  SRV_EVENT_BASE                                                /**< Common event beginning. */
#define SRV_EVENT_COMMON_END    (SRV_EVENT_COMMON_START + SRV_EVENT_COMMON_RANGE - 1) /**< Common event end. */

#define SRV_EVENT_CM_RANGE      0x100                                                                 /**< Connection manager event range. */
#define SRV_EVENT_CM_START      (SRV_EVENT_COMMON_END + 1)                                    /**< Connection manager event beginning. */
#define SRV_EVENT_CM_END        (SRV_EVENT_CM_START + SRV_EVENT_CM_RANGE - 1)         /**< Connection manager event end. */

#define SRV_EVENT_USER_RANGE   0x100                                                                  /**< User defined event range. */
#define SRV_EVENT_USER_START   (SRV_EVENT_CM_END + 1)                                      /**< User defined event beginning. */
#define SRV_EVENT_USER_END     (SRV_EVENT_USER_START + SRV_EVENT_USER_RANGE - 1)      /**< User defined event end. */


#define SRV_EVENT_STATE_CHANGE                 (SRV_EVENT_COMMON_START)  /**< This event indicates the status of sink service has changed. */
#define SRV_EVENT_CONNECTION_INFO_UPDATE       (SRV_EVENT_CM_START)      /**< This event indicates the connection information of current link. */
#define SRV_EVENT_USER                         (SRV_EVENT_USER_START)    /**< This event means the start of the customized event. */

/* Add new event before this line */
#define SRV_EVENT_ALL (SRV_EVENT_USER_END + 1)                       /**< This event means the end of the events and receiver want to receive all the events. */


/**
 *  @brief This enum is the key value.
 */
typedef enum {
    SRV_KEY_NONE,       /**< Invalid key. */
    SRV_KEY_FUNC,       /**< Function key. */
    SRV_KEY_NEXT,       /**< Next key. */
    SRV_KEY_PREV,       /**< Previous key. */
    SRV_KEY_VOL_DOWN,   /**< Volume down key. */
    SRV_KEY_VOL_UP      /**< Volume up key. */
} srv_key_value_t;

/**
 *  @brief This enum is the key action.
 */
typedef enum {
    SRV_KEY_ACT_NONE,                 /**< Key action: invalid key action. */
    SRV_KEY_ACT_PRESS_DOWN,           /**< Key action: press down. */
    SRV_KEY_ACT_PRESS_UP,             /**< Key action: press up. */
    SRV_KEY_ACT_LONG_PRESS_DOWN,      /**< Key action: long press down. */
    SRV_KEY_ACT_LONG_PRESS_UP,        /**< Key action: long press up. */
    SRV_KEY_ACT_LONG_LONG_PRESS_DOWN, /**< Key action: long long press down. */
    SRV_KEY_ACT_LONG_LONG_PRESS_UP,   /**< Key action: long long press up. */
    SRV_KEY_ACT_VERY_LONG_PRESS_DOWN, /**< Key action: very long press up. */
    SRV_KEY_ACT_VERY_LONG_PRESS_UP,   /**< Key action: very long press up. */
    SRV_KEY_ACT_DOUBLE_CLICK,         /**< Key action: double click. */
    SRV_KEY_ACT_TRIPLE_CLICK          /**< Key action: triple click. */
} srv_key_action_t;

/**
 *  @brief This enum is the sink service status.
 */
typedef enum {
    SRV_STATUS_SUCCESS        =     0,    /**< The sink service status: success. */
    SRV_STATUS_FAIL           =    -1,    /**< The sink service status: fail. */
    SRV_STATUS_PENDING        =    -2,    /**< The sink service status: operation is pending. */
    SRV_STATUS_INVALID_PARAM  =    -3,    /**< The sink service status: invalid parameters. */
    SRV_STATUS_DB_NOT_FOUND   =    -4,    /**< The sink service status: database is not found. */
    SRV_STATUS_EVENT_STOP     =    -5,    /**< The sink service status: event stop looping. */
    SRV_STATUS_NO_REQUEST     =    -6,    /**< The sink service status: no request is found. */
    SRV_STATUS_LINK_EXIST     =    -7,    /**< The sink service status: link is already existed. */
    SRV_STATUS_MAX_LINK       =    -8,    /**< The sink service status: reach the max link number. */
    SRV_STATUS_NEED_RETRY     =    -9,    /**< The sink service status: the request need to be retried. */
    SRV_STATUS_REQUEST_EXIST  =    -10,   /**< The sink service status: the request is already existed. */
    SRV_STATUS_INVALID_STATUS =    -11,   /**< The sink service status: invalid status. */
    SRV_STATUS_USER_CANCEL    =    -12    /**< The sink service status: user cancel the action. */
} srv_status_t;

typedef enum {
    SRV_STATE_NONE            = 0x0000,  /**<powered off. */
    SRV_STATE_POWER_ON        = 0x0001,  /**<powered on. */
} srv_state_t;

/**
 *  @brief This enum is the feature configuration of sink service.
 */
typedef enum {
    SRV_FEATURE_NONE               = 0x0000,  /**< None. */
    SRV_FEATURE_POWER_ON_RECONNECT = 0x0001,  /**< Device will reconnect the trusted devices when Bluetooth switch on. */
    SRV_FEATURE_DISCOVERABLE_ONCE  = 0x0002,  /**< Device will be discoverable once when Bluetooth switch on. */
    SRV_FEATURE_ENABLE_APL_CMD     = 0x0004,  /**< Device will enable Apple custom AT command when SLC is connected. */
    SRV_FEATURE_POWER_ON_INQUIRY   = 0x0008,   /**< Device will inquiry the devices when Bluetooth switch on. */
    SRV_FEATURE_DISABLE_LINK_LOST_RECONNECT   = 0x0010   /**< Device will not reconnect remote device when Link lost happened. */
} srv_feature_t;

/**
 *  @brief This structure is service table definition which get from #srv_get_mapping_table.
 */
typedef struct {
    srv_key_value_t  key_value;    /**<  Key value, which key is pressed. */
    srv_key_action_t key_action;   /**<  Key action, the state of the key. */
    srv_state_t      sink_state;   /**<  Sink state, on which state of sink service to handle the action. */
    srv_action_t     sink_action;  /**<  Sink action, which action of sink service will be executed. */
} srv_table_t;

/**
 *  @brief This structure is the callback parameters type of event(#SRV_EVENT_STATE_CHANGE) which indicates sink service state is changed.
 */
typedef struct {
    srv_state_t previous;            /**<  Previous sink service state. */
    srv_state_t now;                 /**<  Current sink service state. */
} srv_state_change_t;


/**
 *  @brief This structure is the callback parameters of event, which indicated aws remote triggered key action
 */
typedef struct {
    srv_key_value_t value;
    srv_key_action_t action;
} srv_key_information_t;

/**
 *  @brief This structure is the callback parameters of #srv_event_callback, it is the union of all the events.
 */
typedef union {
    srv_state_change_t             state_change;            /**<  State change. */
    srv_key_information_t          key_info;                /**<  Key Press information*/
} srv_event_param_t;

/**
 *  @brief This structure is the parameters of #srv_init which configure the features of sink service.
 */
typedef struct {
    srv_feature_t features;    /**<  Sink service features. */
} srv_features_config_t;

/**
 * @brief                              This function initialze the sink service.
 * @param[in] features          is the feature configuration of sink service.
 * @return                            None.
 */
void srv_init(srv_features_config_t *features);

/**
 * @brief                          This function send a key value and key action,
 *                                    sink service will control remote device according the mapping table which get from #srv_get_mapping_table.
 * @param[in] key_value    is current input key value.
 * @param[in] key_action   is current input key action.
 * @return                         #SRV_STATUS_SUCCESS, send the action successfully.
 *                                     #SRV_STATUS_FAIL, fail to send the action, there is no mapping action in current status.
 */
srv_status_t srv_key_action(srv_key_value_t key_value,
        srv_key_action_t key_action);


/**
 * @brief                          This function get the mapping table of key and action.
 * @return                        The mapping table supplied by user.
 */
const srv_table_t *srv_get_mapping_table(void);

/**
 * @brief                         This function is a static callback for the application to listen to the event. Provide a user-defined callback.
 * @param[in] event_id     is the callback event ID.
 * @param[in] param        is the parameter of the callback message.
 * @return                       None.
 */
void srv_event_callback(srv_event_t event_id, srv_event_param_t *param);

/**
 * @brief                             Receive Bluetooth protocol stack events.
 * @param[in] msg               is the callback message type.
 * @param[in] status            is the status of the callback message.
 * @param[in] buffer             is the payload of the callback message.
 * @return                           The status of this operation returned from the callback.
 */
bt_status_t srv_common_callback(bt_msg_type_t msg, bt_status_t status, void *buffer);

#ifdef __cplusplus
}
#endif
#endif
