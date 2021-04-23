
QT       += core
QT       -= gui

TARGET = PeLib
TEMPLATE = app

CONFIG += c++11 # use of auto

CONFIG(debug, debug|release) {
        DEFINES += _DEBUG
}

INCLUDEPATH += ..

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
    targetver.h \
    Qualifiers.h \
    PELibError.h \
    CodeContainer.h \
    DataContainer.h \
    CustomAttributeContainer.h \
    AssemblyDef.h \
    Namespace.h \
    Property.h \
    Class.h \
    Method.h \
    Field.h \
    Enum.h \
    Allocator.h \
    Operand.h \
    Instruction.h \
    Value.h \
    MethodSignature.h \
    Type.h \
    Callback.h \
    Resource.h

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
    Namespace.cpp \
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
    test.cpp \
    Resource.cpp

DISTFILES +=







