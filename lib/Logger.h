#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "LogWriter.h"
#include "LogBuffer.h"

enum LogLevel {
    FATAL,
    ERROR,
    WARN,
    INFO,
    DEBUG,
    TRACE
};

class Logger {
    protected:
        LogLevel activeLevel;
        LogWriter *logWriter;
        LogBuffer *logBuffer;

    public:
        Logger();
        void log(LogLevel level, const char* module, const char* text);
        void setLevel(LogLevel level);
        void setLogWriter(LogWriter *logWriter);
        void setBuffer(LogBuffer *logBuffer);

};

#endif