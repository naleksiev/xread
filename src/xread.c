#include "xread.h"

#include <stdio.h>
#include <stdlib.h>

void xr_read(xr_callback cb, const char* cstr) {
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
        ['<']         = &&l_node,
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

    static void* go_node[] = {
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
        ['A' ... 'A'] = &&l_element_name_begin,
        [91 ... 94]   = &&l_error,
        ['_']         = &&l_element_name_begin,
        [96 ... 96]   = &&l_error,
        ['a' ... 'z'] = &&l_element_name_begin,
        [123 ... 255] = &&l_error,
    };

    static void* go_node_close[] = {
        [0 ... 61]    = &&l_error,
        ['>']         = &&l_node_end,
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
        ['A' ... 'A'] = &&l_element_end_name_begin,
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
        ['>']         = &&l_node_end,
        [63 ... 96]   = &&l_error,
        ['a' ... 'z'] = &&l_attrib_name_begin,
        [123 ... 255] = &&l_error,
    };

    static void *go_attrib_eq[] = {
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

    static void *go_attrib_val_begin[] = {
        [0 ... 8]     = &&l_error,
        ['\t']        = &&l_next,
        ['\n']        = &&l_next,
        [11 ... 12]   = &&l_error,
        ['\r']        = &&l_next,
        [14 ... 31]   = &&l_error,
        [' ']         = &&l_next,
        [33]          = &&l_error,
        ['\"']        = &&l_attrib_val_begin_double,
        [35 ... 38]   = &&l_error,
        ['\'']        = &&l_attrib_val_begin_single,
        [40 ... 255]  = &&l_error,
    };

    static void *go_attrib_val_single[] = {
        [0 ... 31]    = &&l_error,
        [32 ... 33]   = &&l_next,
        ['\"']        = &&l_attrib_val_end,
        [35 ... 255]  = &&l_next,
    };

    static void *go_attrib_val_double[] = {
        [0 ... 31]    = &&l_error,
        [32 ... 33]   = &&l_next,
        ['\"']        = &&l_attrib_val_end,
        [35 ... 255]  = &&l_next,
    };

    xr_str nil  = { .cstr = 0, .len = 0 };
    xr_str node = nil;
    xr_str name = nil;
    xr_str val  = nil;

    const char* c;

    void** go = go_root;
    void*  name_handle = 0;

l_next:
    goto *go[*(c = cstr++)];

l_error:
    name = (xr_str){ .cstr = "Error!", .len = 6 };
    val = (xr_str){ .cstr = c, .len = 1 };
    cb(xr_type_error, name, val);
    return;

l_name:
    goto *name_handle;

l_node:
    go = go_node;
    goto l_next;

l_node_end:
    go = go_root;
    goto l_next;

l_element_name_begin:
    node.cstr = c;
    name_handle = &&l_element_name;
    go = go_name;
    goto l_next;

l_element_name:
    node.len = (int32_t)(c - node.cstr);
    cb(xr_type_element, node, nil);
    go = go_attrib;
    goto *go[*c];

l_element_empty:
    cb(xr_type_element_end, node, nil);
    go = go_node_close;
    goto l_next;

l_element_end:
    go = go_element_end;
    goto l_next;

l_element_end_name_begin:
    node.cstr = c;
    name_handle = &&l_element_end_name;
    go = go_name;
    goto l_next;

l_element_end_name:
    node.len = (int32_t)(c - node.cstr);
    cb(xr_type_element_end, node, nil);
    go = go_node_close;
    goto *go[*c];

l_attrib_name_begin:
    name.cstr = c;
    name_handle = &&l_attrib_name;
    go = go_name;
    goto l_next;

l_attrib_name:
    name.len = (int32_t)(c - name.cstr);
    go = go_attrib_eq;
    goto *go[*c];

l_attrib_eq:
    go = go_attrib_val_begin;
    goto l_next;

l_attrib_val_begin_single:
    val.cstr = cstr;
    go = go_attrib_val_single;
    goto l_next;

l_attrib_val_begin_double:
    val.cstr = cstr;
    go = go_attrib_val_double;
    goto l_next;

l_attrib_val_end:
    val.len = (int32_t)(c - val.cstr);
    cb(xr_type_attribute, name, val);
    go = go_attrib;
    goto l_next;

l_done:
    return;
}

