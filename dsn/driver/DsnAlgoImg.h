#ifndef __DSN_DRIVER_H__
#define __DSN_DRIVER_H__

#include "DsnAlgoWraper.h"

class DsnAlgoImg:public DsnAlgoWraper
{
public:
	DsnAlgoImg();
	~DsnAlgoImg();
	int Init();
	int Process();
	int Deinit();
private:
};

#endif