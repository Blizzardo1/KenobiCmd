#ifndef MAIN_H
#define MAIN_H

#include <stdarg.h>
#include <stdint.h>
#include <hidapi/hidapi.h>


#define VENDOR_ID 0x3434
#define PRODUCT_ID 0x2A0
#define USAGE_PAGE 0xFF60
#define USAGE_ID 0x61

#define __MESSAGE_BUFFER 1024

#define BUFF_SIZE 32
#define BUFF_OFF 2
#define PARAM_BUFF (BUFF_SIZE - BUFF_OFF)

#define RESPONSE_TIMEOUT 3000


/**
 * @brief Error - Keyboard Never Opened for reading/writing
 */
#define ERRNEO lerror("Keyboard was never opened for reading/writing!")

typedef enum {
    COMMAND_A = 0x41,
    COMMAND_BATTERY = 0xC0,
    COMMAND_LAYOUT = 0xC1,
    COMMAND_LOCK_STATUS = 0xC2,
    COMMAND_WPM = 0xC3,
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
} InfoBlock;


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
 * @return int Returns 0 on success; else -1 for failure.
 */
int respond(COMMANDS command, unsigned char buffer[PARAM_BUFF], int skip_read);

/**
 * @brief Get battery information.
 *
 * @return int Returns 0 on success; else -1 for failure.
 */
int get_battery_info();

/**
 * @brief Get the device layout information.
 *
 * @return int Returns 0 on success; else -1 for failure.
 */
int get_layout_info();

/**
 * @brief Get the current lock status of the device.
 *
 * @return int Returns 0 on success; else -1 for failure.
 */
int get_lock_status();

#endif