#ifndef DOTNETPELIB_RESOURCE_H
#define DOTNETPELIB_RESOURCE_H

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

#include <deque>
#include <stddef.h>

namespace DotNetPELib
{
#if 0
    class RefCounted
    {
    public:
        RefCounted();
        virtual ~RefCounted();

        void addRef();
        void release();

#ifdef _DEBUG
        static QSet<RefCounted*> s_inst;
#endif
    private:
        quint32 d_refCount;
    };

    template<class T>
    class Ref
    {
    public:
        Ref( T* t = 0 ):d_obj()
        {
            assign(t);
        }
        Ref( const Ref<T>& rhs ):d_obj()
        {
            *this = rhs;
        }
        ~Ref() { release(); }

        void release()
        {
            if( d_obj )
                d_obj->release();
            d_obj = 0;
        }

        void assign(T* obj)
        {
            if( obj == d_obj )
                return;
            release();
            d_obj = obj;
            if( obj )
                d_obj->addRef();
        }

        operator T*() const { return d_obj; }

        T* operator->() const { return d_obj; }

        Ref<T>& operator=( T* obj )
        {
            assign(obj);
            return *this;
        }

        Ref<T>& operator=( const Ref<T>& rhs )
        {
            assign(rhs.d_obj);
            return *this;
        }

        bool isNull() const { return d_obj == 0; }

    private:
        T* d_obj;
    };
#endif

    class Resource
    {
    public:
        Resource() {}
        virtual ~Resource() {}

        void* operator new( size_t size)
        {
            // avoid stack allocated objects from s_all
            void* ptr = ::operator new(size);
            s_all.push_back((Resource*)ptr);
            return ptr;
        }
        void operator delete(void* ptr)
        {
            // usually only PELib calls delete on these objects
            for( int i = 0; i < s_all.size(); i++ )
            {
                if( s_all[i] == (Resource*)ptr )
                {
                    s_all[i] = 0;
                    break;
                }
            }
            ::operator delete(ptr);
        }

    private:
        friend class PELib;
        static std::deque<Resource*> s_all; // TODO replace with RefCounted
    };
}

#endif // DOTNETPELIB_RESOURCE_H
