#include "LogBufferFB.h"

#if DEBUG_LOGGER != 1
#define NODEBUG_PRINT
#endif
#include <debug_print.h>

LogBufferFB::LogBufferFB(const char* logFName, uint16_t capacity){
    this->logFName = strdup(logFName);
    fb = new FileBuffer<LogRecord>(capacity);
}

LogBufferFB::~LogBufferFB(){
    stop();
    free(this->logFName);
    delete fb;
}

int LogBufferFB::begin(int clear){

    DEBUG_PRINT("[lbFB:begin] opening buffer files clear=%d, exists=%d\n",clear,SPIFFS.exists(this->logFName) );
    if (!fb->open(logFName, clear, true)){
        DEBUG_PRINT("[lbFB:begin] failed opening buffer\n");
        return 0;
    }

    return 1;
}

void LogBufferFB::stop(){
    fb->close();
}



int LogBufferFB::write(LogRecord *rec){
    if (!fb->isReady()) {
        DEBUG_PRINT("[lbFB:write] ERROR buffer not ready\n");
        return 0;
    }

    DEBUG_PRINT("[lbFB:write] writing to log buffer date=%s\n",rec->datetime);
    fb->push(*rec);

    return 1;
}


int LogBufferFB::read(LogRecord *rec){
    if (!fb->isReady() || fb->isEmpty()) return 0;
    
    DEBUG_PRINT("[lbFB:read] reading log buffer ");
    *rec = fb->pop();
    DEBUG_PRINT("date=%s\n",rec->datetime);

    return 1;

}