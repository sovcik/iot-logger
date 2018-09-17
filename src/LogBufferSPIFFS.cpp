#include "LogBufferSPIFFS.h"

// uncomment/comment following line to stop/start debug print 
#define NODEBUG_PRINT
#include <debug_print.h>

#define MARK_NEW 'N'
#define MARK_OLD 'O'

LogBufferSPIFFS::LogBufferSPIFFS(const char* logFName, unsigned int size, int circular){
    _rfilepos = 0;
    _wfilepos = 0;
    _size = size;
    _readMode = BUFFER_READ_MODE_SEEK;
    _circular = circular;
    _logFName = logFName;
}

LogBufferSPIFFS::~LogBufferSPIFFS(){
    stop();
}

void LogBufferSPIFFS::begin(int clear){
    const char *mode;

    DEBUG_PRINT("[lbSPF:begin] opening buffer files\n");
    if (clear || !SPIFFS.exists(_logFName))
        mode="w+";
    else 
        mode="r+";

    _logFile = SPIFFS.open(_logFName, mode);
    if (!_logFile) {
        DEBUG_PRINT("[lbSPF:begin] failed opening buffer\n");
        return;
    }

    DEBUG_PRINT("[lbSPF:begin] buffer file available\n");

    //DEBUG_PRINT("[lbSPF:begin] rpos=%lu wpos=%lu\n", _rfilepos, _wfilepos);
    // set reading position to the first "new" record or EOF, start search from begin
    _rfilepos = seekNext(MARK_NEW, 1);

    if (_logFile.available()) {
        // set writing position to the first "old" record or EOF
        _wfilepos = seekNext(MARK_OLD, 0);
    } else {
        _wfilepos = 0;
        _noNewRecords = 1;
    }


    //DEBUG_PRINT("[lbSPF:begin] adjusted rpos=%lu wpos=%lu\n", _rfilepos, _wfilepos);
}

void LogBufferSPIFFS::stop(){
    _logFile.close();
}


unsigned int LogBufferSPIFFS::size() {
    return _size;
}

int LogBufferSPIFFS::isReady(){
    return _logFile?1:0;
}

uint32_t LogBufferSPIFFS::seekNext(char what, int fromStart){

    DEBUG_PRINT("[lbSPF:seekNext] what=%c pos=%lu fromStart=%d\n", what, _logFile.position(), fromStart);
    uint32_t pos = _logFile.position();
    
    LogRecord buffer;
    
    if (fromStart) {
        _logFile.seek(0, SeekSet);
        pos = 0;
    }

    while (buffer.mark != what && _logFile.available())
        _logFile.read((uint8_t*)&buffer,sizeof(buffer));

    // if new record found, start reading from there
    if (buffer.mark == what) {
        pos = _logFile.position()-sizeof(buffer);
        DEBUG_PRINT("[lbSPF:seekNext] found=%c at pos=%lu\n", what, pos);
        _logFile.seek(pos, SeekSet);
        
    } else {
        pos = _logFile.position();
    }

    DEBUG_PRINT("[lbSPF:seekNext] new position=%lu\n", pos);

    return pos;
}

int LogBufferSPIFFS::write(LogRecord *rec){
    int16_t ret = 0;

    LogRecord r2;

    //DEBUG_PRINT("[lbSPF:write] date=%s, module=%s, text=%s\n",rec->datetime, rec->module, rec->text);

    if (!_logFile) {
        DEBUG_PRINT("[lbSPF:write] logfile not available\n");
        return 0;
    }

    //DEBUG_PRINT("[lbSPF:write] wpos1=%lu, size=%d, record=%d\n",_wfilepos, _size, _wfilepos / sizeof(*rec));

    if (_circular && ((_wfilepos / sizeof(*rec)) >= _size))
        _wfilepos = 0;

    _logFile.seek(_wfilepos, SeekSet);
    DEBUG_PRINT("[lbSPF:write] going to write. pos=%lu\n",_wfilepos);

    rec->mark = MARK_NEW;  

    //DEBUG_PRINT("[lbSPF:write] pos=%lu buff=",_logFile.position());
    //DEBUG_ARRAYF((uint8_t*)rec,sizeof(*rec),"%c.");
    //DEBUG_PRINT("\n");

    ret = _logFile.write((uint8*)rec, sizeof(*rec));

    _logFile.flush();

    _wfilepos = _logFile.position();
    //DEBUG_PRINT("[lbSPF:write] wpos3=%lu\n",_wfilepos);

    _noNewRecords = 0; // there is at least this new record

    return ret;
}


int LogBufferSPIFFS::read(LogRecord *rec){
    //DEBUG_PRINT("[lbSPF:read] rpos1=%lu\n",_rfilepos);
    
    _logFile.seek(_rfilepos, SeekSet);
    if (!_noNewRecords && _circular && !_logFile.available() && _logFile.size() > 0){
        _rfilepos = seekNext(MARK_NEW, 1);
        if (!_logFile.available()){ // reached end trying to find next NEW record
            _noNewRecords = 1;
            DEBUG_PRINT("[lbSPF:read] seek found no new records in the whole file\n");
        }
    }

    if (_noNewRecords){
        DEBUG_PRINT("[lbSPF:read] flag 'no new records' is SET\n");
        return 0;
    }

    //DEBUG_PRINT("[lbSPF:read] rpos2=%lu size=%d\n",_logFile.position(),sizeof(*rec));
    uint r = _logFile.read((uint8_t*)rec,sizeof(*rec));

    //DEBUG_PRINT("[lbSPF:read] read=%lu buff=",r);
    //DEBUG_ARRAYF((uint8_t*)rec,sizeof(*rec),"%c.");
    //DEBUG_PRINT("\n");


    //DEBUG_PRINT("[lbSPF:read] mark=%c, dt=%s, level=%s, mod=%s, txt=%s\n", rec->mark, rec->datetime, rec->level, rec->module, rec->text);
    if (rec->mark == MARK_NEW) {
        uint32_t op = _rfilepos;
        _rfilepos = _logFile.position();
        _logFile.seek(op, SeekSet);
        _logFile.write(MARK_OLD);
        return 1;
    } else {
        DEBUG_PRINT("[lbSPF:read] no new recod found\n");
    }

    return 0;

}