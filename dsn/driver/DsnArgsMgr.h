#ifndef __DSN_ARGS_MGR_H__
#define __DSN_ARGS_MGR_H__

class DsnArgsMgr
{
public:
	DsnArgsMgr();
	~DsnArgsMgr();
	int Init();
	int GetProcCfg();
	int Process();
	int Deinit();
private:
};

#endif