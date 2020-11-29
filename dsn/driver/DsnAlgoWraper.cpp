#include "DsnAlgoWraper.h"

DsnAlgoWraper::DsnAlgoWraper():m_mgr(nullptr)
{
	m_mgr = new DsnArgsMgr();
}

DsnAlgoWraper::~DsnAlgoWraper()
{

}

int DsnAlgoWraper::Init()
{
	m_mgr->Init();
	return 0;
}

int DsnAlgoWraper::Process()
{
	m_mgr->GetProcCfg();
	return 0;
}

int DsnAlgoWraper::Deinit()
{
	m_mgr->Deinit();
	return 0;
}