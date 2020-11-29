#include "DsnModImg.h"

#define MOD_NAME "png"

DsnModImg::DsnModImg():DsnProcModule(MOD_NAME)
{

}

DsnModImg::~DsnModImg()
{
}

int DsnModImg::Init()
{
	DsnProcModule::Init();
	return 0;
}

int DsnModImg::Process()
{
	DsnProcModule::Process();
	return 0;
}
int DsnModImg::Deinit()
{
	DsnProcModule::Deinit();
	return 0;
}