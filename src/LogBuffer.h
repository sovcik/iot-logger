#ifndef __LOGBUFFER_H__
#define __LOGBUFFER_H__

#define BUFFER_RECORD_DATE_SIZE     21
#define BUFFER_RECORD_LEVEL_SIZE    6
#define BUFFER_RECORD_MODULE_SIZE   20
#define BUFFER_RECORD_TEXT_SIZE     100
//#define BUFFER_RECORD_SIZE  (BUFFER_RECORD_DATE_SIZE+BUFFER_RECORD_LEVEL_SIZE+BUFFER_RECORD_MODULE_SIZE+BUFFER_RECORD_TEXT_SIZE+10)

struct LogRecord {
    char mark;
    char datetime[BUFFER_RECORD_DATE_SIZE];
    char level[BUFFER_RECORD_LEVEL_SIZE];
    char module[BUFFER_RECORD_MODULE_SIZE];
    char text[BUFFER_RECORD_TEXT_SIZE];
};

class LogBuffer {

    public:
        virtual void begin(int clear = 0);
        virtual void stop();
        
        virtual unsigned int size()=0;
        virtual int read(LogRecord *rec)=0; // should return zero if read failed
        virtual int write(LogRecord *rec)=0; // should return zero if read failed
        
        virtual int isReady()=0;
        virtual int isEmpty()=0;
};

#endif