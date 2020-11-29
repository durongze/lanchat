#ifndef __DSN_ALGO_WRAPER_H__
#define __DSN_ALGO_WRAPER_H__

#include "DsnArgsMgr.h"

class DsnAlgoWraper
{
public:
	DsnAlgoWraper();
	~DsnAlgoWraper();
	int Init();
	int Process();
	int Deinit();
private:
	DsnArgsMgr* m_mgr;
};

#endif
