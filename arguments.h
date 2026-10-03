#ifndef ARGUMENTS_H
#define ARGUMENTS_H

#include <argp.h>

typedef struct {
    char *args[6];
    int battery;
    int keyboard_layout;
    int lock_status;
    int wpm;
    int bluetooth;
    int factory_test;
    int banner;
    int strip;
    int daemon;
    unsigned int interval_ms;
}arguments;

int arg_usage(void);

arguments parse(int argc, char **argv);

#endif