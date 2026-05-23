#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

/* Open simulation_log.txt for writing; safe to call once at startup. */
void logger_init(void);

/* Truncate simulation_log.txt before a new simulation run (keeps console table). */
void logger_begin_simulation_run(void);

/* Close the open event-log table borders (keeps log file open for append). */
void logger_close_table_for_append(void);

/* Close log file; call before program exit. */
void logger_close(void);

/*
 * Write one line to console and log file.
 * simTime: current simulation clock (shown as t= in output).
 */
void log_message(double simTime, LogLevel level, const char* message);

/* Log that a new event was scheduled (wraps log_message INFO). */
void log_event_created(double simTime, const char* eventDesc);

/* Log that an event was dispatched to its handler (wraps log_message INFO). */
void log_event_handled(double simTime, const char* eventDesc);

#endif /* LOGGER_H */
