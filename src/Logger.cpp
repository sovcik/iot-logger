#include "Logger.h"
#include <TimeLib.h>

// uncomment/comment following line to stop/start debug print 
#define NODEBUG_PRINT
#include <debug_print.h>

const char *LogLevelStrings[] = {"FATAL","ERROR","WARN", "INFO", "DEBUG", "TRACE"};

Logger::Logger() {
    activeLevel = ERROR;
    logWriter = NULL;
}

void Logger::setLevel(LogLevel level) {
    activeLevel = level;
}

void Logger::setLogWriter(LogWriter *logWriter) {
    this->logWriter = logWriter;
}

void Logger::setBuffer(LogBuffer *logBuffer) {
    this->logBuffer = logBuffer;
}


void Logger::log(LogLevel level, const char* module, const char* text, ...) {
    LogRecord rec;
    bool useBuffer = true;
    
    if (level > activeLevel) return;

    va_list argptr;
    va_start(argptr, text);
    vsnprintf(&rec.text[0], BUFFER_RECORD_TEXT_SIZE, text, argptr);
    va_end(argptr);

    sprintf(&rec.datetime[0],"%d-%02d-%02dT%02d:%02d:%02dZ",year(), month(), day(), hour(), minute(), second());

    DEBUG_PRINT("[logger:log] going to log date=%s, level=%s, module=%s, text=%s\n", rec.datetime, LogLevelStrings[level], module, &txt[0]);

    if (logWriter) 
        useBuffer = logWriter->writeLogEntry(&rec.datetime[0], LogLevelStrings[level], module, &rec.text[0]) == 0;

    if (useBuffer && logBuffer) { 
        DEBUG_PRINT("[logger:log] writer failed -> going to use buffer\n");
        strncpy(&rec.level[0], LogLevelStrings[level], BUFFER_RECORD_LEVEL_SIZE);
        rec.level[BUFFER_RECORD_LEVEL_SIZE-1]=0;
        strncpy(&rec.module[0], module,BUFFER_RECORD_MODULE_SIZE);
        rec.module[BUFFER_RECORD_MODULE_SIZE-1]=0;
        //DEBUG_PRINT("[logger:log] date=%s, module=%s, text=%s\n",rec.datetime, rec.module, rec.text);
        logBuffer->write(&rec);
    }

}

void Logger::processBuffer(){
    if (!logBuffer || logBuffer->isEmpty() || !logWriter) 
        return;

    LogRecord rec;
    if (logBuffer->read(&rec)){  // if there is new record
        DEBUG_PRINT("[logger:proLog] processing buffered log entry. date=%s, text=%s\n",&rec.datetime[0], &rec.text[0]);
        int r = logWriter->writeLogEntry( &rec.datetime[0], &rec.level[0], &rec.module[0], &rec.text[0]);
        if (!r) // TODO: if write failed - put it back to log?
            DEBUG_PRINT("[logger:proLog] failed writing log entry - log entry lost\n");
    }

}