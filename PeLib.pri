#/*
# *     Copyright(C) 2021 by me@rochus-keller.ch
# *
# *     The file is free software: you can redistribute it and/or modify
# *     it under the terms of the GNU General Public License as published by
# *     the Free Software Foundation, either version 2 of the License, or
# *     (at your option) any later version.
# *
# */

INCLUDEPATH += ..

HEADERS += \
    bigdigits.h \
    bigdtypes.h \
    DLLExportReader.h \
    PELib.h \
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
    Operand.h \
    Instruction.h \
    Value.h \
    MethodSignature.h \
    Type.h \
    Callback.h \
    Resource.h \
    $$PWD/PublicApi.h

SOURCES += \
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
    Resource.cpp
