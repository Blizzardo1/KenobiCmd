#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include "main.h"
#include "pretty.h"
#include "arguments.h"

// #define DEBUG

struct hid_device_info *hdi;
hid_device *handle;


char* get_name(COMMANDS command) {
    char *name;
    switch(command) {
        case COMMAND_A:
            name = "(A) Command";
            break;
        case COMMAND_BATTERY:
            name = "Battery Command";
            break;
        case COMMAND_LAYOUT:
            name = "Layout Command";
            break;
        case COMMAND_LOCK_STATUS:
            name = "Lock Status Command";
            break;
        case COMMAND_BLUETOOTH_ENABLE:
            name = "Bluetooth Enable Command";
            break;
        case COMMAND_WPM:
            name = "Words Per Minute";
            break;
        case COMMAND_FACTORY_TEST:
            name = "Factory Test Command";
            break;
    }

    return name;
}

int lerror(char *msg, ...) {
    char rmsg[__MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    return fprintf(stderr, "\x1b[1;31m%s\x1b[0m\n", rmsg);
}

int lwarn(char *msg, ...) {
    char rmsg[__MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    return fprintf(stdout, "\x1b[1;33mWarning: %s\x1b[0m\n", rmsg);
}

int linfo(char *msg, ...) {
    char rmsg[__MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    return fprintf(stdout, "\x1b[1;34mInfo: %s\x1b[0m\n", rmsg);
}

int ldebug(char *msg, ...) {
#ifdef DEBUG
    char rmsg[__MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    return fprintf(stdout, "\x1b[1;35mDebug: %s\x1b[0m\n", rmsg);
#else
    return 0;
#endif
}

void segfault(int trap) {
    lerror("A Segmentation fault happened! Please check the source and try again! Trap: %d", trap);
    exit(EXIT_FAILURE);
}

void success() {
    printf("\x1b[1;32msuccess!\x1b[0m\n");
}

void failure() {
    printf("\x1b[1;31mfailure!\x1b[0m\n");
}

void initialize() {
#ifdef DEBUG
    printf("Initializing... ");
#endif
    signal(SIGSEGV, segfault);
    int res = hid_init();
    if(res == 1) {
        lerror("Unable to initialize HID!");
        return;
    }
#ifdef DEBUG
    success();
#endif
}

void cleanup() {
    ldebug("Cleaning up...");
    if(handle != NULL)
        hid_close(handle);
    hid_exit();
    ldebug("Goodbye :')");
}

void print_buffer(const unsigned char *buffer) {
#ifdef DEBUG
    for(int j = 0; j < BUFF_SIZE; j++) {
        printf("%02X", buffer[j]);
        if (j + 1 < BUFF_SIZE) {
            printf(" ");
        }
    }
    printf("\n");
#else
    (void)buffer;
#endif
}

int send_command(COMMANDS command, unsigned char data[PARAM_BUFF]) {
    if(handle == NULL) {
        ERRNEO;
        return -1;
    }
    unsigned char buffer[BUFF_SIZE + 1] = {0};
    buffer[0] = 0x00;
    buffer[1] = (unsigned char)command;

    for(int i = BUFF_OFF; i < BUFF_SIZE - 1; i++) {
        buffer[i] = data[i - BUFF_OFF];
    }

    ldebug("Running %s (%02X)", get_name(command), command);
    print_buffer(buffer);

    return hid_write(handle, buffer, sizeof(buffer));
}

int read_response(unsigned char *response) {
    if(handle == NULL) {
        ERRNEO;
        return -1;
    }

    int res = hid_read_timeout(handle,
                    response,
                    BUFF_SIZE,
                    RESPONSE_TIMEOUT);


    if(res < 0) {
        lerror("Error reading from device: %ls", hid_error(handle));
    } else if(res == 0) {
        lerror("Timed out waiting for response: %ls", hid_error(handle));
    }

    return res;
}

void parse_battery(int strip, unsigned char *data) {
    uint8_t percent = (uint8_t)data[1];
    float voltage = (float)((data[2] << 8) + data[3]) / 1000.0f;

    ldebug("What is data? %s", data);

    if(strip) {
        printf("%d%% %.3f\n", percent, voltage);
        return;
    }

    linfo("Battery: %d%%; Voltage: %.3f", percent, voltage);
}

void parse_keyboard_layout(int strip, unsigned char *data) {
    uint8_t layout = (uint8_t)data[1];

    if(strip) {
        printf("%d\n", layout);
        return;
    }

    linfo("Keyboard Layer: %d", layout);
}

void parse_lock_status(int strip, unsigned char *data) {
    uint8_t lock = (uint8_t)data[1];

    if(strip) {
        printf("%d\n", lock);
        return;
    }

    linfo("Lock Status: %s", lock == 0 ? "Unlocked" : "Secured");
}

void parse_wpm(int strip, unsigned char *data) {
    uint8_t wpm = (uint8_t)data[1];

    if(strip) {
        printf("%d\n", wpm);
        return;
    }

    linfo("Words Per Minute: %d", wpm);
}

int respond(COMMANDS command, unsigned char buffer[PARAM_BUFF], int skip_read) {
    int sz = send_command(command, buffer);
    ldebug("%d bytes written", sz);
    unsigned char resbuffer[sz];
    if(skip_read) return 0;
    int res = read_response(resbuffer);
    ldebug("%d bytes read", res);

    if(res == -1) {
        lerror("Unable to read response: %ls", hid_error(handle));
        return -1;
    }

    if(res == 0) {
        lwarn("No data was returned.");
        return 0;
    }

    if (resbuffer[0] != command) {
        lwarn("Unexpected command response. Please see below.");
    }

    print_buffer(resbuffer);

    memcpy(buffer, resbuffer, PARAM_BUFF);

    return 0;
}

void send_feature_report() {
    if(!handle) {
        ERRNEO;
        return;
    }

    ldebug("Sending feature report...");
    unsigned char buffer[PARAM_BUFF] = {0};
    hid_send_feature_report(handle, buffer, sizeof(buffer));
}

void call(int strip, InfoBlock *ib) {
    if (!handle) {
        ERRNEO;
        return;
    }

    if(ib->skip_read && ib->message) {
        linfo("%s", ib->message);
    }
    ldebug("Calling %s", ib->name);
    unsigned char buffer[PARAM_BUFF] = {0};
    int res = ib->cb(ib->command, buffer, ib->skip_read);
    if(res == -1) {
        cleanup();
        exit(EXIT_FAILURE);
    }
    if(ib->parse)
        ib->parse(strip, buffer);
#ifdef DEBUG
    printf("------\n");
#endif
}

void print_info() {
    if(!handle) {
        ERRNEO;
        return;
    }

    hdi = hid_get_device_info(handle);
    pretty_println_info("Hardware Manufacturer", hdi->manufacturer_string, GREEN, YELLOW, true);
    pretty_println_info("Product", hdi->product_string, GREEN, YELLOW, true);
    pretty_println_info("Serial Number", hdi->serial_number, GREEN, YELLOW, true);
}

static hid_device* open_device(void) {
    struct hid_device_info *devices = hid_enumerate(VENDOR_ID, PRODUCT_ID);
    hid_device *device = NULL;

    for(struct hid_device_info *cur = devices; cur; cur = cur ->next) {
        if(cur->usage_page == USAGE_PAGE && cur->usage == USAGE_ID) {
            device = hid_open_path(cur->path);
            break;
        }
    }
    hid_free_enumeration(devices);
    return device;
}

void usage(void) {
    (void)0;
}

int main(int argc, char **argv) {
    if(argc < 2) {
        usage();
    }

    arguments args = parse(argc, argv);

    initialize();

#ifdef DEBUG
    printf("Opening Device... ");
#endif
    handle = open_device();

    if(handle == NULL) {
        hid_error(NULL);
        lerror("Unable to open Keyboard Device!");
        cleanup();
        return EXIT_FAILURE;
    }

#ifdef DEBUG
    success();
#endif

    hid_set_nonblocking(handle, 0);

    if(args.banner) {
        print_info();
    }

    if(args.battery) {
        call(args.strip, &(InfoBlock){
            .name = "Get Battery Info",
            .command = COMMAND_BATTERY,
            .cb = respond,
            .parse = parse_battery
        });
    }

    if(args.keyboard_layout) {
        call(args.strip, &(InfoBlock){
            .name = "Get Layout Info",
            .command = COMMAND_LAYOUT,
            .cb = respond,
            .parse = parse_keyboard_layout
        });
    }

    if(args.wpm){
        call(args.strip, &(InfoBlock){
            .name = "Get Words Per Minute",
            .command = COMMAND_WPM,
            .cb = respond,
            .parse = parse_wpm
        });
    }

    if(args.lock_status) {
        call(args.strip, &(InfoBlock) {
            .name = "Get Lock Status",
            .command = COMMAND_LOCK_STATUS,
            .cb = respond,
            .parse = parse_lock_status
        });
    }

    if(args.bluetooth) {
        call(0, &(InfoBlock) {
            .name = "Enable Bluetooth",
            .command = COMMAND_BLUETOOTH_ENABLE,
            .cb = respond,
            .skip_read = 1,
            .message = "I can send a bluetooth connect request, but note you need to switch over to Bluetooth if your device has a switch for it."
        });
    }

    if(args.factory_test) {
        call(0, &(InfoBlock) {
            .name = "Run Factory Test",
            .command = COMMAND_FACTORY_TEST,
            .cb = respond,
            .skip_read = 1,
            .message = "Performing factory test..."
        });
    }

    cleanup();

    return EXIT_SUCCESS;
}
