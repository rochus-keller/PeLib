
QT       += core
QT       -= gui

TARGET = PeLib
TEMPLATE = app

CONFIG += c++11 # use of auto

HEADERS += \
    bigdigits.h \
    bigdtypes.h \
    DLLExportReader.h \
    DotNetPELib.h \
    MZHeader.h \
    PEFile.h \
    RSAEncoder.h \
    PEHeader.h \
    sha1.h \
    targetver.h

SOURCES += \
    Allocator.cpp \
    AssemblyDef.cpp \
    bigdigits.cpp \
    Class.cpp \
    CodeContainer.cpp \
    CreateGUID.cpp \
    CustomAttributeContainer.cpp \
    DataContainer.cpp \
    DLLExportReader.cpp \
    Enum.cpp \
    Field.cpp \
    Instruction.cpp \
    Method.cpp \
    MethodSignature.cpp \
    NameSpace.cpp \
    NetSignature.cpp \
    Operand.cpp \
    PECor20Headers.cpp \
    PELib.cpp \
    PELibError.cpp \
    PEReader.cpp \
    PEWriter.cpp \
    Property.cpp \
    Qualifiers.cpp \
    RSAEncoder.cpp \
    sha1.cpp \
    Type.cpp \
    Value.cpp \
    test.cpp

DISTFILES +=







