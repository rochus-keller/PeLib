#include "Resource.h"
using namespace DotNetPELib;

#ifdef _DEBUG
QSet<RefCounted*> RefCounted::s_inst;
#endif

RefCounted::RefCounted():d_refCount(0)
{
#ifdef _DEBUG
    s_inst.insert(this);
#endif
}

RefCounted::~RefCounted()
{
#ifdef _DEBUG
    s_inst.remove(this);
#endif
}

void RefCounted::addRef()
{
    d_refCount++;
}

void RefCounted::release()
{
    if( d_refCount == 1 )
    {
        d_refCount = 0;
        delete this;
    }
}



