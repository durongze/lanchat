#ifndef __DSN_MOD_PNG_H__
#define __DSN_MOD_PNG_H__

#include "DsnProcModule.h"

class DsnModPng :public DsnProcModule
{
public:
	DsnModPng();
	~DsnModPng();
	virtual int Init();
	virtual int Process();
	virtual int Deinit();
};

#endif