#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../src/xread.h"

void handler(xr_type type, const xr_str* name, const xr_str* val) {
    switch (type) {
        case xr_type_element_start:
        case xr_type_element_end:
        case xr_type_attribute:
            return;
        case xr_type_error:
            exit(1);
            return;
    }
}

int main() {
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
    xr_read(&handler, buffer);
    printf("%.4f\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    free(buffer);
}

