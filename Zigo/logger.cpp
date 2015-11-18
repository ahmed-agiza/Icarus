#include "logger.h"

int Logger::info(char *message) {
    if (LOG_LVL <= Verbose) {
        FILE *logFile = fopen(LOG_INFO_PATH, "a+");

        if (logFile) {
            tm currentTime = _getCurrentTime();
            fprintf(logFile, "Information(%d-%d-%d %d:%d:%d): %s\n", currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);
            int closeRC = fclose(logFile);
            printf("Close: %d\n", closeRC);
            return WRITE_SUCCESS;
        }

        fprintf(stderr, "Could not open the log file for writing.\n");

        return WRITE_FAILED;
    }
    return INVALID_LOG_LEVEL;
}
int Logger::warn(char *message) {
    if (LOG_LVL <= Warning) {
        FILE *logFile = fopen(LOG_ERROR_PATH, "a+");

        if (logFile) {
            tm currentTime = _getCurrentTime();
            fprintf(logFile, "Information(%d-%d-%d %d:%d:%d): %s\n", currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);
            int closeRC = fclose(logFile);
            printf("Close: %d\n", closeRC);
            return WRITE_SUCCESS;
        }

        fprintf(stderr, "Could not open the log file for writing.\n");

        return WRITE_FAILED;
    }
    return INVALID_LOG_LEVEL;
}
int Logger::error(char *message) {
    if (LOG_LVL <= Error) {
        FILE *logFile = fopen(LOG_ERROR_PATH, "a+");

        if (logFile) {
            tm currentTime = _getCurrentTime();
            fprintf(logFile, "Error(%d-%d-%d %d:%d:%d): %s\n", currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);
            int closeRC = fclose(logFile);
            printf("Close: %d\n", closeRC);
            return WRITE_SUCCESS;
        }

        fprintf(stderr, "Could not open the log file for writing.\n");

        return WRITE_FAILED;
    }
    return INVALID_LOG_LEVEL;
}

tm Logger::_getCurrentTime() {
    time_t t = time(NULL);
    tm currentTime = *localtime(&t);
    return currentTime;
}
