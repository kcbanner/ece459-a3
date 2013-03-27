#-------------------------------------------------
#
# Project created by QtCreator 2011-12-12T11:00:12
#
#-------------------------------------------------

QT       += core
CONFIG   += console

TARGET = test_harness
#TEMPLATE = app

QMAKE_CXXFLAGS_RELEASE += -g
LIBS += -lprofiler

SOURCES += test_harness.cpp \
    model.cpp

HEADERS  +=  model.h
