/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.2 */

#ifndef PB_MSG_TYPE_MQTTSERVERSWITCH_PB_H_INCLUDED
#define PB_MSG_TYPE_MQTTSERVERSWITCH_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef struct _MQTT_SERVER_SWITCH_ACK_STR {
    int32_t result;
} MQTT_SERVER_SWITCH_ACK_STR;

typedef struct _MQTT_SERVER_SWITCH_STR {
    char ip_url[48];
    int32_t port;
    char mqttUser[32];
    char mqttPassword[32];
} MQTT_SERVER_SWITCH_STR;


/* Initializer values for message structs */
#define MQTT_SERVER_SWITCH_STR_init_default      {"", 0, "", ""}
#define MQTT_SERVER_SWITCH_ACK_STR_init_default  {0}
#define MQTT_SERVER_SWITCH_STR_init_zero         {"", 0, "", ""}
#define MQTT_SERVER_SWITCH_ACK_STR_init_zero     {0}

/* Field tags (for use in manual encoding/decoding) */
#define MQTT_SERVER_SWITCH_ACK_STR_result_tag    1
#define MQTT_SERVER_SWITCH_STR_ip_url_tag        1
#define MQTT_SERVER_SWITCH_STR_port_tag          2
#define MQTT_SERVER_SWITCH_STR_mqttUser_tag      3
#define MQTT_SERVER_SWITCH_STR_mqttPassword_tag  4

/* Struct field encoding specification for nanopb */
#define MQTT_SERVER_SWITCH_STR_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   ip_url,            1) \
X(a, STATIC,   SINGULAR, INT32,    port,              2) \
X(a, STATIC,   SINGULAR, STRING,   mqttUser,          3) \
X(a, STATIC,   SINGULAR, STRING,   mqttPassword,      4)
#define MQTT_SERVER_SWITCH_STR_CALLBACK NULL
#define MQTT_SERVER_SWITCH_STR_DEFAULT NULL

#define MQTT_SERVER_SWITCH_ACK_STR_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    result,            1)
#define MQTT_SERVER_SWITCH_ACK_STR_CALLBACK NULL
#define MQTT_SERVER_SWITCH_ACK_STR_DEFAULT NULL

extern const pb_msgdesc_t MQTT_SERVER_SWITCH_STR_msg;
extern const pb_msgdesc_t MQTT_SERVER_SWITCH_ACK_STR_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define MQTT_SERVER_SWITCH_STR_fields &MQTT_SERVER_SWITCH_STR_msg
#define MQTT_SERVER_SWITCH_ACK_STR_fields &MQTT_SERVER_SWITCH_ACK_STR_msg

/* Maximum encoded size of messages (where known) */
#define MQTT_SERVER_SWITCH_STR_size              126
#define MQTT_SERVER_SWITCH_ACK_STR_size          11

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
