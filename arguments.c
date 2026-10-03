#include <argp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arguments.h"

const char *argp_program_version = "kenobicmd 1.0";
const char *argp_program_bug_address = "<bugz@blizzeta.net>";

static char doc[] = "Kenobi Command -- a program that will communicate with QMK Kenobi Firmware";

static char args_doc[] = "";

static struct argp_option options[] = {
    {"battery", 'b', 0, 0, "Get battery information"},
    {"keyboard-layout", 'k', 0, 0, "Get keyboard layout"},
    {"lock-status", 'l', 0, 0, "Get keyboard lock status"},
    {"wpm", 'w', 0,0, "Get keyboard words per minute"},
    {"bluetooth", 't', 0, 0, "Enable bluetooth"},
    {"factory-test", 'f', 0, 0, "Run factory test"},
    {"no-banner", 'n', 0, 0, "Don't print the banner"},
    {"strip", 's', 0, 0, "Strip text, leave only numbers"},
    {"daemon", 'd', 0, 0, "Run this as a daemon"},
    {"interval", 'i', "MS", 0, "Specify the refresh interval in milliseconds"},
    {0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
    (void)arg;
    arguments *args = state->input;

    switch(key) {
        case 'b': args->battery = 1;                     break;
        case 'k': args->keyboard_layout = 1;             break;
        case 'l': args->lock_status = 1;                 break;
        case 'w': args->wpm = 1;                         break;
        case 't': args->bluetooth = 1;                   break;
        case 'f': args->factory_test = 1;                break;
        case 'n': args->banner = 0;                      break;
        case 's': args->strip = 1;                       break;
        case 'd': args->daemon = 1;                      break;
        case 'i': args->interval_ms = atoi(arg);    break;
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
    args.battery = 0;
    args.keyboard_layout = 0;
    args.lock_status = 0;
    args.wpm = 0;
    args.bluetooth = 0;
    args.factory_test = 0;
    args.banner = 1;
    args.strip = 0;
    args.daemon = 0;
    args.interval_ms = 1000;

    error_t arg_error = argp_parse(&argp, argc, argv, 0, NULL, &args);
    if(arg_error == EINVAL) {
        fprintf(stderr, "There was an error parsing arguments: %s", strerror(arg_error));
        return (arguments){0};
    }
    return args;
}