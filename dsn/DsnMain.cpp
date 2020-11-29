#include "DsnInterface.h"

int main(int argc, char** argv)
{
	ProcMod m = PROC_MOD_IMG;
	DsnInterface di;
	di.Process(m);
	return 0;
}