//
// Created by zgy on 2025/12/3.
//
#include "logger.h"
#include "TraceLogger.h"
#include "sds.h"
logger *_logger = nullptr;

void initLogger(size_t function_address)
{
    _logger = new logger();
    _logger->buf = sdsempty();
    _logger->logfile = getLogPath(LogType::QBDI_TRACE,(void*)function_address);
    _logger->lastwrite = 0;
    _logger->totallen = 0;
}

void deleteLogger()
{
    if(_logger != nullptr)
    {
        sdsfree(_logger->buf);
    }
    delete _logger;
    _logger = nullptr;
}

void appendlog(const char* str)
{
      if(_logger != nullptr)
      {
          _logger->buf = sdscat(_logger->buf, str);
      }
}

void appendlogendl()
{
    appendlog("\n");
}

void appendformat(const char* format,...)
{
    va_list ap;
    va_start(ap, format);
    _logger->buf = sdscatvprintf(_logger->buf,format,ap);
    va_end(ap);
}

void writelog()
{
    _logger->totallen = _logger->lastwrite + sdslen(_logger->buf);
    LOGE("write log:%lx,%lx,%s", _logger->lastwrite,_logger->totallen,_logger->logfile.c_str());
    std::ofstream out(_logger->logfile.c_str(), std::ios::app);
    if (!out.is_open()) {
        LOGE("Failed to create trace log file: %s", _logger->logfile.c_str());
        return ;
    }
    out << _logger->buf;
    out.close();
    _logger->lastwrite = _logger->totallen;
    sdsfree(_logger->buf);
    _logger->buf = sdsempty();
    LOGE("write log done!");
}