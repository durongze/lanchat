#include "DsnModFactory.h"

DsnModFactory::DsnModFactory()
{

}
DsnModFactory::~DsnModFactory()
{

}

DsnProcModule *DsnModFactory::CreateModule(ProcMod modId)
{
	std::string modName;
	DsnProcModule* mod = nullptr;

	switch (modId)
	{
	case PROC_MOD_IMG:
		mod = new DsnProcModule(modName);
		break;
	case PROC_MOD_PNG:
		mod = new DsnProcModule(modName);
		break;
	default:
		break;
	}
	return mod;
}

void DsnModFactory::DestoryModule(DsnProcModule* mod)
{
	delete mod;
}