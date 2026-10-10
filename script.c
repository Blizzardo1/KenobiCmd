#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "main.h"
#include "arguments.h"

#define MAX_TOK  8
#define LINE_MAX_LEN 256

static int g_strip;

/**
 * @brief Prints an error message indicating incorrect usage and returns -1.
 *
 * @param usage the usage message to print.
 * @return int -1 indicating failure.
 */
static int bad(const char *usage) { lerror("usage: %s", usage); return -1; }

/**
 * @brief Runs the specified information block.
 *
 * @param ib the information block to run.
 * @return int 0 on success, -1 on failure.
 */
static int run_block(InfoBlock *ib) {
    return call(g_strip, ib);
}

#define QUERY(fn, label, cmd, parser)                                      \
    int fn(int argc, char **argv) { (void)argc; (void)argv;         \
        return run_block(&(InfoBlock){ .name = label, .command = cmd,      \
                                       .cb = respond, .parse = parser }); }

QUERY(sc_ping,    "Ping",            COMMAND_GET_PING,      parse_ping)
QUERY(sc_battery, "Get Battery",     COMMAND_BATTERY,       parse_battery)
QUERY(sc_wpm,     "Get WPM",         COMMAND_WPM,           parse_wpm)
QUERY(sc_os,      "Get OS",          COMMAND_GET_OS,        parse_os)
QUERY(sc_version, "Get Version",     COMMAND_GET_VERSION,   parse_version)
QUERY(sc_conn,    "Get Conn Mode",   COMMAND_GET_CONN_MODE, parse_conn_mode)
QUERY(sc_lockst,  "Get Lock Status", COMMAND_LOCK_STATUS,   parse_lock_status)
QUERY(sc_lock,    "Lock Keyboard",   COMMAND_LOCK,          parse_status)


/**
 * @brief Sets the RGB mode for the specified LED.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_rgb_mode(int argc, char **argv) {
    long m;
    (void)argc;
    if (!parse_int(argv[1], 0, 255, &m)) return bad("rgb-mode 0-255");
    return run_block(&(InfoBlock){ .name = "Set RGB Mode", .command = COMMAND_SET_RGB_MODE,
        .cb = respond, .parse = parse_status, .params = { (unsigned char)m } });
}

/**
 * @brief Sets the brightness for the specified LED.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_brightness(int argc, char **argv) {
    long v; (void)argc;
    if (!parse_int(argv[1], 0, 255, &v)) return bad("brightness 0-255");
    return run_block(&(InfoBlock){ .name = "Set Brightness", .command = COMMAND_SET_BRIGHTNESS,
        .cb = respond, .parse = parse_status, .params = { (unsigned char)v } });
}

/**
 * @brief Sets the RGB color for the specified LED.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_rgb_color(int argc, char **argv) {
    long h, s, v; (void)argc;
    if (!parse_int(argv[1], 0, 255, &h) || !parse_int(argv[2], 0, 255, &s) ||
        !parse_int(argv[3], 0, 255, &v)) return bad("rgb-color H S V   (each 0-255)");
    return run_block(&(InfoBlock){ .name = "Set RGB Color", .command = COMMAND_SET_RGB_COLOR,
        .cb = respond, .parse = parse_status,
        .params = { (unsigned char)h, (unsigned char)s, (unsigned char)v } });
}

/**
 * @brief Sets the RGB color for the specified LED.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_led(int argc, char **argv) {
    long idx, rgb, ms = 0;
    if (!parse_int(argv[1], 0, 254, &idx) ||
        !parse_int(argv[2], 0, 0xFFFFFF, &rgb) ||     // We want to write in hexadecimal format, not just hex.
        (argc > 3 && !parse_int(argv[3], 0, 65535, &ms)))
        return bad("led IDX 0xRRGGBB [MS]");
    return run_block(&(InfoBlock){ .name = "Set LED", .command = COMMAND_SET_LED,
        .cb = respond, .parse = parse_status,
        .params = { (unsigned char)idx, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF,
                    (ms >> 8) & 0xFF, ms & 0xFF } });
}

/**
 * @brief Clears the specified LED or all LEDs if no index is provided.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_clear_led(int argc, char **argv) {
    long idx = 0xFF;
    if (argc > 1 && !parse_int(argv[1], 0, 254, &idx)) return bad("clear-led [IDX]");
    return run_block(&(InfoBlock){ .name = "Clear LED", .command = COMMAND_CLEAR_LED,
        .cb = respond, .parse = parse_status, .params = { (unsigned char)idx } });
}

/**
 * @brief Pauses the script execution for the specified number of milliseconds.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_sleep(int argc, char **argv) {
    long ms; (void)argc;
    if (!parse_int(argv[1], 0, 600000, &ms)) return bad("sleep MS   (max 600000)");
    msleep(ms);
    return 0;
}

/**
 * @brief Enters the bootloader mode if the argument is "yes".
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 1 if the bootloader mode is entered, otherwise 0.
 */
static int sc_bootloader(int argc, char **argv) {
    if (argc < 2 || strcmp(argv[1], "yes") != 0) return bad("bootloader yes");
    call(g_strip, &(InfoBlock){ .name = "Enter Bootloader", .command = COMMAND_BOOTLOADER,
        .cb = respond, .skip_read = 1, .message = "Entering bootloader mode...",
        .params = { 0xDE, 0xAD, 0xB0, 0x0B } });
    return 1;   // device is gone: end the session
}

typedef struct { const char *name, *usage; int min, max; int (*fn)(int, char **); } ScriptCmd;

static int sc_help(int argc, char **argv);

static const ScriptCmd table[] = {
    { "ping",       "ping",                     1, 1, sc_ping },
    { "battery",    "battery",                  1, 1, sc_battery },
    { "wpm",        "wpm",                      1, 1, sc_wpm },
    { "os",         "os",                       1, 1, sc_os },
    { "version",    "version",                  1, 1, sc_version },
    { "conn-mode",  "conn-mode",                1, 1, sc_conn },
    { "lock-status","lock-status",              1, 1, sc_lockst },
    { "lock",       "lock",                     1, 1, sc_lock },
    { "rgb-mode",   "rgb-mode N",               2, 2, sc_rgb_mode },
    { "rgb-color",  "rgb-color H S V",          4, 4, sc_rgb_color },
    { "brightness", "brightness N",             2, 2, sc_brightness },
    { "led",        "led IDX 0xRRGGBB [MS]",    3, 4, sc_led },
    { "clear-led",  "clear-led [IDX]",          1, 2, sc_clear_led },
    { "sleep",      "sleep MS",                 2, 2, sc_sleep },
    { "bootloader", "bootloader yes",           2, 2, sc_bootloader },
    { "help",       "help",                     1, 1, sc_help },
};
#define NCMDS (sizeof table / sizeof table[0])

/**
 * @brief Displays help information for the available script commands.
 *
 * @param argc the number of arguments.
 * @param argv the array of arguments.
 * @return int 0 on success.
 */
static int sc_help(int argc, char **argv) {
    (void)argc; (void)argv;
    for (size_t i = 0; i < NCMDS; i++) printf("  %s\n", table[i].usage);
    puts("  quit | exit");
    return 0;
}

/**
 * @brief Tokenizes a line of input into an array of arguments.
 *
 * @param line the line of input to tokenize.
 * @param argv the array to store the tokenized arguments.
 * @param max the maximum number of arguments to store.
 * @return int the number of arguments successfully tokenized, or -1 if the maximum number of arguments is exceeded.
 */
static int tokenize(char *line, char **argv, int max) {
    int n = 0;
    char *p = line;
    while (*p) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p || *p == '#') break;
        if (n == max) return -1;
        argv[n++] = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p) *p++ = '\0';
    }
    return n;
}

/**
    @brief Runs a script from the given input file stream, handling interactive mode and stripping certain elements.
    @param in the input file stream.
    @param interactive whether to run in interactive mode.
    @param strip whether to strip certain elements.
    @return 0 on success, nonzero on error or early stop.
*/
int run_script(FILE *in, int interactive, int strip) {
    char line[LINE_MAX_LEN];
    int lineno = 0, rc = 0;
    int prompt = interactive && isatty(fileno(in));
    g_strip = strip;

    for (;;) {
        if (prompt) { fputs("kenobi> ", stdout); fflush(stdout); }
        if (!fgets(line, sizeof line, in)) break; // EOF / Ctrl-D
        lineno++;

        if (!strchr(line, '\n') && !feof(in)) { // overlong line: discard the rest
            int c; while ((c = fgetc(in)) != '\n' && c != EOF) {}
            lerror("line %d: too long", lineno);
            rc = 1;
            if (!interactive) break;
            continue;
        }

        char *argv[MAX_TOK];
        int argc = tokenize(line, argv, MAX_TOK);
        if (argc < 0) { lerror("line %d: too many arguments", lineno); rc = 1; if (!interactive) break; continue; }
        if (argc == 0) continue;

        if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit")) break;

        const ScriptCmd *cmd = NULL;
        for (size_t i = 0; i < NCMDS; i++)
            if (!strcmp(argv[0], table[i].name)) { cmd = &table[i]; break; }

        if (!cmd) { lerror("line %d: unknown command '%s' (try 'help')", lineno, argv[0]); rc = 1; }
        else if (argc < cmd->min || argc > cmd->max) { lerror("line %d: usage: %s", lineno, cmd->usage); rc = 1; }
        else {
            int r = cmd->fn(argc, argv);
            if (r > 0) return 0; // bootloader: stop it cleanly
            if (r < 0) { rc = 1; }
        }

        if (rc && !interactive) { lerror("stopped at line %d", lineno); return rc; }
        if (interactive) rc = 0; // So errors won't stick in a session
    }
    return rc;
}
