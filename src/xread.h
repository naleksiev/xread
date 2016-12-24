/*
 * Copyright 2016 Nikolay Aleksiev. All rights reserved.  * License: https://github.com/naleksiev/xread/blob/master/LICENSE
 */

#ifndef XREAD_H
#define XREAD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xr_type {
    xr_type_attribute,
    xr_type_element_start,
    xr_type_element_end,
    xr_type_error,
} xr_type;

typedef struct xr_str {
    const char* cstr;
    int32_t     len;
} xr_str;

typedef void (*xr_callback)(xr_type type, const xr_str* name, const xr_str* value, void* user_data);

void xr_read(xr_callback cb, const char* doc, void* user_data);

#ifdef __cplusplus
}
#endif

#endif //#ifndef XREAD_H
