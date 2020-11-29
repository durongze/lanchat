#include "DsnProcModule.h"
DsnProcModule::DsnProcModule(std::string modName):m_algo(nullptr),m_modName(modName)
{
}

DsnProcModule::~DsnProcModule()
{
}

int DsnProcModule::Init()
{
	m_algo->Init();
	return 0;
}

int DsnProcModule::Process()
{
	m_algo->Process();
	return 0;
}

int DsnProcModule::Deinit()
{
	m_algo->Deinit();
	return 0;
}