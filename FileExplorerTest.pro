QT += testlib
QT += gui
QT += core
QT += widgets
QT += sql
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += \
    Actions/QuickWhereActions.cpp \
    Component/PerformersManager.cpp \
    Component/ProductionStudioManager.cpp \
    Component/QuickWhereClause.cpp \
    FileOperation/FileOperation.cpp \
    PublicTool.cpp \
    PublicVariable.cpp \
    TestCase/CatergorizerTest.cpp \
    TestCase/FilesNameBatchStandardizerTest.cpp \
    TestCase/MovieNameStandardizerTest.cpp \
    TestCase/PerformersManagerTest.cpp \
    TestCase/ProductionStudioManagerTest.cpp \
    TestCase/QStringFunctionTest.cpp \
    TestCase/QuickWhereClauseTest.cpp \
    TestCase/RenameWidgetTest.cpp \
    TestCase/FileOperationTest.cpp \
    TestCase/UndoRedoTest.cpp \
    Tools/Categorizer.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/NameStandardizer.cpp \
    UndoRedo.cpp


HEADERS += \
    Actions/QuickWhereActions.h \
    Component/PerformersManager.h \
    Component/ProductionStudioManager.h \
    Component/QuickWhereClause.h \
    FileOperation/FileOperation.h \
    PublicTool.h \
    PublicVariable.h \
    Tools/Categorizer.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/NameStandardizer.h \
    UndoRedo.h

DISTFILES += \
    readme.md

RESOURCES += \
    themes/res.qrc
