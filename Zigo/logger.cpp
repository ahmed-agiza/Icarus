#include "logger.h"

int Logger::info(char *message) {
    if (LOG_LVL <= Verbose) {
        return _logMessage(LOG_INFO_PATH, "Infromation", message);
    }
    return INVALID_LOG_LEVEL;
}
int Logger::warn(char *message) {
    if (LOG_LVL <= Warning) {
        return _logMessage(LOG_ERROR_PATH, "Warning", message);
    }
    return INVALID_LOG_LEVEL;
}
int Logger::error(char *message) {
    if (LOG_LVL <= Error) {
        return _logMessage(LOG_ERROR_PATH, "Error", message);
    }
    return INVALID_LOG_LEVEL;
}

tm Logger::_getCurrentTime() {
    time_t t = time(NULL);
    tm currentTime = *localtime(&t);
    return currentTime;
}

int Logger::_logMessage(char *filePath, char *prefix, char *message) {
    FILE *logFile = fopen(filePath, "a+");

    if (logFile) {
        tm currentTime = _getCurrentTime();

        fprintf(logFile, "%s(%d-%d-%d %d:%d:%d): %s\n", prefix, currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);

        fclose(logFile);

        if (LOG_STD) {
            printf("%s(%d-%d-%d %d:%d:%d): %s\n", prefix, currentTime.tm_year + 1900, currentTime.tm_mon + 1, currentTime.tm_mday, currentTime.tm_hour, currentTime.tm_min, currentTime.tm_sec, message);
        }

        return WRITE_SUCCESS;
    }

    fprintf(stderr, "Could not open the log file for writing.\n");

    return WRITE_FAILED;
}
