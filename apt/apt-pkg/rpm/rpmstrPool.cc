#include <apt-pkg/rpmstrPool.h>
#include <rpm/rpmstrpool.h>

static rpmstrPool global_rpmstrPool;

rpmds dsSingleGlobal(rpmTagVal tagN, const char * N, const char * EVR,
	rpmsenseFlags Flags)
{
    if (!global_rpmstrPool)
	global_rpmstrPool = rpmstrPoolCreate();

    return rpmdsSinglePool(global_rpmstrPool, tagN, N, EVR, Flags);
}

int dsRpmlibGlobal(rpmds * dsp, const void * tblp)
{
    if (!global_rpmstrPool)
	global_rpmstrPool = rpmstrPoolCreate();

    return rpmdsRpmlibPool(global_rpmstrPool, dsp, tblp);
}
