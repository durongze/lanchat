#ifndef TRACE_H
#define TRACE_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define MODE "ALL";
extern int g_itrace;

typedef enum tagEN_CTRACE
{
    EN_CTRACE_DISABLE,
    EN_CTRACE_ERROR,
    EN_CTRACE_DEBUG,
    EN_CTRACE_INFO,
    EN_CTRACE_BUFF
}EN_CTRACE;

void cTrace(const char* format,...);
//void print_reason(int sig, siginfo_t * info, void *secret);

#define TRACE(LEVEL,FORMAT,...) \
if(g_itrace & LEVEL) \
{ \
    cTrace("[%s-%d]" FORMAT,__FUNCTION__,__LINE__,##__VA_ARGS__); \
}

#endif // TRACE_H