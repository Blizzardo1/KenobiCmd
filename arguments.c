#include <argp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arguments.h"
#include "main.h"

const char *argp_program_version = "kenobicmd 1.0";
const char *argp_program_bug_address = "<bugz@blizzeta.net>";

static char doc[] = "Kenobi Command -- a program that will communicate with QMK Kenobi Firmware";

static char args_doc[] = "";

static struct argp_option options[] = {
    {"get-os",          'o', 0,               0, "Get operating system information"},
    {"battery",         'b', 0,               0, "Get battery information (useful with -d)"},
    {"bluetooth",       't', 0,               0, "Enable bluetooth"},
    {"bootloader",      'U', 0,               0, "Enter bootloader mode"},
    {"clear-led",       'C', "IDX",           OPTION_ARG_OPTIONAL, "Clear LED overlay (all if IDX omitted)"},
    {"daemon",          'd', 0,               0, "Run this as a daemon"},
    {"factory-test",    'f', 0,               0, "Run factory test"},
    {"get-conn-mode",   'c', 0,               0, "Get connection mode information"},
    {"get-version",     'v', 0,               0, "Get version information"},
    {"interval",        'i', "MS",            0, "Refresh interval in milliseconds. Use -d for daemon mode"},
    {"interactive",     'I', 0,               0, "Run in interactive mode"},
    {"keyboard-layout", 'k', 0,               0, "Get keyboard layout (useful with -d)"},
    {"lock-status",     'l', 0,               0, "Get keyboard lock status"},
    {"lock",            'O', 0,               0, "Lock the keyboard"},
    {"no-banner",       'n', 0,               0, "Don't print the banner"},
    {"ping",            'p', 0,               0, "Ping the device"},
    {"set-brightness",  'B', "0-255",         0, "Set keyboard brightness (useful with -d)"},
    {"set-led",         'L', "IDX,RRGGBB[,MS]", 0, "Set a single LED (MS=0: firmware default timeout)"},
    {"set-rgb-color",   'G', "H,S,V",         0, "Set keyboard HSV color (each 0-255)"},
    {"set-rgb-mode",    'R', "MODE",          0, "Set keyboard RGB effect number"},
    {"strip",           's', 0,               0, "Strip text, leave only numbers"},
    {"script",          'S', "PATH",          0, "Run the script from the specified file"},
    {"wpm",             'w', 0,               0, "Get keyboard words per minute (useful with -d)"},
    {"watch", 'W', "EVENTS", OPTION_ARG_OPTIONAL, "Stream device events: lock,layer,os,battery (default all)"},
    {0}
};

bool parse_int(const char *s, long lo, long hi, long *out) {
    char *end;
    errno = 0;
    long v = strtol(s, &end, 0);
    if (errno || end == s || *end || v < lo || v > hi) return false;
    *out = v;
    return true;
}

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    (void)arg;
    arguments *args = state->input;
    long v = 0;

    switch(key) {
        case 'b': args->flags.battery = 1;                     break;
        case 'd': args->flags.daemon = 1;                      break;
        case 'f': args->flags.factory_test = 1;                break;
        case 'i': args->interval_ms = atoi(arg);         break;
        case 'I': args->flags.interactive = 1;                 break;
        case 'k': args->flags.keyboard_layout = 1;             break;
        case 'l': args->flags.lock_status = 1;                 break;
        case 'n': args->flags.banner = 0;                      break;
        case 's': args->flags.strip = 1;                       break;
        case 'S': args->script_path = arg;                     break;
        case 't': args->flags.bluetooth = 1;                   break;
        case 'w': args->flags.wpm = 1;                         break;
        case 'W':
            args->values.watch_mask = parse_ev_mask(arg);
            if (!args->values.watch_mask) argp_error(state, "EVENTS must be a list of: lock,layer,os,battery");
            args->flags.watch = 1;
        break;

        case 'B':
            if (!parse_int(arg, 0, 255, &v)) argp_error(state, "brightness must be 0-255");
            args->values.brightness = (uint8_t)v;
            args->flags.set_brightness = 1;
        break;

        case 'R':
            if (!parse_int(arg, 0, 255, &v)) argp_error(state, "mode must be 0-255");
            args->values.rgb_mode = (uint8_t)v;
            args->flags.set_rgb_mode = 1;
            break;

        case 'C':
            args->values.clear_idx = 0xFF;                       // default: clear all
            if (arg) {
                if (!parse_int(arg, 0, 254, &v)) argp_error(state, "LED index must be 0-254");
                args->values.clear_idx = (uint8_t)v;
            }
            args->flags.clear_led = 1;
            break;

        case 'G': {
            int h;
            int s;
            int val;
            if (sscanf(arg, "%d,%d,%d", &h, &s, &val) != 3 ||
                h < 0 || h > 255 || s < 0 || s > 255 || val < 0 || val > 255)
                argp_error(state, "expected H,S,V with each value 0-255");
            args->values.hsv[0] = (uint8_t)h;
            args->values.hsv[1] = (uint8_t)s;
            args->values.hsv[2] = (uint8_t)val;
            args->flags.set_rgb_color = 1;
            break;
        }

        case 'L': {
            int idx = 0;
            int ms = 0;
            unsigned rgb = 0;
            int n = sscanf(arg, "%d,%x,%d", &idx, &rgb, &ms);
            if (n < 2 || idx < 0 || idx > 254 || rgb > 0xFFFFFF || ms < 0 || ms > 65535)
                argp_error(state, "expected IDX,RRGGBB[,MS]");
            args->values.led_idx    = (uint8_t)idx;
            args->values.led_rgb[0] = (rgb >> 16) & 0xFF;
            args->values.led_rgb[1] = (rgb >> 8)  & 0xFF;
            args->values.led_rgb[2] =  rgb        & 0xFF;
            args->values.led_ms     = (n == 3) ? (uint16_t)ms : 0;
            args->flags.set_led = 1;
            break;
        }
        case 'O': args->flags.lock = 1;                        break;
        case 'U': args->flags.bootloader = 1;                  break;
        case 'v': args->flags.get_version = 1;                 break;
        case 'c': args->flags.get_conn_mode = 1;               break;
        case 'p': args->flags.ping = 1;                        break;
        case 'o': args->flags.get_os = 1;                      break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


int arg_usage(void) {
    return 0;
}

static struct argp argp = {options, parse_opt, args_doc, doc};

arguments parse(int argc, char **argv) {
    arguments args;
    args.flags.battery = 0;
    args.flags.keyboard_layout = 0;
    args.flags.lock_status = 0;
    args.flags.wpm = 0;
    args.flags.bluetooth = 0;
    args.flags.factory_test = 0;
    args.flags.banner = 1;
    args.flags.strip = 0;
    args.flags.daemon = 0;
    args.interval_ms = 1000;
    args.flags.lock = 0;
    args.flags.bootloader = 0;
    args.flags.clear_led = 0;
    args.flags.set_brightness = 0;
    args.flags.set_led = 0;
    args.flags.set_rgb_color = 0;
    args.flags.set_rgb_mode = 0;
    args.flags.get_version = 0;
    args.flags.get_conn_mode = 0;
    args.flags.ping = 0;
    args.flags.get_os = 0;
    args.flags.interactive = 0;
    args.flags.get_event = 0;
    args.script_path = NULL;
    args.values.brightness = 0;
    args.values.rgb_mode = 0;
    args.values.hsv[0] = 0;
    args.values.hsv[1] = 0;
    args.values.hsv[2] = 0;
    args.values.clear_idx = 0;
    args.values.led_idx = 0;
    args.values.led_rgb[0] = 0;
    args.values.led_rgb[1] = 0;
    args.values.led_rgb[2] = 0;
    args.values.led_ms = 0;

    error_t arg_error = argp_parse(&argp, argc, argv, 0, NULL, &args);
    if(arg_error == EINVAL) {
        fprintf(stderr, "There was an error parsing arguments: %s", strerror(arg_error));
        return (arguments){0};
    }
    return args;
}
