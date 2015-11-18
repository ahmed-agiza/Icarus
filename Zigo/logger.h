#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define LOG_ERROR_PATH "error.log"
#define LOG_INFO_PATH "info.log"

#define WRITE_FAILED -1
#define WRITE_SUCCESS 0
#define INVALID_LOG_LEVEL -2


enum LogLevel {
    Verbose = 0,
    Warning = 1,
    Error = 2
};

#define LOG_LVL Verbose

class Logger {
    static tm _getCurrentTime();
public:
    static int info(char *message);
    static int warn(char *message);
    static int error(char *message);
};

#endif
