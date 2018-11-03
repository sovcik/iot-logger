#include "LogBufferSPIFFS.h"

#if DEBUG_LOGGER != 1
#define NODEBUG_PRINT
#endif
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

int LogBufferSPIFFS::begin(int clear){
    const char *mode;
    uint8_t b;

    DEBUG_PRINT("[lbSPF:begin] opening buffer files clear=%d, exists=%d\n",clear,SPIFFS.exists(_logFName) );
    if (clear || !SPIFFS.exists(_logFName)) {
        mode="w+";
    } else { 
        mode="r+";
    }

    _logFile = SPIFFS.open(_logFName, mode);
    if (!_logFile) {
        DEBUG_PRINT("[lbSPF:begin] failed opening buffer\n");
        return 0;
    }

    DEBUG_PRINT("[lbSPF:begin] buffer file available\n");

    // do test read in order to detect corrupted file
    if (_logFile.available() && _logFile.read(&b,1) != 1){
        DEBUG_PRINT("[lbSPF:begin] ERROR - buffer file corrupted\n");
        return 0;
    }

    // set reading position to the first "new" record or EOF, start search from begin
    _rfilepos = seekNext(MARK_NEW, 1);

    if (_logFile.available()) {
        // set writing position to the first "old" record or EOF
        _wfilepos = seekNext(MARK_OLD, 0);
    } else {
        _wfilepos = 0;
        _noNewRecords = 1;
    }

    DEBUG_PRINT("[lbSPF:begin] adjusted rpos=%lu wpos=%lu\n", _rfilepos, _wfilepos);

    return 1;
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
    buffer.mark = '~'; // force reading current position
    
    if (fromStart) {
        _logFile.seek(0, SeekSet);
        pos = 0;
    }

    size_t r = sizeof(buffer);
    size_t rr = r;

    while (buffer.mark != what && r == rr && _logFile.available()) {
        r = _logFile.read((uint8_t*)&buffer, rr);
    }

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

    if (!_logFile) {
        DEBUG_PRINT("[lbSPF:write] logfile not available\n");
        return 0;
    }

    if (_circular && ((_wfilepos / sizeof(*rec)) >= _size))
        _wfilepos = 0;

    _logFile.seek(_wfilepos, SeekSet);
    DEBUG_PRINT("[lbSPF:write] going to write. pos=%lu\n",_wfilepos);

    rec->mark = MARK_NEW;  

    ret = _logFile.write((uint8*)rec, sizeof(*rec));

    _logFile.flush();

    _wfilepos = _logFile.position();

    _noNewRecords = 0; // there is at least this new record

    return ret;
}


int LogBufferSPIFFS::read(LogRecord *rec){
    DEBUG_PRINT("[lbSPF:read] rpos1=%lu\n",_rfilepos);
    size_t recSize = sizeof(*rec);
    
    _logFile.seek(_rfilepos, SeekSet);
    if (!_noNewRecords && _circular && _logFile.available() >= recSize && _logFile.size() > 0){
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
    size_t r = _logFile.read((uint8_t*)rec, recSize);

    if (r != recSize){
        DEBUG_PRINT("[lbSPF:read] read only %lu bytes - buffer most likely corrupted\n",r);
        return 0;
    }

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