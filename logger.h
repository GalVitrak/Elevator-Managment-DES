#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

void logger_init(void);
void logger_close(void);
void log_message(double simTime, LogLevel level, const char* message);
void log_event_created(double simTime, const char* eventDesc);
void log_event_handled(double simTime, const char* eventDesc);

#endif /* LOGGER_H */
