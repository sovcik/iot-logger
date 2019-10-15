#include "Logger.h"
#include <Arduino.h>
#include <TimeLib.h>

#define NODEBUG_PRINT
#include <debug_print.h>

const char* Logger::LogLevelStrings[] = {"fatal","error","warn","info","debug","trace"};

Logger::Logger() {
    activeLevel = ERROR;
    logWriter = NULL;
    logBuffer = NULL;
}

Logger::~Logger(){
    
}

void Logger::setLevel(LogLevel level) {
    activeLevel = level;
}

void Logger::setLevel(const char *level) {
    int i = 0;
    while (i<6 && strcmp(level,Logger::LogLevelStrings[i]) != 0)
        i++;
    if (i<6)
        setLevel((LogLevel)i);
}


void Logger::setLogWriter(LogWriter *logWriter) {
    this->logWriter = logWriter;
}

void Logger::setBuffer(LogBuffer *logBuffer) {
    this->logBuffer = logBuffer;
}

LogBuffer* Logger::getBuffer(){
    return logBuffer;
}

void Logger::getISOTime(char *buff, size_t size) {
    // return time zone offset in seconds - both positive and negative - depending on time zone
    // override this method depending on your implementation of time zone handling
    snprintf(buff, size, "%d-%02d-%02dT%02d:%02d:%02dZ", year(), month(), day(), hour(), minute(), second());
}

void Logger::log(LogLevel level, const char* module, const char* text, ...){   
    if (level > activeLevel) return;

    LogRecord *rec = new LogRecord;

    va_list argptr;
    va_start(argptr, text);
    vsnprintf(rec->text, BUFFER_RECORD_TEXT_SIZE, text, argptr);
    va_end(argptr);

    getISOTime(rec->datetime, BUFFER_RECORD_DATE_SIZE);

    DEBUG_PRINT("[logger:logP] going to log date=%s, level=%s, module=%s, text=%s\n", rec->datetime, LogLevelStrings[level], module, rec->text);

    strncpy(rec->level, LogLevelStrings[level], BUFFER_RECORD_LEVEL_SIZE);
    rec->level[BUFFER_RECORD_LEVEL_SIZE-1]=0;
    strncpy_P(rec->module, module, BUFFER_RECORD_MODULE_SIZE);
    rec->module[BUFFER_RECORD_MODULE_SIZE-1]=0;

    // in order to make sure log records will be written (e.g. sent to server) in the right order
    // write directly only if buffer is already empty
    bool useBuffer = true;
    if (!logBuffer || logBuffer->isEmpty()){
        if (logWriter) {
            useBuffer = logWriter->writeLogEntry(rec->datetime, rec->level, rec->module, rec->text) == 0;
            if (useBuffer) 
                DEBUG_PRINT("[logger:log] log writing failed\n");
        } else {
            DEBUG_PRINT("[logger:log] writer not configured.\n");
        }
    }    

    if (useBuffer && logBuffer) { 
        DEBUG_PRINT("[logger:logP] going to use buffer\n");

        //DEBUG_PRINT("[logger:log] date=%s, module=%s, text=%s\n",rec.datetime, rec.module, rec.text);
        logBuffer->write(rec);
    }

    delete rec;
}

void Logger::processBuffer(){
    if (!logBuffer || logBuffer->isEmpty() || !logWriter) 
        return;

    LogRecord *rec = new LogRecord;
    if (logBuffer->read(rec)){  // if there is new record
        DEBUG_PRINT("[logger:proLog] processing buffered log entry. date=%s, text=%s\n",rec->datetime, rec->text);
        int r = logWriter->writeLogEntry( rec->datetime, rec->level, rec->module, rec->text);
        if (!r) // TODO: if write failed - put it back to log?
            DEBUG_PRINT("[logger:proLog] failed writing log entry - log entry lost\n");
    }
    delete rec;

}

void Logger::flushBuffer(){
    if (!logBuffer) return;
    logBuffer->flush();
}