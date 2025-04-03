QT += testlib gui core sql
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += $$files(TestCase/*.cpp)

SOURCES += \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    MainTestSuite.cpp \
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/SyncModifiyFileSystem.cpp \
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
    Tools/Classify/ItemsClassifier.cpp \
    Tools/Classify/SceneMixed.cpp \
    Component/QuickWhereClause.cpp \
    Component/StateLabel.cpp \
    Component/NotificatorFrame.cpp \
    FileOperation/FileOperation.cpp \
    FileOperation/FileOperatorPub.cpp \
    PublicTool.cpp \
    PublicVariable.cpp \
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


HEADERS += $$files(TestCase/pub/*.h)

HEADERS += \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Component/RenameWidgets/AdvanceRenamer.h \
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
    Tools/Classify/ItemsClassifier.h \
    Tools/Classify/SceneMixed.h \
    model/ConflictsFileSystemModel.h \
    UndoRedo.h \
    public/DisplayEnhancement.h

DISTFILES += \
    readme.md

RESOURCES += \
    bin/themes/res.qrc

DEFINES += QT_TEST
