QT += testlib
QT += gui
QT += core
QT += widgets
QT += sql
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += \
    Component/PerformersManager.cpp \
    Component/ProductionStudioManager.cpp \
    FileOperation/FileOperation.cpp \
    PublicTool.cpp \
    TestCase/CatergorizerTest.cpp \
    TestCase/FilesNameBatchStandardizerTest.cpp \
    TestCase/MovieNameStandardizerTest.cpp \
    TestCase/PerformersManagerTest.cpp \
    TestCase/ProductionStudioManagerTest.cpp \
    TestCase/QStringFunctionTest.cpp \
    TestCase/RenameWidgetTest.cpp \
    TestCase/FileOperationTest.cpp \
    TestCase/UndoRedoTest.cpp \
    Tools/Categorizer.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/NameStandardizer.cpp \
    UndoRedo.cpp


HEADERS += \
    Component/PerformersManager.h \
    Component/ProductionStudioManager.h \
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
