#ifndef __XREAD_H__
#define __XREAD_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum xr_type {
    xr_type_attribute,
    xr_type_cdata,
    xr_type_comment,
    xr_type_doctype,
    xr_type_element,
    xr_type_element_end,
    xr_type_error,
    xr_type_proc_instr,
    xr_type_text,
    xr_type_xml_decl,
} xr_type;

typedef struct xr_str {
    const char* cstr;
    int32_t     len;
} xr_str;

typedef void (*xr_callback)(xr_type type, xr_str name, xr_str value);

void xr_read(xr_callback cb, const char* doc);

#ifdef __cplusplus
}
#endif

#endif //#ifndef __XREAD_H__
