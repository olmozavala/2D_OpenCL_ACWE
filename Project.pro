#OPENCL="/opt/AMDAPP/"
OPENCL="/usr/local/cuda"
OZLIB="OZlib/"
#This is used because I was getting the following erro (April 2014)
# error running a compiled C++ file (uses OpenGL). Error: “Inconsistency detected by ld.so: dl-version.c: 224”

LIBS += -L$${OZLIB} # Adds lib folder (for ozlib)

LIBS += -lGL -lGLU -lglut -lGLEW -lX11 -lm -lFileManager -lOpenCL 
LIBS +=  -lGLManager -lCLManager -lImageManager -lGordonTimers -lfreeimage

#INCLUDEPATH += $${OZLIB}/.."/khronos"

INCLUDEPATH += $${OZLIB}
INCLUDEPATH += $${OPENCL}"/include"
INCLUDEPATH += "../2DSignedDistFunc/src/headers"
INCLUDEPATH += "/usr/include/GL" # For glew.h
INCLUDEPATH += "./src/headers/"  # All headers
INCLUDEPATH += "./src/forms/headers/"  # All headers

HEADERS += src/headers/*.h
HEADERS += src/forms/headers/*.h
HEADERS += ../2DSignedDistFunc/src/headers/SignedDistFunc.h

SOURCES += src/*.cpp
SOURCES += src/forms/src/*.cpp
SOURCES += ../2DSignedDistFunc/src/SignedDistFunc.cpp

FORMS += src/forms/ui/*.ui
MOD_DIR = build/moc
UI_SOURCES_DIR = src/forms/src
UI_HEADERS_DIR = src/forms/headers

TARGET = RunActiveContoursQt
OBJECTS_DIR = build
DESTDIR = dist
MOC_DIR = build/moc

#DEFINES += DEBUG

CONFIG += qt 
#CONFIG += qt debug

QT +=core gui opengl
QMAKE_CXXFLAGS += -w -std=gnu++11 -O3
