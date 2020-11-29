#include "DsnModContext.h"

DsnModContext::DsnModContext():m_mapMod()
{

}
DsnModContext::~DsnModContext()
{

}
int DsnModContext::Init(std::vector<ProcMod>& mods)
{
	DsnProcModule* procMod;
	std::vector<ProcMod>::iterator modIter;
	for (modIter = mods.begin(); mods.end() != modIter; modIter++) {
		procMod = DsnModFactory::CreateModule(*modIter);
		m_mapMod[*modIter] = procMod;
	}
	return 0;
}
int DsnModContext::Process(ProcMod modId)
{
	m_mapMod[modId]->Process();
	return 0;
}
int DsnModContext::Deinit()
{
	std::map<ProcMod, DsnProcModule*>::iterator modIter;
	for (modIter = m_mapMod.begin(); m_mapMod.end() != modIter; modIter++) {
		if (modIter->second != NULL) {
			delete modIter->second;
			modIter->second = NULL;
		}
	}
	return 0;
}