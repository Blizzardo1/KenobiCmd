#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "main.h"
#include "pretty.h"
#include "arguments.h"

// #define DEBUG

struct hid_device_info *hdi;
hid_device *handle;

int out_lines = 0;
int is_daemon = 0;


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
        case COMMAND_GET_OS:
            name = "Get OS Command";
            break;
        case COMMAND_GET_CONN_MODE:
            name = "Get Connection Mode Command";
            break;
        case COMMAND_GET_PING:
            name = "Get Ping Command";
            break;
        case COMMAND_GET_VERSION:
            name = "Get Version Command";
            break;
        case COMMAND_SET_BRIGHTNESS:
            name = "Set Brightness Command";
            break;
        case COMMAND_SET_RGB_MODE:
            name = "Set RGB Mode Command";
            break;
        case COMMAND_SET_RGB_COLOR:
            name = "Set RGB Color Command";
            break;
        case COMMAND_SET_LED:
            name = "Set LED Command";
            break;
        case COMMAND_CLEAR_LED:
            name = "Clear LED Command";
            break;
        case COMMAND_LOCK:
            name = "Lock Command";
            break;
        case COMMAND_BOOTLOADER:
            name = "Bootloader Command";
            break;
        case COMMAND_EVENT:
            name = "Event Command";
            break;
        }

    return name;
}

int lerror(char *msg, ...) {
    char rmsg[MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    out_lines++;
    return fprintf(stderr, "\x1b[1;31m%s\x1b[0m\n", rmsg);
}

int lwarn(char *msg, ...) {
    char rmsg[MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    out_lines++;
    return fprintf(stdout, "\x1b[1;33mWarning: %s\x1b[0m\n", rmsg);
}

int linfo(char *msg, ...) {
    char rmsg[MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    out_lines++;
    return fprintf(stdout, "\x1b[1;34mInfo: %s\x1b[0m\n", rmsg);
}

int ldebug(char *msg, ...) {
#ifdef DEBUG
    char rmsg[MESSAGE_BUFFER];
    va_list varg;
    va_start(varg, msg);
    vsnprintf(rmsg, sizeof(rmsg), msg, varg);
    va_end(varg);
    out_lines++;
    return fprintf(stdout, "\x1b[1;35mDebug: %s\x1b[0m\n", rmsg);
#else
    return 0;
#endif
}

void cleanup() {
    ldebug("Cleaning up...");
    if(handle != NULL)
        hid_close(handle);
    hid_exit();

#ifdef DEBUG
    success();
#endif

    ldebug("Goodbye :')");
}

void segfault(int trap) {
    lerror("A Segmentation fault happened! Please check the source and try again! Trap: %d", trap);
    exit(EXIT_FAILURE);
}

void trap(int trap) {
    linfo("Stopping Daemon.");
    if(trap == SIGABRT) {
        lerror("Something happened while terminating session. Please check the output and open an issue on GitHub.");
    }
    is_daemon = 0;
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
    signal(SIGINT, trap);
    int res = hid_init();
    if(res == 1) {
        lerror("Unable to initialize HID!");
        return;
    }
#ifdef DEBUG
    success();
#endif
}

static int read_matching(unsigned char command, unsigned char *out) {
    for (int tries = 0; tries < 8; tries++) {
        int n = read_response(out);
        if (n <= 0) return n;
        if (out[0] == command) return n;
        if (out[0] == COMMAND_EVENT) {
            ldebug("event %02X %02X", out[1], out[2]);
            continue;
        }
        ldebug("discarding stale frame %02X", out[0]);
    }
    return 0;
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

void parse_watch_event(int strip, unsigned char * data) {
	uint8_t event = data[1];
	if(strip) {
		printf("%s\n", event);
		return;
	}
	// Here is where we can parse the events.
	// It's simple, The data that we pass in is going to be the command that was parsed.
	// It's a basic watcher which means the data flow is going to be double transmitted,
	// from my understanding.
}

void parse_status(int strip, unsigned char *data) {
    static const char *why[] = { "OK", "value out of range", "bad length", "denied" };
    uint8_t s = data[1];
    if (s == 0) {
        if (!strip)
            linfo("OK");
        return;
    }
    lerror("Device error: %s (0x%02X)", s < 4 ? why[s] : "unknown", s);
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

void parse_version(int strip, unsigned char *data) {
    uint8_t major = (uint8_t)data[1];
    uint8_t minor = (uint8_t)data[2];

    if(strip) {
        printf("%d.%d\n", major, minor);
        return;
    }

    linfo("Version: %d.%d", major, minor);
}

void parse_conn_mode(int strip, unsigned char *data) {
    transport mode = (uint8_t)data[1];

    char *conn_mode;

    switch(mode) {
        case TRANSPORT_USB:
            conn_mode = "USB";
            break;
        case TRANSPORT_BLUETOOTH:
            conn_mode = "BLE";
            break;
        default:
            conn_mode = "Unknown";
            break;
    }

    if(strip) {
        printf("%s", conn_mode);
        return;
    }

    linfo("Connection Mode: %s", conn_mode);
}

void parse_ping(int strip, unsigned char *data) {
    // ping is an echo reply, as in echo back what was sent
    // I hate you AI
    if(strip) {
        printf("Ping received\n");
        return;
    }

    linfo("Ping received");
}

void parse_os(int strip, unsigned char *data) {
    // This will return a number. We need to interpret it based on the OS mapping.
    uint8_t os = (uint8_t)data[1];

    os_variant detected = (os_variant)os;
    char *os_name;
    switch(detected) {
        case OS_UNSURE:
            os_name = "Unsure";
            break;
        case OS_LINUX:
            os_name = "Linux";
            break;
        case OS_WINDOWS:
            os_name = "Windows";
            break;
        case OS_MACOS:
            os_name = "macOS";
            break;
        case OS_IOS:
            os_name = "iOS";
            break;
    }

    if(strip) {
        printf("%s", os_name);
        return;
    }

    linfo("OS: %s", os_name);
}

int respond(COMMANDS command, unsigned char buffer[PARAM_BUFF], int skip_read) {
    int sz = send_command(command, buffer);
    if (sz < 0) {
        lerror("Unable to write to device: %ls", hid_error(handle));
        return -1;
    }
    ldebug("%d bytes written", sz);

    if (skip_read) return 0;

    unsigned char resbuffer[sz];

    int n = read_matching((char)command, resbuffer);

    if (n < 0)
        return -1;

    if (n == 0) {
        lwarn("No reply from device.");
        return 1;
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

int call(int strip, InfoBlock *ib) {
    if (!handle) {
        ERRNEO;
        return -1;
    }

    if(ib->skip_read && ib->message) {
        linfo("%s", ib->message);
    }
    ldebug("Calling %s", ib->name);

    unsigned char buffer[PARAM_BUFF];
    memcpy(buffer, ib->params, sizeof(buffer));

    int res = ib->cb(ib->command, buffer, ib->skip_read);

    if(res == -1) {
        cleanup();
        exit(EXIT_FAILURE);
    }

    if(res == 1) {
        lwarn("No Reply");
        return -1;
    }

    if(ib->parse)
        ib->parse(strip, buffer);
#ifdef DEBUG
    printf("------\n");
#endif
    return 0;
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

/**
 * @brief Rewind the terminal to the beginning of the output block
 */
static void rewind_out(void) {
    if(!isatty(STDOUT_FILENO)) return;
    if(out_lines > 0) {
        printf("\x1b[%dA", out_lines);
    }
    printf("\r\x1b[J");
    fflush(stdout);
    out_lines = 0;
}

int msleep(long msec) {
    struct timespec ts;
    int res;

    if(msec < 0) {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
    return res;
}

static void run_requested_commands(const arguments *args) {

    if (args->flags.set_rgb_mode) {
        call(args->flags.strip, &(InfoBlock) {
                .name = "Set RGB Mode",
                .command = COMMAND_SET_RGB_MODE,
                .cb = respond,
                .parse = parse_status,
                .message = "Setting the RGB mode...",
                .params = { args->values.rgb_mode }
            });
    }

    if (args->flags.set_rgb_color) {
        call(args->flags.strip, &(InfoBlock) {
                .name = "Set RGB Color",
                .command = COMMAND_SET_RGB_COLOR,
                .cb = respond,
                .parse = parse_status,
                .message = "Setting the RGB color...",
                .params = { args->values.hsv[0],
                    args->values.hsv[1],
                    args->values.hsv[2]
                }
            });
    }
    if (args->flags.set_brightness) {
        call(args->flags.strip, &(InfoBlock) {
                .name = "Set Brightness",
                .command = COMMAND_SET_BRIGHTNESS,
                .cb = respond,
                .parse = parse_status,
                .message = "Setting the brightness...",
                .params = { args->values.brightness }
        });
    }
    if (args->flags.clear_led) {
        call(args->flags.strip, &(InfoBlock) {
                .name = "Clear LED",
                .command = COMMAND_CLEAR_LED,
                .cb = respond,
                .parse = parse_status,
                .message = "Clearing the LED...",
                .params = { args->values.clear_idx }
        });
    }
    if (args->flags.set_led) {
        call(args->flags.strip, &(InfoBlock) {
                .name = "Set LED",
                .command = COMMAND_SET_LED,
                .cb = respond,
                .parse = parse_status,
                .message = "Setting the LED...",
                .params = { args->values.led_idx,
                    args->values.led_rgb[0], args->values.led_rgb[1], args->values.led_rgb[2],
                    args->values.led_ms >> 8, args->values.led_ms & 0xFF }
        });
    }

    if (args->flags.lock) {
        call(args->flags.strip, &(InfoBlock) {
                .name = "Lock Keyboard",
                .command = COMMAND_LOCK,
                .cb = respond,
                .parse = parse_status,
                .message = "Locking the keyboard..."
            });
    }

    if(args->flags.battery) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get Battery Info",
            .command = COMMAND_BATTERY,
            .cb = respond,
            .parse = parse_battery
        });
    }

    if(args->flags.keyboard_layout) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get Layout Info",
            .command = COMMAND_LAYOUT,
            .cb = respond,
            .parse = parse_keyboard_layout
        });
    }

    if(args->flags.wpm){
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get Words Per Minute",
            .command = COMMAND_WPM,
            .cb = respond,
            .parse = parse_wpm
        });
    }

    if(args->flags.lock_status) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get Lock Status",
            .command = COMMAND_LOCK_STATUS,
            .cb = respond,
            .parse = parse_lock_status
        });
    }

    if(args->flags.set_brightness) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Set Brightness",
            .command = COMMAND_SET_BRIGHTNESS,
            .cb = respond,
            .skip_read = 1,
            .message = "Setting the brightness..."
        });
    }

    if(args->flags.get_version) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get Version",
            .command = COMMAND_GET_VERSION,
            .cb = respond,
            .parse = parse_version
        });
    }

    if(args->flags.get_conn_mode) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get Connection Mode",
            .command = COMMAND_GET_CONN_MODE,
            .cb = respond,
            .parse = parse_conn_mode
        });
    }

    if(args->flags.ping) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Ping",
            .command = COMMAND_GET_PING,
            .cb = respond,
            .parse = parse_ping
        });
    }

    if(args->flags.get_os) {
        call(args->flags.strip, &(InfoBlock) {
            .name = "Get OS",
            .command = COMMAND_GET_OS,
            .cb = respond,
            .parse = parse_os
        });
    }

    if(args->flags.get_event) {
    	call(args->flags.strip, &(InfoBlock) {
	    .name = "Get Event",
	    .command = COMMAND_EVENT,
	    .cb = respond,
	    .parse = parse_watch_event
	});
    }

    if (args->flags.bootloader) {
        call(args->flags.strip,
            &(InfoBlock){
                .name = "Enter Bootloader",
                .command = COMMAND_BOOTLOADER,
                .cb = respond,
                .skip_read = 1,
                .message = "Entering bootloader mode...",
                .params = { 0xDE, 0xAD, 0xB0, 0x0B }
        });
    }
}

void loop(const arguments *args) {
    do {
        ldebug("While args.daemon == 1 (%d); Sleep for (%dms)", args->flags.daemon, args->interval_ms);
        run_requested_commands(args);
        if(!is_daemon) break;
        msleep(args->interval_ms);
        rewind_out();
    } while(is_daemon);
}

int main(int argc, char **argv) {
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

    if(args.flags.banner) {
        print_info();
    }

    ldebug("Interval set at %dms", args.interval_ms);

    if(args.flags.bluetooth) {
        call(0, &(InfoBlock) {
            .name = "Enable Bluetooth",
            .command = COMMAND_BLUETOOTH_ENABLE,
            .cb = respond,
            .skip_read = 1,
            .message = "I can send a bluetooth connect request, but note you need to switch over to Bluetooth if your device has a switch for it."
        });
    }

    if(args.flags.factory_test) {
        call(0, &(InfoBlock) {
            .name = "Run Factory Test",
            .command = COMMAND_FACTORY_TEST,
            .cb = respond,
            .skip_read = 1,
            .message = "Performing factory test..."
        });
    }

    is_daemon = args.flags.daemon;

    if (args.script_path || args.flags.interactive) {
        signal(SIGINT, SIG_DFL);      // the daemon trap() isn't wanted here; Ctrl-C should just exit
        int rc = 0;
        if (args.script_path) {
            FILE *f = strcmp(args.script_path, "-") == 0 ? stdin : fopen(args.script_path, "r");
            if (!f) { lerror("Cannot open %s", args.script_path); cleanup(); return EXIT_FAILURE; }
            rc = run_script(f, 0, args.flags.strip);
            if (f != stdin) fclose(f);
        } else {
            rc = run_script(stdin, 1, args.flags.strip);
        }
        cleanup();
        return rc ? EXIT_FAILURE : 0;
    }
    loop(&args);

    cleanup();

    return EXIT_SUCCESS;
}
