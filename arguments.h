#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char *args[6];
    unsigned int interval_ms;
    char *script_path;

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
        unsigned int interactive : 1;
	unsigned int get_event : 1;
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

/**
 * @brief Parses an integer from a string, ensuring it falls within the specified range.
 * Parses the string `s` as an integer and checks if it falls within the range `lo` to `hi`.
 * If the conversion is successful and the value is within the range, it stores the result in `out` and returns true.
 * Otherwise, it returns false.
 *
 * @param s The string to parse as an integer.
 * @param lo The lower bound of the valid range.
 * @param hi The upper bound of the valid range.
 * @param out A pointer to store the parsed integer if successful.
 * @return true if the parsing was successful and the value is within the range.
 * @return false if the parsing failed or the value is out of range.
 */
bool parse_int(const char *s, long lo, long hi, long *out);
arguments parse(int argc, char **argv);

#endif
