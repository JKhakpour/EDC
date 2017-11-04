QT += core
QT -= gui
QT += concurrent

TARGET = edc
CONFIG += console
CONFIG -= app_bundle
CONFIG +=c++11

TEMPLATE = app

SOURCES += main.cpp \
    leven.cpp

HEADERS += \
    leven.h

