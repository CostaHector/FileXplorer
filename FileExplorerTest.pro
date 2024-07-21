QT += testlib
QT += gui
QT += core
QT += widgets
QT += sql
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    TestCase/ConflictSolveModelTest.cpp \
    TestCase/ConflictsItemHelperTest.cpp \
    TestCase/ConvertUnicodeCharsetToAsciiTest.cpp \
    TestCase/NameSectionArrangeTest.cpp \
    TestCase/NameToolTest.cpp \
    TestCase/PathToolTest.cpp \
    TestCase/RenameUnqiueCheckTest.cpp \
    TestCase/SyncModifiyFileSystemTest.cpp \
    TestCase/ToConsecutiveFileNameNoTest.cpp \
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/SyncModifiyFileSystem.cpp \
    TestCase/ValueCheckerTest.cpp \
    TestCase/FinderTest.cpp \
    Tools/ProductionStudioManager.cpp \
    Tools/PathTool.cpp\
    Tools/NameTool.cpp\
    Tools/ConflictsItemHelper.cpp \
    Tools/ConvertUnicodeCharsetToAscii.cpp\
    Component/QuickWhereClause.cpp \
    Component/StateLabel.cpp \
    Component/NotificatorFrame.cpp \
    FileOperation/FileOperation.cpp \
    FileOperation/FileOperatorPub.cpp \
    PublicTool.cpp \
    PublicVariable.cpp \
    TestCase/CatergorizerTest.cpp \
    TestCase/DuplicateImagesRemoverTest.cpp \
    TestCase/FilesNameBatchStandardizerTest.cpp \
    TestCase/MovieNameStandardizerTest.cpp \
    TestCase/PerformersStringParserTest.cpp \
    TestCase/ProductionStudioManagerTest.cpp \
    TestCase/QStringFunctionTest.cpp \
    TestCase/QuickWhereClauseTest.cpp \
    TestCase/FileOperationTest.cpp \
    TestCase/UndoRedoTest.cpp \
    Tools/Categorizer.cpp \
    Tools/PerformersAkaManager.cpp \
    Tools/DuplicateImagesRemover.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/NameStandardizer.cpp \
    Tools/PerformersManager.cpp \
    Tools/RenameNamesUnique.cpp \
    Tools/ToConsecutiveFileNameNo.cpp \
    Tools/NameSectionArrange.cpp \
    model/ConflictsFileSystemModel.cpp \
    UndoRedo.cpp


HEADERS += \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Component/RenameWidgets/AdvanceRenamer.h \
    TestCase/pub/FileSystemRelatedTest.h \
    Tools/ProductionStudioManager.h \
    Tools/PathTool.h\
    Tools/NameTool.h\
    Tools/ConflictsItemHelper.h \
    Component/QuickWhereClause.h \
    Component/StateLabel.h \
    Component/NotificatorFrame.h \
    Component/SyncModifiyFileSystem.h \
    FileOperation/FileOperation.h \
    FileOperation/FileOperatorPub.h \
    PublicTool.h \
    PublicVariable.h \
    Tools/Categorizer.h \
    Tools/PerformersAkaManager.h \
    Tools/DuplicateImagesRemover.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/NameStandardizer.h \
    Tools/PerformersManager.h \
    Tools/RenameNamesUnique.h \
    Tools/ToConsecutiveFileNameNo.h \
    Tools/ConvertUnicodeCharsetToAscii.h\
    Tools/NameSectionArrange.h \
    model/ConflictsFileSystemModel.h \
    UndoRedo.h

DISTFILES += \
    readme.md

RESOURCES += \
    themes/res.qrc
