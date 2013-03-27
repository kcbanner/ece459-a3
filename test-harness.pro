#-------------------------------------------------
#
# Project created by QtCreator 2011-12-12T11:00:12
#
#-------------------------------------------------

QT       += core
CONFIG   += console

TARGET = test_harness
#TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3

QMAKE_CXXFLAGS_RELEASE -= -mtune=generic
QMAKE_CXXFLAGS_RELEASE += -mtune=corei7-avx

QMAKE_CXXFLAGS_RELEASE -= -march=x86-64
QMAKE_CXXFLAGS_RELEASE += -march=native

QMAKE_CXXFLAGS_RELEASE += -ffast-math
#QMAKE_CXXFLAGS_RELEASE += -ftree-vectorizer-verbose=5

QMAKE_CXXFLAGS += -pthread
QMAKE_CXXFLAGS += -ggdb
QMAKE_CXXFLAGS += -mssse3
QMAKE_CXXFLAGS += -malign-double

LIBS += -lprofiler

SOURCES += test_harness.cpp \
    model.cpp

HEADERS  +=  model.h
