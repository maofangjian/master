/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.8 */

#ifndef PB_YA_REMOTESTOP_PB_H_INCLUDED
#define PB_YA_REMOTESTOP_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
/* 远程结束充电 server_to_device */
typedef struct _ya_REMOTE_STOP_REQ {
    int32_t slotId; /* 枪头 */
    char orderNo[20]; /* 订单号 */
    int32_t stopReason; /* 停止原因 0用户手动停止 */
} ya_REMOTE_STOP_REQ;

typedef struct _ya_REMOTE_STOP_RES {
    int32_t slotId; /* 枪头 */
    char orderNo[20]; /* 订单号 */
    int32_t code; /* 结果 0：成功 1：失败 */
} ya_REMOTE_STOP_RES;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define ya_REMOTE_STOP_REQ_init_default          {0, "", 0}
#define ya_REMOTE_STOP_RES_init_default          {0, "", 0}
#define ya_REMOTE_STOP_REQ_init_zero             {0, "", 0}
#define ya_REMOTE_STOP_RES_init_zero             {0, "", 0}

/* Field tags (for use in manual encoding/decoding) */
#define ya_REMOTE_STOP_REQ_slotId_tag            1
#define ya_REMOTE_STOP_REQ_orderNo_tag           2
#define ya_REMOTE_STOP_REQ_stopReason_tag        3
#define ya_REMOTE_STOP_RES_slotId_tag            1
#define ya_REMOTE_STOP_RES_orderNo_tag           2
#define ya_REMOTE_STOP_RES_code_tag              3

/* Struct field encoding specification for nanopb */
#define ya_REMOTE_STOP_REQ_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    slotId,            1) \
X(a, STATIC,   SINGULAR, STRING,   orderNo,           2) \
X(a, STATIC,   SINGULAR, INT32,    stopReason,        3)
#define ya_REMOTE_STOP_REQ_CALLBACK NULL
#define ya_REMOTE_STOP_REQ_DEFAULT NULL

#define ya_REMOTE_STOP_RES_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, INT32,    slotId,            1) \
X(a, STATIC,   SINGULAR, STRING,   orderNo,           2) \
X(a, STATIC,   SINGULAR, INT32,    code,              3)
#define ya_REMOTE_STOP_RES_CALLBACK NULL
#define ya_REMOTE_STOP_RES_DEFAULT NULL

extern const pb_msgdesc_t ya_REMOTE_STOP_REQ_msg;
extern const pb_msgdesc_t ya_REMOTE_STOP_RES_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define ya_REMOTE_STOP_REQ_fields &ya_REMOTE_STOP_REQ_msg
#define ya_REMOTE_STOP_RES_fields &ya_REMOTE_STOP_RES_msg

/* Maximum encoded size of messages (where known) */
#define YA_REMOTESTOP_PB_H_MAX_SIZE              ya_REMOTE_STOP_REQ_size
#define ya_REMOTE_STOP_REQ_size                  43
#define ya_REMOTE_STOP_RES_size                  43

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
