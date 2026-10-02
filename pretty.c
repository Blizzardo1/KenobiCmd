#include "pretty.h"
#include <stdio.h>

int pretty_print_info(const char * header, const wchar_t *value, uint8_t header_color, uint8_t value_color, bool bold_header) {
    if(bold_header)
        return printf("\x1b[1;%dm%s: \x1b[1;%dm%ls\x1b[0m", header_color, header, value_color, value);
    return printf("\x1b[%dm%s: \x1b[2;%dm%ls\x1b[0m", header_color, header, value_color, value);
}

int pretty_println_info(const char * header, const wchar_t *value, uint8_t header_color, uint8_t value_color, bool bold_header) {
    int res = pretty_print_info(header, value, header_color, value_color, bold_header);
    res += printf("\n");
    return res;
}