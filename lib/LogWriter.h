#ifndef __LOGWRITER_H__
#define __LOGWRITER_H__

class LogWriter {

    public:
        virtual int write (const char* datetime, const char* loglevel, const char* module, const char* text)=0; // returns zero if failed
};

#include <Print.h>

class ConsoleLogWriter : public LogWriter {
    protected:
        Print *_os;

    public:
        ConsoleLogWriter(Print *serial){ _os = serial;}
        ~ConsoleLogWriter(){_os->flush();}
        int write(const char* datetime, const char* loglevel, const char* module, const char* text){
            _os->printf("%s %s [%s] %s\n", datetime, loglevel, module, text);
            return 1;
        }
};

#endif