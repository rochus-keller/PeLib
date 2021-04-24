#ifndef DotNetPELib_PUBLICAPI
#define DotNetPELib_PUBLICAPI

#include <PeLib/Callback.h>
#include <PeLib/Class.h>
#include <PeLib/CodeContainer.h>
#include <PeLib/CustomAttributeContainer.h>
#include <PeLib/Enum.h>
#include <PeLib/Callback.h>
#include <PeLib/Field.h>
#include <PeLib/AssemblyDef.h>
#include <PeLib/DataContainer.h>
#include <PeLib/Instruction.h>
#include <PeLib/Method.h>
#include <PeLib/MethodSignature.h>
#include <PeLib/Namespace.h>
#include <PeLib/Operand.h>
#include <PeLib/PELibError.h>
#include <PeLib/Property.h>
#include <PeLib/Qualifiers.h>
#include <PeLib/Type.h>
#include <PeLib/Value.h>
#include <PeLib/PELib.h>

// NOTE that all subclasses of Resource are automatically deleted when the PELib instance
// is deleted; there can only be one instance of PELib at a time.

#endif // DotNetPELib_PUBLICAPI

