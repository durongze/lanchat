#include "trace.h"
int g_itrace = 0xFFFFFFFF;
void cTrace(const char* format,...)
{
    char buf[1024] = {0};
    va_list va_param;
    va_start(va_param, format);
    vsprintf(buf,format,va_param);
    va_end(va_param);
    std::cout<<buf<<std::endl;
    return ;
}
