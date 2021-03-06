#include "DsnAdapter.h"
#include "DsnModFactory.h"

DsnAdapter::DsnAdapter():m_modCtx(NULL)
{
	m_modCtx = new DsnModContext();
}

DsnAdapter::~DsnAdapter()
{
	if (m_modCtx != NULL) {
		delete m_modCtx;
		m_modCtx = NULL;
	}
}

int DsnAdapter::Init()
{
	std::vector<ProcMod> mods;
	int m;
	for (m = PROC_MOD_IMG; PROC_MOD_PNG >= m; ++m) {
		mods.push_back((ProcMod)m);
	}
	m_modCtx->Init(mods);
	return 0;
}

int DsnAdapter::Process(ProcMod mod)
{
	m_modCtx->Process(mod);
	return 0;
}

int DsnAdapter::Deinit()
{
	m_modCtx->Deinit();
	return 0;
}