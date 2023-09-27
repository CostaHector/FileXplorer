QT += testlib
QT += gui
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app


SOURCES += TestCase/SomeTestTemplate.cpp \
    FileOperation/FileOperation.cpp \
    TestCase/FileOperationTest.cpp

HEADERS += \
    FileOperation/FileOperation.h
