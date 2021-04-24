/*
 *     Copyright(C) 2021 by me@rochus-keller.ch
 *
 *     This file is part of the PELib package.
 *
 *     The file is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 2 of the License, or
 *     (at your option) any later version.
 */

#include "Resource.h"
using namespace DotNetPELib;

#if 0
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

#endif



std::set<Resource*> Resource::s_all;


Resource::~Resource()
{
    s_all.erase(this);
}
