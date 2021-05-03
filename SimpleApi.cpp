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

#include "SimpleApi.h"
#include "PublicApi.h"
#include <QList>
#include <QtDebug>
using namespace DotNetPELib;

class SimpleApi::Imp : public PELib
{
public:
    QList<Resource*> d_idx;
    SimpleApi::ModuleKind d_kind;
    Method* d_meth;
    QList<Class*> d_classStack;
    QList<Operand*> d_labels;
    QList<Namespace*> d_nsStack;
    QHash<QByteArray,Type> d_basicTypes;
    bool d_hasCode;

    Imp( const QString& name ):PELib( name.toUtf8().constData() ),d_meth(0),d_hasCode(false)
    {
        d_basicTypes.insert("bool",Type(Type::Bool));
        d_basicTypes.insert("float32",Type(Type::r32));
        d_basicTypes.insert("float64",Type(Type::r64));
        d_basicTypes.insert("int",Type(Type::inative));
        d_basicTypes.insert("unsigned int",Type(Type::unative));
        d_basicTypes.insert("uint",Type(Type::unative));
        d_basicTypes.insert("int8",Type(Type::i8));
        d_basicTypes.insert("unsigned int8",Type(Type::u8));
        d_basicTypes.insert("uint8",Type(Type::u8));
        d_basicTypes.insert("int16",Type(Type::i16));
        d_basicTypes.insert("unsigned int16",Type(Type::u16));
        d_basicTypes.insert("uint16",Type(Type::u16));
        d_basicTypes.insert("int32",Type(Type::i32));
        d_basicTypes.insert("unsigned int32",Type(Type::u32));
        d_basicTypes.insert("uint32",Type(Type::u32));
        d_basicTypes.insert("int64",Type(Type::i64));
        d_basicTypes.insert("unsigned int64",Type(Type::u64));
        d_basicTypes.insert("uint64",Type(Type::u64));
        d_basicTypes.insert("object",Type(Type::object));
        d_basicTypes.insert("string",Type(Type::string));
        d_basicTypes.insert("char",Type(Type::Char));
    }

    void inline add(Instruction* i)
    {
        Q_ASSERT( d_meth != 0 );
        d_hasCode = true;
        d_meth->AddInstruction( i );
    }

    typedef QPair<Resource*,PELib::eFindType> Found;
    Found findName( const QByteArray& path )
    {
        const int rPar = path.lastIndexOf(')');
        if( rPar != -1 )
        {
            const int lPar = path.indexOf('(');
            if( lPar == -1 )
                throw PELibError(PELibError::Syntax, path.constData() );
            const QByteArrayList tmp = path.mid( lPar + 1, rPar - lPar - 1 ).split(',');
            std::vector<Type> args1;
            for( int i = 0; i < tmp.size(); i++ )
            {
                // TODO: array support
                QByteArray arg = tmp[i].simplified();
                if( arg.endsWith('&') )
                {
                    // Byref is not relevant for matching, just remove it
                    arg.chop(1);
                    arg = arg.trimmed();
                }
                QHash<QByteArray,Type>::const_iterator j = d_basicTypes.find(arg);
                if( j != d_basicTypes.end() )
                    args1.push_back( j.value() );
                else
                {
                    Resource* res;
                    const PELib::eFindType what = Find( arg.constData(), &res );
                    if( what == PELib::s_class || what == PELib::s_enum )
                        args1.push_back( Type(static_cast<Class*>(res)) );
                    else
                        throw PELibError(PELibError::NotSupported, path.constData() );
                }
            }
            std::vector<Type*> args2(args1.size());
            for( int i = 0; i < args1.size(); i++ )
                args2[i] = &args1[i];
            Method* res;
            const PELib::eFindType what = Find( path.left(lPar).constData(), &res, args2 );
            return qMakePair( (Resource*) res, what );
        }else
        {
            Resource* res;
            const PELib::eFindType what = Find( path.constData(), &res );
            return qMakePair( res, what );
        }
    }

    Resource* findName( const QByteArray& path, PELib::eFindType what )
    {
        Imp::Found res = findName(path);
        if( res.second == PELib::s_ambiguous )
            throw PELibError(PELibError::Ambiguous, path.constData());
        if( res.second != what )
            throw PELibError(PELibError::NotFound, path.constData());
        return res.first;
    }

    Type* findType( QByteArray path )
    {
        // TODO: array support, method pointers
        path = path.simplified();
        bool byref;
        if( path.endsWith('&') )
        {
            path.chop(1);
            path = path.trimmed();
            byref = true;
        }
        QHash<QByteArray,Type>::iterator j = d_basicTypes.find(path);
        if( j != d_basicTypes.end() )
        {
            if( !byref )
                return &j.value();
            else
            {
                Type* t = new Type( j.value().GetBasicType() );
                t->ByRef(true);
                return t;
            }
        }else
        {
            Resource* res;
            const PELib::eFindType what = Find( path.constData(), &res );
            if( what == PELib::s_class || what == PELib::s_enum )
            {
                Type* t = new Type(static_cast<Class*>(res));
                t->ByRef(byref);
                return t;
            }else
                throw PELibError(PELibError::NotSupported, path.constData() );
        }
        return 0;
    }

    void push( Class* cls )
    {
        if( !d_classStack.isEmpty() )
        {
            // this is a nested class
            d_classStack.back()->Add(cls);
            d_classStack.push_back(cls);
        }else if( !d_nsStack.isEmpty() )
        {
            // this is a top class in a namespace
            d_nsStack.back()->Add(cls);
            d_classStack.push_back(cls);
        }else
        {
            // this is a top class in the assembly root
            WorkingAssembly()->Add(cls);
            d_classStack.push_back(cls);
        }
    }
};

SimpleApi::SimpleApi():d_imp(0)
{

}

SimpleApi::~SimpleApi()
{
    if( d_imp )
        delete d_imp;
}

void SimpleApi::beginModule(const QByteArray& moduleName, SimpleApi::ModuleKind kind)
{
    Q_ASSERT( d_imp == 0 );
    d_imp = new Imp(moduleName);
    d_imp->d_kind = kind;
}

void SimpleApi::writeByteCode(const QByteArray& filePath)
{
    Q_ASSERT( d_imp != 0 );
    d_imp->DumpOutputFile(filePath.constData(), PELib::peexe, d_imp->d_kind == GuiApp );
}

void SimpleApi::writeAssembler(const QByteArray& filePath)
{
    Q_ASSERT( d_imp != 0 );
    d_imp->DumpOutputFile(filePath.constData(), PELib::ilasm, d_imp->d_kind == GuiApp );
}

void SimpleApi::endModule()
{
    Q_ASSERT( d_imp != 0 );
    delete d_imp;
    d_imp = 0;
}

void SimpleApi::addModuleReference(const QByteArray& moduleName)
{
    Q_ASSERT( d_imp != 0 );
    d_imp->LoadAssembly(moduleName.constData());
}

void SimpleApi::beginNamespace(const QByteArray& name)
{
    Q_ASSERT( d_imp != 0 );
    Namespace* ns = new Namespace(name.constData());
    if( d_imp->d_nsStack.isEmpty() )
        d_imp->WorkingAssembly()->Add(ns);
    else
        d_imp->d_nsStack.back()->Add(ns);
    d_imp->d_nsStack.push_back(ns);
}

void SimpleApi::endNamespace()
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( !d_imp->d_nsStack.isEmpty() );
    d_imp->d_nsStack.pop_back();
}

void SimpleApi::beginMethod(const QByteArray& methodName, bool isPublic, bool isStatic, bool isPrimary)
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( d_imp->d_meth == 0 );
    d_imp->d_labels.clear();
    MethodSignature* sig = new MethodSignature( methodName.constData(),
                                        MethodSignature::Managed, d_imp->WorkingAssembly() );
    Qualifiers q = Qualifiers::CIL |
            Qualifiers::Managed;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;
    if( isStatic )
        q |= Qualifiers::Static;
    if( methodName == ".ctor" || methodName == ".cctor" )
        q |= Qualifiers::SpecialName | Qualifiers::RTSpecialName;
    // TODO: Qualifiers::HideBySig
    d_imp->d_meth = new Method( sig, q, isPrimary);

    if( d_imp->d_classStack.isEmpty() )
        d_imp->WorkingAssembly()->Add(d_imp->d_meth); // namespaces only apply to classes!
    else
        d_imp->d_classStack.back()->Add(d_imp->d_meth);
}

void SimpleApi::endMethod()
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( d_imp->d_meth != 0 );

    if( !d_imp->d_meth->Signature()->ReturnType() )
        d_imp->d_meth->Signature()->ReturnType( new Type(Type::Void) );

    if( !d_imp->d_hasCode )
        qWarning() << "method without body:" << d_imp->d_meth->Signature()->Name().c_str();

    d_imp->d_meth->Optimize(*d_imp);

    d_imp->d_meth = 0;
    d_imp->d_labels.clear();
}

void SimpleApi::beginClass(const QByteArray& className, bool isPublic, const QByteArray& superClassQualifier)
{
    Qualifiers q;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;

    Class* cls = new Class(className.constData(), q, -1, -1 );

    if( !superClassQualifier.isEmpty() )
    {
        Resource* res = d_imp->findName(superClassQualifier.constData(), PELib::s_class);
        cls->Extends( (Class*)res );
    }

    d_imp->push(cls);
}

void SimpleApi::endClass()
{
    Q_ASSERT( d_imp && !d_imp->d_classStack.isEmpty() );
    d_imp->d_classStack.pop_back();
}

void SimpleApi::beginStruct(const QByteArray& structName, bool isPublic)
{
    Qualifiers q = Qualifiers::Value | Qualifiers::Sealed;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;

    Class* cls = new Class(structName.constData(), q, -1, -1 );
    d_imp->push(cls);
}

void SimpleApi::endStruct()
{
    endClass();
}

static inline void writeUint32( Byte* buf, quint32 val )
{
    buf[3] = (val >> 24) & 0xff;
    buf[2] = (val >> 16) & 0xff;
    buf[1] = (val >> 8) & 0xff;
    buf[0] = (val >> 0) & 0xff;
}

void SimpleApi::addEnum(const QByteArray& enumName, bool isPublic, const EnumItems& items, bool withRuntime)
{
    Qualifiers q = Qualifiers::Enum;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;

    Class* cls = new Class(enumName.constData(), q, -1, -1 );
    d_imp->push(cls);

    q = Qualifiers::SpecialName;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;

    Field* __value = new Field( "__value", new Type(Type::i32), q );
    cls->Add(__value);

    q = Qualifiers::Static;
    if( !withRuntime )
        q |= Qualifiers::Literal;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;

    int lastNr = -1;
    foreach( const EnumItem& item, items )
    {
        Field* f = new Field( item.first.constData(), new Type(Type::i32), q );
        if( item.second >= 0 )
        {
            if( item.second <= lastNr )
                throw PELibError(PELibError::NotSupported, enumName.constData());
            lastNr = item.second;
        }else
            lastNr++;
        if( withRuntime )
        {
            Byte* byte = d_imp->AllocateBytes(4);
            writeUint32( byte, lastNr );
            f->AddInitializer(byte,4);
        }else
            f->AddEnumValue( lastNr, Field::i32);
        cls->Add(f);
    }

    d_imp->d_classStack.pop_back();
}

void SimpleApi::addField(const QByteArray& fieldName, const QByteArray& typeQualifier, bool isPublic, bool isStatic)
{
    Q_ASSERT( d_imp != 0 );

    Qualifiers q;
    if( isPublic )
        q |= Qualifiers::Public;
    else
        q |= Qualifiers::Private;
    if( isStatic )
        q |= Qualifiers::Static;

    Field* f = new Field( fieldName.constData(), d_imp->findType(typeQualifier), q );

    if( d_imp->d_classStack.isEmpty() )
        d_imp->WorkingAssembly()->Add(f);
    else
        d_imp->d_classStack.back()->Add(f);
}

quint32 SimpleApi::addLocal(const QByteArray& typeQualifier, QByteArray name)
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( d_imp->d_meth );
    if( name.isEmpty() )
        name = "loc" + QByteArray::number(d_imp->d_meth->size());
    Local* l = new Local( name.constData(), d_imp->findType(typeQualifier) );
    d_imp->d_meth->AddLocal( l );
    return l->Index();
}

quint32 SimpleApi::addArgument(const QByteArray& typeQualifier, QByteArray name)
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( d_imp->d_meth );
    if( name.isEmpty() )
        name = "arg" + QByteArray::number( d_imp->d_meth->Signature()->ParamCount() );
    Param* p = new Param( name.constData(), d_imp->findType(typeQualifier) );
    d_imp->d_meth->Signature()->AddParam( p );
    return p->Index();
}

void SimpleApi::setReturnType(const QByteArray& typeQualifier)
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( d_imp->d_meth );
    d_imp->d_meth->Signature()->ReturnType(d_imp->findType(typeQualifier));
}

void SimpleApi::ADD(bool withOverflow, bool withUnsignedOverflow)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( withUnsignedOverflow )
        d_imp->add( new Instruction(Instruction::i_add_ovf_un ) );
    else if( withOverflow )
        d_imp->add( new Instruction(Instruction::i_add_ovf ) );
    else
        d_imp->add( new Instruction(Instruction::i_add ) );
}

void SimpleApi::AND()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_and) );
}

void SimpleApi::BEQ(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_beq, d_imp->d_labels[label] ) );
}

void SimpleApi::BGE(quint32 label, bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_bge_un, d_imp->d_labels[label] ) );
    else
        d_imp->add( new Instruction(Instruction::i_bge, d_imp->d_labels[label] ) );
}

void SimpleApi::BGT(quint32 label, bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_bgt_un, d_imp->d_labels[label] ) );
    else
        d_imp->add( new Instruction(Instruction::i_bgt, d_imp->d_labels[label] ) );
}

void SimpleApi::BLE(quint32 label, bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_ble_un, d_imp->d_labels[label] ) );
    else
        d_imp->add( new Instruction(Instruction::i_ble, d_imp->d_labels[label] ) );
}

void SimpleApi::BLT(quint32 label, bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_blt_un, d_imp->d_labels[label] ) );
    else
        d_imp->add( new Instruction(Instruction::i_blt, d_imp->d_labels[label] ) );
}

void SimpleApi::BNE(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_bne_un, d_imp->d_labels[label] ) );
}

void SimpleApi::BR(quint32 label)
{
    Q_ASSERT( d_imp != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_br, d_imp->d_labels[label] ) );
}

void SimpleApi::BREAK(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_break, d_imp->d_labels[label] ) );
}

void SimpleApi::BRFALSE(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_brfalse, d_imp->d_labels[label] ) );
}

void SimpleApi::BRNULL(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_brnull, d_imp->d_labels[label] ) );
}

void SimpleApi::BRZERO(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_brzero, d_imp->d_labels[label] ) );
}

void SimpleApi::BRTRUE(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_brtrue, d_imp->d_labels[label] ) );
}

void SimpleApi::BRINST(quint32 label)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Q_ASSERT( label < d_imp->d_labels.size() );
    d_imp->add( new Instruction(Instruction::i_brinst, d_imp->d_labels[label] ) );
}

void SimpleApi::CALL(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_method);
    Method* meth = static_cast<Method*>(res);
    d_imp->add( new Instruction(Instruction::i_call, new Operand( new MethodName( meth->Signature() ))));
}

void SimpleApi::CONV(SimpleApi::ToType t, bool withOverflow, bool withUnsignedOverflow)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Instruction::iop i = Instruction::i_unknown;

    switch( t )
    {
    case ToI1:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_i1_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_i1;
        else
            i = Instruction::i_conv_i1;
        break;
    case ToI2:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_i2_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_i2;
        else
            i = Instruction::i_conv_i2;
        break;
    case ToI4:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_i4_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_i4;
        else
            i = Instruction::i_conv_i4;
        break;
    case ToI8:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_i8_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_i8;
        else
            i = Instruction::i_conv_i8;
        break;
    case ToR4:
        i = Instruction::i_conv_r4;
        break;
    case ToR8:
        i = Instruction::i_conv_r8;
        break;
    case ToU1:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_u1_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_u1;
        else
            i = Instruction::i_conv_u1;
        break;
    case ToU2:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_u2_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_u2;
        else
            i = Instruction::i_conv_u2;
        break;
    case ToU4:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_u4_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_u4;
        else
            i = Instruction::i_conv_u4;
        break;
    case ToU8:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_u8_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_u8;
        else
            i = Instruction::i_conv_u8;
        break;
    case ToInt:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_i_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_i;
        else
            i = Instruction::i_conv_i;
        break;
    case ToUint:
        if( withUnsignedOverflow )
            i = Instruction::i_conv_ovf_u_un;
        else if( withOverflow )
            i = Instruction::i_conv_ovf_u;
        else
            i = Instruction::i_conv_u;
        break;
    case ToRun:
        i = Instruction::i_conv_r_un;
        break;
    }
    d_imp->add( new Instruction(i));
}

void SimpleApi::DIV(bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_div_un ) );
    else
        d_imp->add( new Instruction(Instruction::i_div ) );
}

void SimpleApi::DUP()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_dup ) );
}

void SimpleApi::LDARG(quint32 argNum)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( argNum >= d_imp->d_meth->Signature()->ParamCount() )
        throw PELibError(PELibError::IndexOutOfRange, "lddarg" );

    d_imp->add( new Instruction(Instruction::i_ldarg,
                                new Operand( d_imp->d_meth->Signature()->getParam(argNum))));
}

void SimpleApi::LDARGA(quint32 argNum)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( argNum >= d_imp->d_meth->Signature()->ParamCount() )
        throw PELibError(PELibError::IndexOutOfRange, "lddarga" );

    d_imp->add( new Instruction(Instruction::i_ldarga,
                                new Operand( d_imp->d_meth->Signature()->getParam(argNum))));
}

void SimpleApi::LDC(qint32 val)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldc_i4, new Operand( val, Operand::i32)));
}

void SimpleApi::LDC(qint64 val)
{
    Q_ASSERT( d_imp != 0 );
    d_imp->add( new Instruction(Instruction::i_ldc_i8, new Operand( val, Operand::i64)));
}

void SimpleApi::LDC(float val)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldc_r4, new Operand( val, Operand::r4)));
}

void SimpleApi::LDC(double val)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldc_r8, new Operand( val, Operand::r8)));
}

void SimpleApi::LDIND(SimpleApi::IndType t)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Instruction::iop i = Instruction::i_unknown;
    switch( t )
    {
    case I1:
        i = Instruction::i_ldind_i1;
        break;
    case I2:
        i = Instruction::i_ldind_i2;
        break;
    case I4:
        i = Instruction::i_ldind_i4;
        break;
    case I8:
        i = Instruction::i_ldind_i8;
        break;
    case U1:
        i = Instruction::i_ldind_u1;
        break;
    case U2:
        i = Instruction::i_ldind_u2;
        break;
    case U4:
        i = Instruction::i_ldind_u4;
        break;
    case U8:
        i = Instruction::i_ldind_u8;
        break;
    case R4:
        i = Instruction::i_ldind_r4;
        break;
    case R8:
        i = Instruction::i_ldind_r8;
        break;
    case Int:
        i = Instruction::i_ldind_i;
        break;
    case Ref:
        i = Instruction::i_ldind_ref;
        break;
    }
    d_imp->add( new Instruction(i) );
}

void SimpleApi::LDLOC(quint32 locNum)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( locNum >= d_imp->d_meth->size() )
        throw PELibError(PELibError::IndexOutOfRange, "ldloc" );
    d_imp->add( new Instruction(Instruction::i_ldloc,
                                new Operand( d_imp->d_meth->getLocal(locNum))));
}

void SimpleApi::LDLOCA(quint32 locNum)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( locNum >= d_imp->d_meth->size() )
        throw PELibError(PELibError::IndexOutOfRange, "ldloca" );
    d_imp->add( new Instruction(Instruction::i_ldloca,
                                new Operand( d_imp->d_meth->getLocal(locNum))));
}

void SimpleApi::LDNULL()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldnull) );
}

void SimpleApi::MUL(bool withOverflow, bool withUnsignedOverflow)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( withUnsignedOverflow )
        d_imp->add( new Instruction(Instruction::i_mul_ovf_un ) );
    else if( withOverflow )
        d_imp->add( new Instruction(Instruction::i_mul_ovf ) );
    else
        d_imp->add( new Instruction(Instruction::i_mul ) );
}

void SimpleApi::NEG()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_neg) );
}

void SimpleApi::NOP()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_nop) );
}

void SimpleApi::NOT()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_not) );
}

void SimpleApi::OR()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_or) );
}

void SimpleApi::POP()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_pop) );
}

void SimpleApi::REM(bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_rem_un ) );
    else
        d_imp->add( new Instruction(Instruction::i_rem ) );
}

void SimpleApi::RET()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ret) );
}

void SimpleApi::SHL()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_shl) );
}

void SimpleApi::SHR(bool withUnsigned)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( withUnsigned )
        d_imp->add( new Instruction(Instruction::i_shr_un ) );
    else
        d_imp->add( new Instruction(Instruction::i_shr ) );
}

void SimpleApi::STARG(quint32 argNum)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( argNum >= d_imp->d_meth->Signature()->ParamCount() )
        throw PELibError(PELibError::IndexOutOfRange, "stdarg" );

    d_imp->add( new Instruction(Instruction::i_starg,
                                new Operand( d_imp->d_meth->Signature()->getParam(argNum))));
}

void SimpleApi::STIND(SimpleApi::IndType t)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Instruction::iop i = Instruction::i_unknown;
    switch( t )
    {
    case I1:
        i = Instruction::i_stind_i1;
        break;
    case I2:
        i = Instruction::i_stind_i2;
        break;
    case I4:
        i = Instruction::i_stind_i4;
        break;
    case I8:
        i = Instruction::i_stind_i8;
        break;
    case R4:
        i = Instruction::i_stind_r4;
        break;
    case R8:
        i = Instruction::i_stind_r8;
        break;
    case Int:
        i = Instruction::i_stind_i;
        break;
    case Ref:
        i = Instruction::i_stind_ref;
        break;
    default:
        throw PELibError(PELibError::NotSupported);
    }
    d_imp->add( new Instruction(i) );
}

void SimpleApi::STLOC(quint32 locNum)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( locNum >= d_imp->d_meth->size() )
        throw PELibError(PELibError::IndexOutOfRange, "stloc" );
    d_imp->add( new Instruction(Instruction::i_stloc,
                                new Operand( d_imp->d_meth->getLocal(locNum))));
}

void SimpleApi::SUB(bool withOverflow, bool withUnsignedOverflow)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    if( withUnsignedOverflow )
        d_imp->add( new Instruction(Instruction::i_sub_ovf_un ) );
    else if( withOverflow )
        d_imp->add( new Instruction(Instruction::i_sub_ovf ) );
    else
        d_imp->add( new Instruction(Instruction::i_sub ) );
}

void SimpleApi::XOR()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_xor) );
}

void SimpleApi::CALLVIRT(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_method);
    Method* meth = static_cast<Method*>(res);
    d_imp->add( new Instruction(Instruction::i_callvirt, new Operand( new MethodName( meth->Signature() ))));
}

void SimpleApi::CASTCLASS(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_castclass, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::INITOBJ(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_initobj, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::ISINST(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_isinst, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::LDELEM(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldelem, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::LDELEM(SimpleApi::IndType t)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Instruction::iop i = Instruction::i_unknown;
    switch( t )
    {
    case I1:
        i = Instruction::i_ldelem_i1;
        break;
    case I2:
        i = Instruction::i_ldelem_i2;
        break;
    case I4:
        i = Instruction::i_ldelem_i4;
        break;
    case I8:
        i = Instruction::i_ldelem_i8;
        break;
    case U1:
        i = Instruction::i_ldelem_u1;
        break;
    case U2:
        i = Instruction::i_ldelem_u2;
        break;
    case U4:
        i = Instruction::i_ldelem_u4;
        break;
    case U8:
        i = Instruction::i_ldelem_u8;
        break;
    case R4:
        i = Instruction::i_ldelem_r4;
        break;
    case R8:
        i = Instruction::i_ldelem_r8;
        break;
    case Int:
        i = Instruction::i_ldelem_i;
        break;
    case Ref:
        i = Instruction::i_ldelem_ref;
        break;
    }
    d_imp->add( new Instruction(i) );
}

void SimpleApi::LDELEMA(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldelema, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::LDFLD(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_field);
    d_imp->add( new Instruction(Instruction::i_ldfld,
                                new Operand( new FieldName(static_cast<Field*>(res)))));
}

void SimpleApi::LDFLDA(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_field);
    d_imp->add( new Instruction(Instruction::i_ldflda,
                                new Operand( new FieldName(static_cast<Field*>(res)))));
}

void SimpleApi::LDLEN()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldlen) );
}

void SimpleApi::LDOBJ(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldobj, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::LDSFLD(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_field);
    d_imp->add( new Instruction(Instruction::i_ldsfld,
                                new Operand( new FieldName(static_cast<Field*>(res)))));
}

void SimpleApi::LDSFLDA(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_field);
    d_imp->add( new Instruction(Instruction::i_ldsflda,
                                new Operand( new FieldName(static_cast<Field*>(res)))));
}

void SimpleApi::LDSTR(const QByteArray& string)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_ldstr, new Operand( string.constData(), true ) ) );
}

void SimpleApi::LDVIRTFTN(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Method* meth = static_cast<Method*>(d_imp->findName(qualifier, PELib::s_method));
    d_imp->add( new Instruction(Instruction::i_ldvirtftn, new Operand( new MethodName( meth->Signature() ))));
}

void SimpleApi::NEWARR(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_newarr, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::NEWOBJ(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Method* meth = static_cast<Method*>(d_imp->findName(qualifier, PELib::s_method));
    if( meth->Signature()->Name() != ".ctor" )
        throw PELibError( PELibError::NotSupported, qualifier.constData() );
    d_imp->add( new Instruction(Instruction::i_newobj, new Operand( new MethodName( meth->Signature() ))));
}

void SimpleApi::STELEM(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    d_imp->add( new Instruction(Instruction::i_stelem, new Operand( new Value(d_imp->findType(qualifier)))));
}

void SimpleApi::STELEM(SimpleApi::IndType t)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Instruction::iop i = Instruction::i_unknown;
    switch( t )
    {
    case I1:
        i = Instruction::i_stelem_i1;
        break;
    case I2:
        i = Instruction::i_stelem_i2;
        break;
    case I4:
        i = Instruction::i_stelem_i4;
        break;
    case I8:
        i = Instruction::i_stelem_i8;
        break;
    case R4:
        i = Instruction::i_stelem_r4;
        break;
    case R8:
        i = Instruction::i_stelem_r8;
        break;
    case Int:
        i = Instruction::i_stelem_i;
        break;
    case Ref:
        i = Instruction::i_stelem_ref;
        break;
    default:
        throw PELibError(PELibError::NotSupported);
    }
    d_imp->add( new Instruction(i) );
}

void SimpleApi::STFLD(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_field);
    d_imp->add( new Instruction(Instruction::i_stfld,
                                new Operand( new FieldName(static_cast<Field*>(res)))));
}

void SimpleApi::STSFLD(const QByteArray& qualifier)
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    Resource* res = d_imp->findName(qualifier, PELib::s_field);
    d_imp->add( new Instruction(Instruction::i_stsfld,
                                new Operand( new FieldName(static_cast<Field*>(res)))));
}

quint32 SimpleApi::newLabel()
{
    Q_ASSERT( d_imp != 0 && d_imp->d_meth != 0 );
    const int id = d_imp->d_labels.size();
    Operand* label = new Operand( QByteArray::number(id).constData() ); // seems only to work with string labels
    d_imp->d_labels.append( label );
    d_imp->add( new Instruction(Instruction::i_label, label) );
    return id;
}

