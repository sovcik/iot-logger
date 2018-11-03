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
        // time in ISO format to method log
        virtual void getISOTime(char *buff, size_t size);

    public:
        Logger();
        virtual ~Logger();
        void log(LogLevel level, const char* module, const char* text, ...);
        void setLevel(LogLevel level);
        void setLevel(const char *level);
        void setLogWriter(LogWriter *logWriter);
        void setBuffer(LogBuffer *logBuffer);
        LogBuffer* getBuffer();
        void processBuffer();
        void flushBuffer();

};

#endif