#ifndef __DSN_MOD_FACTORY_H__
#define __DSN_MOD_FACTORY_H__

#include "DsnProcModule.h"

enum ProcMod
{
	PROC_MOD_IMG,
	PROC_MOD_PNG,
};

class DsnModFactory
{
private:
	DsnModFactory();
	~DsnModFactory();
public:
	static DsnProcModule* CreateModule(ProcMod modId);
	static void DestoryModule(DsnProcModule* mod);
};

#endif
