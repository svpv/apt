#include <sys/types.h>
#include <rpm/rpmtypes.h>
#include <rpm/rpmds.h>

rpmds dsSingleGlobal(rpmTagVal tagN, const char * N, const char * EVR,
	rpmsenseFlags Flags);

int dsRpmlibGlobal(rpmds * dsp, const void * tblp);
