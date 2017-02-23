/*
 * Copyright 2016-2017 Nikolay Aleksiev. All rights reserved.
 * License: https://github.com/naleksiev/xread/blob/master/LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/xread.h"

void handler(xr_type_t type, const xr_str_t* name, const xr_str_t* val, void* user_data) {
    switch (type) {
        case xr_type_element_start:
            printf("<%.*s>\n", name->len, name->cstr);
            return;
        case xr_type_element_end:
            printf("</%.*s>\n", name->len, name->cstr);
            return;
        case xr_type_attribute:
            printf("%.*s=\"%.*s\"\n", name->len, name->cstr, val->len, val->cstr);
            return;
        case xr_type_error:
            exit(1);
            return;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2)
        return 1;
    FILE* fp = fopen("test/test.xml", "rb");
    if (!fp)
        return 1;
    fseek(fp, 0, SEEK_END);
    int32_t size = (int32_t)ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = (char*)malloc(size + 1);
    if (fread(buffer, 1, size, fp) < size)
        return 1;
    buffer[size] = '\0';
    fclose(fp);
    clock_t start = clock();
    xr_read(&handler, buffer, NULL);
    printf("%.4f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    free(buffer);
}

