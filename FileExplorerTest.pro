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
    TestCase/AIMediaDuplicateTest.cpp \
    TestCase/ConflictSolveModelTest.cpp \
    TestCase/ConflictsItemHelperTest.cpp \
    TestCase/ConvertUnicodeCharsetToAsciiTest.cpp \
    TestCase/ExtractPileItemsOutFolderTest.cpp \
    TestCase/RenameHelperTest.cpp \
    TestCase/SceneInfoManagerTest.cpp \
    TestCase/ThumbnailProcessActions.cpp \
    TestCase/pub/FileSystemHelper.cpp \
    TestCase/FolderNxtAndLastIteratorTest.cpp \
    TestCase/JsonToolTest.cpp \
    TestCase/LongPathFinderTest.cpp \
    TestCase/MD5CalculatorTest.cpp \
    TestCase/NameSectionArrangeTest.cpp \
    TestCase/NameToolTest.cpp \
    TestCase/PathToolTest.cpp \
    TestCase/RedundantItemsRemoverTest.cpp \
    TestCase/RenameUnqiueCheckTest.cpp \
    TestCase/SceneViewTest.cpp \
    TestCase/SyncModifiyFileSystemTest.cpp \
    TestCase/ToConsecutiveFileNameNoTest.cpp \
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/SyncModifiyFileSystem.cpp \
    TestCase/ValueCheckerTest.cpp \
    TestCase/FinderTest.cpp \
    TestCase/VideosDurationGetterTest.cpp \
    Tools/FolderNxtAndLastIterator.cpp \
    Tools/ThumbnailProcesser.cpp \
    Tools/ItemsPileCategory.cpp \
    Tools/LongPathFinder.cpp \
    Tools/ProductionStudioManager.cpp \
    Tools/PathTool.cpp\
    Tools/NameTool.cpp\
    Tools/ConflictsItemHelper.cpp \
    Tools/ConvertUnicodeCharsetToAscii.cpp \
    Tools/RedundantFolderRemove.cpp \
    Tools/MD5Calculator.cpp \
    Tools/QMediaInfo.cpp \
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
    Tools/RenameHelper.cpp \
    Tools/SceneInfoManager.cpp \
    Tools/ToConsecutiveFileNameNo.cpp \
    Tools/NameSectionArrange.cpp \
    Tools/AIMediaDuplicate.cpp \
    Tools/JsonFileHelper.cpp \
    Tools/ExtractPileItemsOutFolder.cpp \
    Tools/ItemsPileCategory.cpp \
    model/ConflictsFileSystemModel.cpp \
    UndoRedo.cpp


HEADERS += \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Component/RenameWidgets/AdvanceRenamer.h \
    TestCase/pub/FileSystemHelper.h \
    TestCase/pub/BeginToExposePrivateMember.h \
    TestCase/pub/EndToExposePrivateMember.h \
    TestCase/pub/FileSystemRelatedTest.h \
    Tools/FolderNxtAndLastIterator.h \
    Tools/ThumbnailProcesser.h \
    Tools/ItemsPileCategory.h \
    Tools/LongPathFinder.h \
    Tools/ProductionStudioManager.h \
    Tools/PathTool.h\
    Tools/NameTool.h\
    Tools/ConflictsItemHelper.h \
    Tools/MD5Calculator.h \
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
    Tools/RenameHelper.h \
    Tools/SceneInfoManager.h \
    Tools/ToConsecutiveFileNameNo.h \
    Tools/ConvertUnicodeCharsetToAscii.h\
    Tools/NameSectionArrange.h \
    Tools/RedundantFolderRemove.h \
    Tools/AIMediaDuplicate.h \
    Tools/QMediaInfo.h \
    Tools/JsonFileHelper.h \
    Tools/ExtractPileItemsOutFolder.h \
    Tools/ItemsPileCategory.h \
    model/ConflictsFileSystemModel.h \
    UndoRedo.h

DISTFILES += \
    readme.md

RESOURCES += \
    bin/themes/res.qrc
