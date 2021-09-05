#ifndef SIGNATUREGENERATOR_H
#define SIGNATUREGENERATOR_H

#include <stddef.h>

namespace DotNetPELib {

typedef unsigned char Byte; /* 1 byte */

class MethodSignature;
class Property;
class Field;
class Method;
class Type;
class PELib;

// This class holds functions for generating the various signatures we need
// to put in the blob stream
class SignatureGenerator
{
    // this implementation isn't completely thread safe - it uses the equivalent the
    // equivalent of a global variable to keep track of state
public:
    static Byte *MethodDefSig(MethodSignature *signature, size_t &sz);
    static Byte *MethodRefSig(MethodSignature *signature, size_t &sz);
    static Byte *MethodSpecSig(MethodSignature *signature, size_t &sz);
    static Byte *PropertySig(Property *property, size_t &sz);
    static Byte *FieldSig(Field *field, size_t &sz);
    //Byte *PropertySig(Property *property);
    static Byte *LocalVarSig(Method *method, size_t &sz);
    static Byte *TypeSig(Type *type, size_t &sz);

    // end of signature generators, this function is a generic function to embed a type
    // inito a signature
    static size_t EmbedType(int *buf, int offset, Type *tp);
    // this function converts a signature buffer to a blob entry, by compressing
    // the integer values in the signature
    static Byte *ConvertToBlob(int *buf, int size, size_t &sz);

    // this function sets the index for the 'object' class entry
    static void SetObjectType(size_t ObjectBase) { objectBase = ObjectBase; }

private:
    // a shared function for the various signatures that put in method signatures
    static size_t CoreMethod(MethodSignature *method, int paramCount, int *buf, int offset);
    static size_t LoadIndex(Byte *buf, size_t &start, size_t &len);
    static Type *BasicType(PELib& lib, int typeIndex, int pointerLevel);
    static int workArea[400 * 1024];
    static int basicTypes[];
    static int objectBase;
};
}

#endif // SIGNATUREGENERATOR_H

