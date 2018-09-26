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
        
        // override this method order to provide correct
        // time zone offset in seconds to method log
        virtual int getTimeZoneOffset(); 

    public:
        Logger();
        void log(LogLevel level, const char* module, const char* text, ...);
        void setLevel(LogLevel level);
        void setLevel(const char *level);
        void setLogWriter(LogWriter *logWriter);
        void setBuffer(LogBuffer *logBuffer);
        void processBuffer();

};

#endif