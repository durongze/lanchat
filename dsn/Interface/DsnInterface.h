#ifndef __DSN_INTERFACE_H__
#define __DSN_INTERFACE_H__

#include "DsnAdapter.h"

class DsnInterface
{
public:
	DsnInterface();
	~DsnInterface();
	int Init();
	int Process(ProcMod mod);
	int Deinit();
private:
	DsnAdapter* m_adapter;
};

#endif