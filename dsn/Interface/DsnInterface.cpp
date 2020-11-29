#include "DsnInterface.h"

DsnInterface::DsnInterface():m_adapter(NULL)
{
	m_adapter = new DsnAdapter();
}

DsnInterface::~DsnInterface()
{
	if (m_adapter != NULL) {
		delete m_adapter;
		m_adapter = NULL;
	}
}

int DsnInterface::Init()
{
	m_adapter->Init();
	return 0;
}

int DsnInterface::Process(ProcMod mod)
{
	m_adapter->Process(mod);
	return 0;
}

int DsnInterface::Deinit()
{
	m_adapter->Deinit();
	return 0;
}
