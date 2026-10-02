#ifndef PRETTY_H
#define PRETTY_H

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

#define RED 31
#define GREEN 32
#define YELLOW 33


int pretty_print_info(const char * header, const wchar_t *value, uint8_t header_color, uint8_t value_color, bool bold_header);
int pretty_println_info(const char * header, const wchar_t *value, uint8_t header_color, uint8_t value_color, bool bold_header);

#endif