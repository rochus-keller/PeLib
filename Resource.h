#ifndef DOTNETPELIB_RESOURCE_H
#define DOTNETPELIB_RESOURCE_H

#include <set>

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
        Resource() { s_all.insert(this); }
    protected:
        virtual ~Resource();
    private:
        friend class PELib;
        static std::set<Resource*> s_all; // TODO replace with RefCounted
    };
}

#endif // DOTNETPELIB_RESOURCE_H
