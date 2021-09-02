#/*
# *     Copyright(C) 2021 by me@rochus-keller.ch
# *
# *     The file is free software: you can redistribute it and/or modify
# *     it under the terms of the GNU General Public License as published by
# *     the Free Software Foundation, either version 2 of the License, or
# *     (at your option) any later version.
# *
# */

QT       += core
QT       -= gui

TARGET = PeLib
TEMPLATE = app

CONFIG += c++11

CONFIG(debug, debug|release) {
        DEFINES += _DEBUG
}

include( PeLib.pri )

SOURCES += test2.cpp








