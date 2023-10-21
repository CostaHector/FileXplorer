QT += testlib
QT += gui
QT += core
QT += widgets
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += \
    FileOperation/FileOperation.cpp \
    PublicTool.cpp \
    TestCase/CatergorizerTest.cpp \
    TestCase/FilesNameBatchStandardizerTest.cpp \
    TestCase/MovieNameStandardizerTest.cpp \
    TestCase/QStringFunctionTest.cpp \
    TestCase/RenameWidgetTest.cpp \
    TestCase/FileOperationTest.cpp \
    TestCase/UndoRedoTest.cpp \
    Tools/Categorizer.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/NameStandardizer.cpp \
    UndoRedo.cpp


HEADERS += \
    FileOperation/FileOperation.h \
    PublicTool.h \
    Tools/Categorizer.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/NameStandardizer.h \
    UndoRedo.h

DISTFILES += \
    readme.md

RESOURCES += \
    themes/res.qrc
