#ifndef __DSN_MOD_IMG_H__
#define __DSN_MOD_IMG_H__

#include "DsnProcModule.h"

class DsnModImg :public DsnProcModule
{
public:
	DsnModImg();
	~DsnModImg();
	virtual int Init();
	virtual int Process();
	virtual int Deinit();
};

#endif