QT += testlib gui core sql widgets
CONFIG += qt warn_on depend_includepath testcase

TEMPLATE = app

SOURCES += $$files(TestCase/pub/*.cpp)
SOURCES += $$files(TestCase/*.cpp)
SOURCES += $$files(public/*.cpp)
SOURCES += $$files(FileOperation/*.cpp)

SOURCES += \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    Component/NotificatorFrame.cpp \
    Component/QuickWhereClause.cpp \
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/StateLabel.cpp \
    Component/SyncModifiyFileSystem.cpp \
    Tools/AIMediaDuplicate.cpp \
    Tools/Classify/ItemsClassifier.cpp \
    Tools/Classify/SceneMixed.cpp \
    Tools/ConflictsItemHelper.cpp \
    Tools/ConvertUnicodeCharsetToAscii.cpp \
    Tools/DuplicateImagesRemover.cpp \
    Tools/ExtractPileItemsOutFolder.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/FolderNxtAndLastIterator.cpp \
    Tools/ItemsPileCategory.cpp \
    Tools/ItemsPileCategory.cpp \
    Tools/JsonFileHelper.cpp \
    Tools/LongPathFinder.cpp \
    Tools/MD5Calculator.cpp \
    Tools/NameSectionArrange.cpp \
    Tools/NameStandardizer.cpp \
    Tools/NameTool.cpp\
    Tools/PerformersAkaManager.cpp \
    Tools/PerformersManager.cpp \
    Tools/ProductionStudioManager.cpp \
    Tools/QMediaInfo.cpp \
    Tools/RedundantFolderRemove.cpp \
    Tools/RenameHelper.cpp \
    Tools/RenameNamesUnique.cpp \
    Tools/SceneInfoManager.cpp \
    Tools/ThumbnailProcesser.cpp \
    Tools/ToConsecutiveFileNameNo.cpp \
    model/ConflictsFileSystemModel.cpp \

HEADERS += $$files(TestCase/*.h)
HEADERS += $$files(TestCase/pub/*.h)
HEADERS += $$files(public/*.h)
HEADERS += $$files(FileOperation/*.h)

HEADERS += \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Component/NotificatorFrame.h \
    Component/QuickWhereClause.h \
    Component/RenameWidgets/AdvanceRenamer.h \
    Component/StateLabel.h \
    Component/SyncModifiyFileSystem.h \
    Tools/AIMediaDuplicate.h \
    Tools/Classify/ItemsClassifier.h \
    Tools/Classify/SceneMixed.h \
    Tools/ConflictsItemHelper.h \
    Tools/ConvertUnicodeCharsetToAscii.h\
    Tools/DuplicateImagesRemover.h \
    Tools/ExtractPileItemsOutFolder.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/FolderNxtAndLastIterator.h \
    Tools/ItemsPileCategory.h \
    Tools/ItemsPileCategory.h \
    Tools/JsonFileHelper.h \
    Tools/LongPathFinder.h \
    Tools/MD5Calculator.h \
    Tools/NameSectionArrange.h \
    Tools/NameStandardizer.h \
    Tools/NameTool.h\
    Tools/PerformersAkaManager.h \
    Tools/PerformersManager.h \
    Tools/ProductionStudioManager.h \
    Tools/QMediaInfo.h \
    Tools/RedundantFolderRemove.h \
    Tools/RenameHelper.h \
    Tools/RenameNamesUnique.h \
    Tools/SceneInfoManager.h \
    Tools/ThumbnailProcesser.h \
    Tools/ToConsecutiveFileNameNo.h \
    model/ConflictsFileSystemModel.h \

DISTFILES += \
    readme.md

RESOURCES += \
    bin/themes/res.qrc

DEFINES += QT_TEST
