#ifndef DotNetPELib_CALLBACK
#define DotNetPELib_CALLBACK

namespace DotNetPELib
{
    class AssemblyDef;
    class Namespace;
    class Class;
    class Enum;
    class Method;
    class Field;
    class Property;

    ///** The callback structure is passed to 'traverse'... it holds callbacks
    // called while traversing the tree
    class Callback
    {
    public:
        virtual ~Callback() { }

        virtual bool EnterAssembly(const AssemblyDef *) { return true; }
        virtual bool ExitAssembly(const AssemblyDef *) { return true; }
        virtual bool EnterNamespace(const Namespace *) { return true; }
        virtual bool ExitNamespace(const Namespace *) { return true; }
        virtual bool EnterClass(const Class *) { return true; }
        virtual bool ExitClass(const Class *) { return true; }
        virtual bool EnterEnum(const Enum *) { return true; }
        virtual bool ExitEnum(const Enum *) { return true; }
        virtual bool EnterMethod(const Method *) { return true; }
        virtual bool EnterField(const Field *) { return true; }
        virtual bool EnterProperty(const Property *) { return true; }
    };
}

#endif // DotNetPELib_CALLBACK

