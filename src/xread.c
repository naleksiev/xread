/*
 * Copyright 2016 Nikolay Aleksiev. All rights reserved.
 * License: https://github.com/naleksiev/xread/blob/master/LICENSE
 */

#include "xread.h"

#include <stdio.h>
#include <stdlib.h>

#define XR_DISPATCH_NEXT()    goto *go[*(c = cstr++)]
#define XR_DISPATCH_THIS()    goto *go[*c];

void xr_read(xr_callback cb, const char* cstr, void* user_data) {
    static void* go_root[] = {
        ['\0']        = &&l_done,
        [1 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33 ... 59]   = &&l_error,
        ['<']         = &&l_tag,
        [61 ... 255]  = &&l_error,
    };

    static void* go_name[] = {
        [0 ... 44]    = &&l_name,
        ['-']         = &&l_next,
        ['.']         = &&l_next,
        [47 ... 47]   = &&l_name,
        ['0' ... '9'] = &&l_next,
        [58 ... 94]   = &&l_name,
        ['_']         = &&l_next,
        [96 ... 96]   = &&l_name,
        ['a' ... 'z'] = &&l_next,
        [123 ... 255] = &&l_name,
    };

    static void* go_tag[] = {
        [0 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33 ... 46]   = &&l_error,
        ['/']         = &&l_element_end,
        [48 ... 64]   = &&l_error,
        ['A' ... 'Z'] = &&l_element_name_begin,
        [91 ... 94]   = &&l_error,
        ['_']         = &&l_element_name_begin,
        [96 ... 96]   = &&l_error,
        ['a' ... 'z'] = &&l_element_name_begin,
        [123 ... 255] = &&l_error,
    };

    static void* go_tag_close[] = {
        [0 ... 61]    = &&l_error,
        ['>']         = &&l_tag_end,
        [63 ... 255]  = &&l_error,
    };

    static void* go_element_end[] = {
        [0 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33 ... 64]   = &&l_error,
        ['A' ... 'Z'] = &&l_element_end_name_begin,
        [91 ... 94]   = &&l_error,
        ['_']         = &&l_element_end_name_begin,
        [96 ... 96]   = &&l_error,
        ['a' ... 'z'] = &&l_element_end_name_begin,
        [123 ... 255] = &&l_error,
    };

    static void* go_attrib[] = {
        [0 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33 ... 46]   = &&l_error,
        ['/']         = &&l_element_empty,
        [48 ... 61]   = &&l_error,
        ['>']         = &&l_tag_end,
        [63 ... 64]   = &&l_error,
        ['A' ... 'Z'] = &&l_attrib_name_begin,
        [91 ... 94]   = &&l_error,
        ['_']         = &&l_attrib_name_begin,
        [96 ... 96]   = &&l_error,
        ['a' ... 'z'] = &&l_attrib_name_begin,
        [123 ... 255] = &&l_error,
    };

    static void* go_attrib_eq[] = {
        [0 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33 ... 60]   = &&l_error,
        ['=']         = &&l_attrib_eq,
        [62 ... 255]  = &&l_error,
    };

    static void* go_attrib_val_begin[] = {
        [0 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33]          = &&l_error,
        ['"']         = &&l_attrib_val_begin_double,
        [35 ... 38]   = &&l_error,
        ['\'']        = &&l_attrib_val_begin_single,
        [40 ... 255]  = &&l_error,
    };

    static void* go_attrib_val_single[] = {
        [0 ... 31]    = &&l_error,
        [32 ... 33]   = &&l_next,
        ['\"']        = &&l_attrib_val_end,
        [35 ... 255]  = &&l_next,
    };

    static void* go_attrib_val_double[] = {
        [0 ... 31]    = &&l_error,
        [32 ... 33]   = &&l_next,
        ['"']         = &&l_attrib_val_end,
        [35 ... 255]  = &&l_next,
    };

    xr_str tag  = { .cstr = 0, .len = 0 };
    xr_str name = { .cstr = 0, .len = 0 };
    xr_str val  = { .cstr = 0, .len = 0 };

    const char* c = NULL;

    void** go = go_root;
    void*  name_handle = 0;

l_next:
    XR_DISPATCH_NEXT();

l_error:
    name = (xr_str){ .cstr = "Error!", .len = 6 };
    val = (xr_str){ .cstr = c, .len = 1 };
    cb(xr_type_error, &name, &val, user_data);
    return;

l_name:
    goto *name_handle;

l_tag:
    go = go_tag;
    XR_DISPATCH_NEXT();

l_tag_end:
    go = go_root;
    XR_DISPATCH_NEXT();

l_element_name_begin:
    tag.cstr = c;
    name_handle = &&l_element_name;
    go = go_name;
    XR_DISPATCH_NEXT();

l_element_name:
    tag.len = (int32_t)(c - tag.cstr);
    cb(xr_type_element_start, &tag, NULL, user_data);
    go = go_attrib;
    XR_DISPATCH_THIS();

l_element_empty:
    cb(xr_type_element_end, &tag, NULL, user_data);
    go = go_tag_close;
    XR_DISPATCH_NEXT();

l_element_end:
    go = go_element_end;
    XR_DISPATCH_NEXT();

l_element_end_name_begin:
    tag.cstr = c;
    name_handle = &&l_element_end_name;
    go = go_name;
    XR_DISPATCH_NEXT();

l_element_end_name:
    tag.len = (int32_t)(c - tag.cstr);
    cb(xr_type_element_end, &tag, NULL, user_data);
    go = go_tag_close;
    XR_DISPATCH_THIS();

l_attrib_name_begin:
    name.cstr = c;
    name_handle = &&l_attrib_name;
    go = go_name;
    XR_DISPATCH_NEXT();

l_attrib_name:
    name.len = (int32_t)(c - name.cstr);
    go = go_attrib_eq;
    XR_DISPATCH_THIS();

l_attrib_eq:
    go = go_attrib_val_begin;
    XR_DISPATCH_NEXT();

l_attrib_val_begin_single:
    val.cstr = cstr;
    go = go_attrib_val_single;
    XR_DISPATCH_NEXT();

l_attrib_val_begin_double:
    val.cstr = cstr;
    go = go_attrib_val_double;
    XR_DISPATCH_NEXT();

l_attrib_val_end:
    val.len = (int32_t)(c - val.cstr);
    cb(xr_type_attribute, &name, &val, user_data);
    go = go_attrib;
    XR_DISPATCH_NEXT();

l_done:
    return;
}

