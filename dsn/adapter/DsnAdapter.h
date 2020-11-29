#ifndef __DSN_ADAPTER_H__
#define __DSN_ADAPTER_H__

#include "DsnModContext.h"

class DsnAdapter
{
public:
	DsnAdapter();
	~DsnAdapter();
	int Init();
	int Process(ProcMod mod);
	int Deinit();
private:
	DsnModContext* m_modCtx;
};

#endif