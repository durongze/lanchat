#ifndef __DSN_DRIVER_H__
#define __DSN_DRIVER_H__

#include "DsnAlgoWraper.h"

class DsnAlgoPng:public DsnAlgoWraper
{
public:
	DsnAlgoPng();
	~DsnAlgoPng();
	int Init();
	int Process();
	int Deinit();
private:
};

#endif