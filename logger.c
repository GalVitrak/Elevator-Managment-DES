/*
 * logger.c - Dual logging to console and simulation_log.txt (tabular format)
 * PRESENTATION: Demo trail — show log after run; timestamps are simulation time.
 */
#include "logger.h"
#include "constants.h"
#include "text_table.h"

#include <string.h>

static FILE* logFile = NULL;
static int logTableOpenStdout = 0;
static int logTableOpenFile = 0;

static const int LOG_COL_WIDTHS[3] = { 10, 7, 58 };
static const char* LOG_COL_HEADERS[3] = { "Time (s)", "Level", "Message" };

static const char* level_to_string(LogLevel level)
{
    switch (level) {
    case LOG_INFO:    return "INFO";
    case LOG_WARNING: return "WARN";
    case LOG_ERROR:   return "ERROR";
    default:          return "?????";
    }
}

static void logger_open_table_on_stream(FILE* out)
{
    TextTable table;

    if (out == NULL) {
        return;
    }

    text_table_print_banner(out, "SIMULATION EVENT LOG");
    fputc('\n', out);
    text_table_begin(&table, out, LOG_COL_WIDTHS, 3);
    text_table_header(&table, LOG_COL_HEADERS);
}

static void logger_close_table_on_stream(FILE* out)
{
    TextTable table;

    if (out == NULL) {
        return;
    }

    table.out = out;
    table.colCount = 3;
    table.widths[0] = LOG_COL_WIDTHS[0];
    table.widths[1] = LOG_COL_WIDTHS[1];
    table.widths[2] = LOG_COL_WIDTHS[2];
    text_table_end(&table);
}

static void logger_ensure_table_open(void)
{
    if (!logTableOpenStdout) {
        logger_open_table_on_stream(stdout);
        logTableOpenStdout = 1;
    }
    if (logFile != NULL && !logTableOpenFile) {
        logger_open_table_on_stream(logFile);
        logTableOpenFile = 1;
    }
}

static void logger_write_row_to_stream(FILE* out, double simTime, LogLevel level,
                                       const char* message)
{
    char timeBuf[16];
    const char* cells[3];

    if (out == NULL) {
        return;
    }

    snprintf(timeBuf, sizeof(timeBuf), "%.2f", simTime);
    cells[0] = timeBuf;
    cells[1] = level_to_string(level);
    cells[2] = message;

    text_table_row_line(out, LOG_COL_WIDTHS, 3, cells);
}

void logger_init(void)
{
    if (logFile != NULL) {
        return;
    }
    logTableOpenStdout = 0;
    logTableOpenFile = 0;
    logFile = fopen(LOG_FILE_NAME, "w");
    if (logFile == NULL) {
        fprintf(stderr, "Warning: could not open log file '%s'\n", LOG_FILE_NAME);
    }
}

void logger_begin_simulation_run(void)
{
    logger_close_table_for_append();

    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }

    logTableOpenFile = 0;
    logFile = fopen(LOG_FILE_NAME, "w");
    if (logFile == NULL) {
        fprintf(stderr, "Warning: could not open log file '%s'\n", LOG_FILE_NAME);
    }
}

void logger_close_table_for_append(void)
{
    if (logTableOpenStdout) {
        logger_close_table_on_stream(stdout);
        logTableOpenStdout = 0;
    }
    if (logFile != NULL && logTableOpenFile) {
        logger_close_table_on_stream(logFile);
        fflush(logFile);
        logTableOpenFile = 0;
    }
}

void logger_close(void)
{
    logger_close_table_for_append();

    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
}

void log_message(double simTime, LogLevel level, const char* message)
{
    if (message == NULL) {
        message = "";
    }

    logger_ensure_table_open();
    logger_write_row_to_stream(stdout, simTime, level, message);
    if (logFile != NULL) {
        logger_write_row_to_stream(logFile, simTime, level, message);
        fflush(logFile);
    }
}

void log_event_created(double simTime, const char* eventDesc)
{
    char buffer[MAX_NAME_LEN * 4];
    snprintf(buffer, sizeof(buffer), "Event created: %s", eventDesc);
    log_message(simTime, LOG_INFO, buffer);
}

void log_event_handled(double simTime, const char* eventDesc)
{
    char buffer[MAX_NAME_LEN * 4];
    snprintf(buffer, sizeof(buffer), "Event handled: %s", eventDesc);
    log_message(simTime, LOG_INFO, buffer);
}
