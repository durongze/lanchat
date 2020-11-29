#ifndef __DSN_MOD_CONTEXT_H__
#define __DSN_MOD_CONTEXT_H__

#include "DsnProcModule.h"
#include "DsnModFactory.h"
#include <map>
#include <vector>

class DsnModContext
{
public:
	DsnModContext();
	~DsnModContext();
	int Init(std::vector<ProcMod>& mods);
	int Process(ProcMod modId);
	int Deinit();
private:
	std::map<ProcMod, DsnProcModule*> m_mapMod;
};

#endif