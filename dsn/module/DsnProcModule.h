#ifndef __DSN_PROC_MODULE_H__
#define __DSN_PROC_MODULE_H__

#include "DsnAlgoWraper.h"
#include <string>

class DsnProcModule
{
public:
	DsnProcModule(std::string modName);
	~DsnProcModule();
	virtual int Init();
	virtual int Process();
	virtual int Deinit();
private:
	DsnAlgoWraper* m_algo;
	std::string m_modName;
};

#endif