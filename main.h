#ifndef MAIN_H
#define MAIN_H

#include <stdarg.h>
#include <stdint.h>
#include <hidapi/hidapi.h>


#define VENDOR_ID 0x3434
#define PRODUCT_ID 0x2A0
#define USAGE_PAGE 0xFF60
#define USAGE_ID 0x61

#define MESSAGE_BUFFER 1024

#define BUFF_SIZE 32
#define BUFF_OFF 2
#define PARAM_BUFF (BUFF_SIZE - BUFF_OFF)

#define RESPONSE_TIMEOUT 3000


/**
 * @brief Error - Keyboard Never Opened for reading/writing
 */
#define ERRNEO lerror("Keyboard was never opened for reading/writing!")

typedef enum {
    TRANSPORT_NONE,
    TRANSPORT_USB,
    TRANSPORT_BLUETOOTH,
} transport;

typedef enum {
    OS_UNSURE,
    OS_LINUX,
    OS_WINDOWS,
    OS_MACOS,
    OS_IOS,
} os_variant;

typedef enum {
    COMMAND_A = 0x41,
    COMMAND_BATTERY = 0xC0,
    COMMAND_LAYOUT = 0xC1,
    COMMAND_LOCK_STATUS = 0xC2,
    COMMAND_WPM = 0xC3,
    COMMAND_GET_OS = 0xC4,
    COMMAND_GET_CONN_MODE = 0xC5,
    COMMAND_GET_PING = 0xC6,
    COMMAND_GET_VERSION = 0xC7,
    COMMAND_SET_BRIGHTNESS = 0xC8,
    COMMAND_SET_RGB_MODE = 0xC9,
    COMMAND_SET_RGB_COLOR = 0xCA,
    COMMAND_SET_LED = 0xCB,
    COMMAND_CLEAR_LED = 0xCC,
    COMMAND_LOCK = 0xCD,
    COMMAND_BOOTLOADER = 0xCE,
    COMMAND_EVENT = 0xCF,
    COMMAND_BLUETOOTH_ENABLE = 0xAA,
    COMMAND_FACTORY_TEST = 0xAB
} COMMANDS;


typedef int (*callback)(COMMANDS command, unsigned char buffer[PARAM_BUFF], int skip_read);
typedef void (*parse_cb)(int strip, unsigned char *data);

typedef struct {
    char  *name;
    COMMANDS command;
    callback cb;
    parse_cb parse;
    int skip_read;
    char *message;
    unsigned char params[PARAM_BUFF];
} InfoBlock;

/**
 * @brief Runs the script from the given input file, optionally in interactive mode and with optional stripping of output.
 *
 * @remark This function is wired to script.c
 *
 * @param in The input file to read the script from.
 * @param interactive Whether to run the script in interactive mode.
 * @param strip Whether to strip output.
 * @return int 0 on success, -1 on failure.
 */
int run_script(FILE *in, int interactive, int strip);

/**
 * @brief Get the name from the COMMANDS enum field.
 *
 * @param command The enum value to translate.
 * @return char* A string representation of the COMMANDS enum value.
 */
char* get_name(COMMANDS command);

/**
 * @brief Log an error to standard error.
 *
 * @param msg The message (optionally formatted).
 * @param ... (Variadic) argument list.
 * @return int Returns the number of bytes written to standard error.
 */
int lerror(char *msg, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief Log a warning to standard out.
 *
 * @param msg The message (optionally formatted).
 * @param ... (Variadic) argument list.
 * @return int Returns the number of bytes written to standard out.
 */
int lwarn(char *msg, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief Log information to standard out.
 *
 * @param msg The message (optionally formatted).
 * @param ... (Variadic) argument list.
 * @return int Returns the number of bytes written to standard out.
 */
int linfo(char *msg, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief Log debug to standard out.
 *
 * @param msg The message (optionally formatted).
 * @param ... (Variadic) argument list.
 * @return int Returns the number of bytes written to standard out.
 */
int ldebug(char *msg, ...) __attribute__((format(printf, 1, 2)));

/**
 * @brief Traps a SIGSEGV fault.
 *
 * @param trap Trap Code.
 */
void segfault(int trap);

/**
 * @brief Prints out a Success.
 */
void success();

/**
 * @brief Prints out a Failure.
 */
void failure();

/**
 * @brief Initializes HID API library.
 */
void initialize();

/**
 * @brief Does any necessary cleanup.
 */
void cleanup();

/**
 * @brief Calls the appropriate handler for the given InfoBlock, optionally stripping output.
 *
 * @param strip Whether to print information or just the raw data.
 * @param ib The InfoBlock containing the data to be processed.
 */
int call(int strip, InfoBlock *ib);

/**
 * @brief Sends a command to the HID device when opened.
 *
 * @param command A number usually in Hex that will be constructed into the sending buffer.
 * @param data Optional data, else pass {0}.
 * @return int Returns the number of bytes written to the HID device.
 */
int send_command(COMMANDS command, unsigned char data[PARAM_BUFF]);

/**
 * @brief Reads from the HID device.
 *
 * @param response The field to be filled with data received by the HID device.
 * @return int Returns the number of bytes read from the HID device.
 */
int read_response(unsigned char *response);

/**
 * @brief Send a command and receive a response
 *
 * @param command A number usually in Hex that will be constructed into the sending buffer.
 * @return int Returns 0 on success; 1 if the device sent no reply; -1 for failure.
 */
int respond(COMMANDS command, unsigned char buffer[PARAM_BUFF], int skip_read);

/**
 * @brief Sleep for a specified time in milliseconds.
 *
 * @param msec the amount of time to sleep in ms.
 * @return int 0 on success; -1 on failure;
 */
int msleep(long msec);

/**
 * @brief Parses the watch events response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_watch_event(int strip, unsigned char *data);

/**
 * @brief Parses the status response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_status(int strip, unsigned char *data);

/**
 * @brief Parses the battery status response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_battery(int strip, unsigned char *data);

/**
 * @brief Parses the keyboard layout response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_battery(int strip, unsigned char *data);

/**
 * @brief Parses the keyboard layout response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */

void parse_keyboard_layout(int strip, unsigned char *data);

/**
 * @brief Parses the lock status response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_lock_status(int strip, unsigned char *data);

/**
 * @brief Parses the WPM (words per minute) response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_wpm(int strip, unsigned char *data);


/**
 * Parses the version response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_version(int strip, unsigned char *data);


/**
 * @brief Parses the connection mode response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_conn_mode(int strip, unsigned char *data);


/**
 * @brief Parses the ping response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_ping(int strip, unsigned char *data);

/**
 * @brief Parses the operating system response from the HID device.
 *
 * @param strip Whether to print information or just the raw data.
 * @param data The raw data received from the HID device.
 */
void parse_os(int strip, unsigned char *data);

#endif