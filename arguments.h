#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>
#include <stdint.h>

typedef struct {
    char *args[6];
    unsigned int interval_ms;
    struct {
        unsigned int battery : 1;
        unsigned int keyboard_layout : 1;
        unsigned int lock_status : 1;
        unsigned int wpm : 1;
        unsigned int bluetooth : 1;
        unsigned int factory_test : 1;
        unsigned int banner : 1;
        unsigned int strip : 1;
        unsigned int daemon : 1;
        unsigned int lock : 1;
        unsigned int bootloader : 1;
        unsigned int clear_led : 1;
        unsigned int set_brightness : 1;
        unsigned int set_led : 1;
        unsigned int set_rgb_color : 1;
        unsigned int set_rgb_mode : 1;
        unsigned int get_version : 1;
        unsigned int get_conn_mode : 1;
        unsigned int ping : 1;
        unsigned int get_os : 1;
    } flags;
        struct {
        uint8_t brightness;
        uint8_t rgb_mode;
        uint8_t hsv[3];
        uint8_t clear_idx;
        uint8_t led_idx;
        uint8_t led_rgb[3];
        uint16_t led_ms;
    } values;
} arguments;

int arg_usage(void);

arguments parse(int argc, char **argv);

#endif