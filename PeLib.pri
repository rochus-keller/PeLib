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

CONFIG += c++11

HEADERS += \
    $$PWD/bigdigits.h \
    $$PWD/bigdtypes.h \
    $$PWD/DLLExportReader.h \
    $$PWD/PELib.h \
    $$PWD/MZHeader.h \
    $$PWD/PEFile.h \
    $$PWD/RSAEncoder.h \
    $$PWD/PEHeader.h \
    $$PWD/sha1.h \
    $$PWD/targetver.h \
    $$PWD/Qualifiers.h \
    $$PWD/PELibError.h \
    $$PWD/CodeContainer.h \
    $$PWD/DataContainer.h \
    $$PWD/CustomAttributeContainer.h \
    $$PWD/AssemblyDef.h \
    $$PWD/Namespace.h \
    $$PWD/Property.h \
    $$PWD/Class.h \
    $$PWD/Method.h \
    $$PWD/Field.h \
    $$PWD/Enum.h \
    $$PWD/Operand.h \
    $$PWD/Instruction.h \
    $$PWD/Value.h \
    $$PWD/MethodSignature.h \
    $$PWD/Type.h \
    $$PWD/Callback.h \
    $$PWD/Resource.h \
    $$PWD/PublicApi.h \
    $$PWD/SimpleApi.h

SOURCES += \
    $$PWD/AssemblyDef.cpp \
    $$PWD/bigdigits.cpp \
    $$PWD/Class.cpp \
    $$PWD/CodeContainer.cpp \
    $$PWD/CreateGUID.cpp \
    $$PWD/CustomAttributeContainer.cpp \
    $$PWD/DataContainer.cpp \
    $$PWD/DLLExportReader.cpp \
    $$PWD/Enum.cpp \
    $$PWD/Field.cpp \
    $$PWD/Instruction.cpp \
    $$PWD/Method.cpp \
    $$PWD/MethodSignature.cpp \
    $$PWD/Namespace.cpp \
    $$PWD/NetSignature.cpp \
    $$PWD/Operand.cpp \
    $$PWD/PECor20Headers.cpp \
    $$PWD/PELib.cpp \
    $$PWD/PELibError.cpp \
    $$PWD/PEWriter.cpp \
    $$PWD/Property.cpp \
    $$PWD/Qualifiers.cpp \
    $$PWD/RSAEncoder.cpp \
    $$PWD/sha1.cpp \
    $$PWD/Type.cpp \
    $$PWD/Value.cpp \
    $$PWD/Resource.cpp \
    $$PWD/SimpleApi.cpp
