#ifndef __DYZLOG_H__
#define __DYZLOG_H__

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{ 
    DYZLOGCRIT=0,
    DYZLOGERROR,
    DYZLOGWARNING,
    DYZLOGINFO,
    DYZLOGDEBUG, 
    DYZLOGTRACE, 
    DYZLOGALL
} DYZLogLevel;


typedef void (DYZLogCallback)(int level, const char *fmt, va_list);
void DYZLogSetCallback(DYZLogCallback *cb);
void DYZLogSetOutput(FILE *file);

void DYZLogPrintf(const char *format, ...);
void DYZLogStatus(const char *format, ...);
void DYZLog(int level, const char *format, ...);

void DYZLogHex(int level, const uint8_t *data, unsigned long len);
void DYZLogHexString(int level, const uint8_t *data, unsigned long len);
void DYZLogSetLevel(DYZLogLevel lvl);
DYZLogLevel DYZLogGetLevel(void);

#define ModLog(l, m, fmt, ...) DYZLog(l, "%s[%s:%d]" fmt, m, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif
