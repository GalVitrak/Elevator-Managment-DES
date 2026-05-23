#include "logger.h"
#include "constants.h"

#include <string.h>

static FILE* logFile = NULL;

static const char* level_to_string(LogLevel level)
{
    switch (level) {
    case LOG_INFO:    return "INFO";
    case LOG_WARNING: return "WARN";
    case LOG_ERROR:   return "ERROR";
    default:          return "UNKNOWN";
    }
}

void logger_init(void)
{
    if (logFile != NULL) {
        return;
    }
    logFile = fopen(LOG_FILE_NAME, "w");
    if (logFile == NULL) {
        fprintf(stderr, "Warning: could not open log file '%s'\n", LOG_FILE_NAME);
    }
}

void logger_close(void)
{
    if (logFile != NULL) {
        fclose(logFile);
        logFile = NULL;
    }
}

void log_message(double simTime, LogLevel level, const char* message)
{
    char buffer[MAX_NAME_LEN * 4];
    snprintf(buffer, sizeof(buffer), "[t=%.2f][%s] %s",
             simTime, level_to_string(level), message);

    printf("%s\n", buffer);

    if (logFile != NULL) {
        fprintf(logFile, "%s\n", buffer);
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
