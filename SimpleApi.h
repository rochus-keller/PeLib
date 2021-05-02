#ifndef DotNetPELib_SIMPLEAPI_H
#define DotNetPELib_SIMPLEAPI_H

/*
 *     Copyright(C) 2021 by me@rochus-keller.ch
 *
 *     This file is part of the PeLib package.
 *
 *     The file is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 2 of the License, or
 *     (at your option) any later version.
 */

#include <QByteArray>
#include <QPair>

namespace DotNetPELib
{
    class SimpleApi
    {
    public:
        SimpleApi();
        ~SimpleApi();

        enum ModuleKind { Library, // no .subsystem
                            GuiApp, // .subsystem = 2
                            ConsoleApp, // .subsystem = 3
                          };

        // we only support one Module per Assembly here; the Assembly manifest is implicitly generated
        // assembly versioning, hasing, keys and cultures are not used
        // no exported type defs, no Type forwarders, no interfaces
        // all strings are utf-8

        // qualifier: composite name,
        // optionally with [assembly], namespace, etc.
        // optionally with argument types (t1,t2,..), where tn are qualifiers or native type names
        // both valid: Namespace.Class and Namespace::Class
        // identifier start with alpha, #, $, @ or _ and continue with alphanumeric, ?, $, @, _ or `


        void beginModule( const QByteArray& moduleName, ModuleKind = Library );
        void writeByteCode( const QByteArray& filePath ); // dll or exe doesn't actually matter
        void writeAssembler( const QByteArray& filePath );
        void endModule();

        void addModuleReference( const QByteArray& moduleName );

        void beginNamespace( const QByteArray& name );
        void endNamespace(); // namespaces can be nested, only apply to classes, structs and enums

        void beginMethod(const QByteArray& methodName, // can be on top level or in a class/struct; cannot be in a method
                         bool isPublic,
                         bool isStatic,
                         bool isPrimary );
        void endMethod();

        void beginClass(const QByteArray& className, bool isPublic,
                         const QByteArray& superClassQualifier = QByteArray() );
        void endClass(); // classes can be nested

        void beginStruct( const QByteArray& structName, bool isPublic );
        void endStruct(); // structs can be nested

        typedef QPair<QByteArray,qint32> EnumItem; // -1 undefined
        typedef QList<EnumItem> EnumItems;
        void addEnum( const QByteArray& enumName, bool isPublic, const EnumItems&, bool withRuntime = false );

        void addField( const QByteArray& fieldName, // on top level or in class
                       const QByteArray& typeQualifier,
                       bool isPublic,
                       bool isStatic );

        void addLocal( const QByteArray& varName, const QByteArray& typeQualifier );
        void addParam( const QByteArray& varName, const QByteArray& typeQualifier );

        quint32 newLabel();

        // Base instructions:
        void ADD( bool withOverflow = false, bool withUnsignedOverflow = false );
        void AND();
        void BEQ( quint32 label );
        void BGE( quint32 label, bool withUnsigned = false );
        void BGT( quint32 label, bool withUnsigned = false );
        void BLE( quint32 label, bool withUnsigned = false );
        void BLT( quint32 label, bool withUnsigned = false );
        void BNE( quint32 label );
        void BR( quint32 label );
        void BREAK( quint32 label );
        void BRFALSE( quint32 label );
        void BRNULL( quint32 label );
        void BRZERO( quint32 label );
        void BRTRUE( quint32 label );
        void BRINST( quint32 label );
        void CALL( const QByteArray& qualifier );
        // CALLI
        // CEQ, CGT, CLT
        // Ckfinite
        enum ToType { ToI1, ToI2, ToI4, ToI8, ToR4, ToR8, ToU1, ToU2, ToU4, ToU8, ToInt, ToUint, ToRun };
        void CONV( ToType, bool withOverflow = false, bool withUnsignedOverflow = false );
        // Cpblk
        void DIV(bool withUnsigned = false);
        void DUP();
        // endfilter, endfinally
        // initblk
        // JMP
        void LDARG(quint32 argNum);
        void LDARGA(quint32 argNum);
        void LDC( qint32 );
        void LDC( qint64 );
        void LDC( float );
        void LDC( double );
        // void LDFTN( const QByteArray& qualifier );
        enum IndType { I1, I2, I4, I8, U1, U2, U4, U8, R4, R8, Int, Ref };
        void LDIND( IndType );
        void LDLOC( quint32 locNum );
        void LDLOCA( quint32 locNum );
        void LDNULL();
        // leave
        // localloc
        void MUL(bool withOverflow = false, bool withUnsignedOverflow = false);
        void NEG();
        void NOP();
        void NOT();
        void OR();
        void POP();
        void REM( bool withUnsigned = false );
        void RET();
        void SHL();
        void SHR(bool withUnsigned = false );
        void STARG( quint32 argNum );
        void STIND( IndType ); // only subset I, R, Int, Ref supported!
        void STLOC( quint32 locNum );
        void SUB( bool withOverflow = false, bool withUnsignedOverflow = false );
        // switch
        void XOR();

        // Object model instructions:
        // box
        void CALLVIRT(const QByteArray& qualifier);
        void CASTCLASS(const QByteArray& qualifier);
        // copobj
        void INITOBJ(const QByteArray& qualifier);
        void ISINST(const QByteArray& qualifier);
        void LDELEM(const QByteArray& qualifier);
        void LDELEM( IndType );
        void LDELEMA(const QByteArray& qualifier);
        void LDFLD(const QByteArray& qualifier);
        void LDFLDA(const QByteArray& qualifier);
        void LDLEN();
        void LDOBJ(const QByteArray& qualifier);
        void LDSFLD(const QByteArray& qualifier);
        void LDSFLDA(const QByteArray& qualifier);
        void LDSTR( const QByteArray& string );
        // ldtoken
        void LDVIRTFTN(const QByteArray& qualifier);
        // mkrefany
        void NEWARR(const QByteArray& qualifier);
        void NEWOBJ(const QByteArray& qualifier);
        // refanytype, refanyval
        // rethrow
        // sizeof
        void STELEM(const QByteArray& qualifier);
        void STELEM( IndType ); // only subset I, R, Int, Ref supported
        void STFLD(const QByteArray& qualifier);
        // stobj
        void STSFLD(const QByteArray& qualifier);
        // throw
        // unbox

    private:
        class Imp;
        Imp* d_imp;
    };
}

#endif // DotNetPELib_SIMPLEAPI_H
