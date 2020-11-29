#include "DsnModPng.h"

#define MOD_NAME "img"

DsnModPng::DsnModPng():DsnProcModule(MOD_NAME)
{

}

DsnModPng::~DsnModPng()
{
}

int DsnModPng::Init()
{
	DsnProcModule::Init();
	return 0;
}

int DsnModPng::Process()
{
	DsnProcModule::Process();
	return 0;
}

int DsnModPng::Deinit()
{
	DsnProcModule::Deinit();
	return 0;
}