/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.2 */

#ifndef PB_GOOGLE_PROTOBUF_GOOGLE_PROTOBUF_ANY_PB_H_INCLUDED
#define PB_GOOGLE_PROTOBUF_GOOGLE_PROTOBUF_ANY_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */
typedef PB_BYTES_ARRAY_T(1024) google_protobuf_Any_value_t;
typedef struct _google_protobuf_Any {
    char type_url[64];
    google_protobuf_Any_value_t value;
} google_protobuf_Any;


/* Initializer values for message structs */
#define google_protobuf_Any_init_default         {"", {0, {0}}}
#define google_protobuf_Any_init_zero            {"", {0, {0}}}

/* Field tags (for use in manual encoding/decoding) */
#define google_protobuf_Any_type_url_tag         1
#define google_protobuf_Any_value_tag            2

/* Struct field encoding specification for nanopb */
#define google_protobuf_Any_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, STRING,   type_url,          1) \
X(a, STATIC,   SINGULAR, BYTES,    value,             2)
#define google_protobuf_Any_CALLBACK NULL
#define google_protobuf_Any_DEFAULT NULL

extern const pb_msgdesc_t google_protobuf_Any_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define google_protobuf_Any_fields &google_protobuf_Any_msg

/* Maximum encoded size of messages (where known) */
#define google_protobuf_Any_size                 1092

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
