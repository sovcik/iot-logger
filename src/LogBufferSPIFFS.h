#ifndef __LOGBUFFERSPIFFS_H__
#define __LOGBUFFERSPIFFS_H__

#include <FS.h>
#include "LogBuffer.h"

#define BUFFER_MAX_RECORDS  250

#define BUFFER_READ_MODE_SEEK   0
#define BUFFER_READ_MODE_NEXT   1

class LogBufferSPIFFS : public LogBuffer {
    protected:
        File _logFile;   // file containing log entries, each fixed length
        int _circular;
        int _readMode;
        int8_t _noNewRecords;
        unsigned int _size;
        const char *_logFName;
        uint32_t _wfilepos;  // pointer for writing to file
        uint32_t _rfilepos;  // pointer for reading from file

    
        int readRecord(LogRecord *rec);
        uint32_t seekNext(char what, int fromStart = 0);

    public:
        LogBufferSPIFFS(const char* logFName, unsigned int size = BUFFER_MAX_RECORDS, int circular = 1);
        ~LogBufferSPIFFS();
        void begin(int clear = 0) override;
        void stop() override;
        int isReady() override;
        int isEmpty() { return _noNewRecords;} override;
        
        unsigned int size() override;
        int write(LogRecord *rec) override;
        int read(LogRecord *);
        
};

#endif