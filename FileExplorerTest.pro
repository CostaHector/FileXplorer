QT += testlib gui core sql widgets
CONFIG += qt warn_on depend_includepath testcase
CONFIG += c++11
win32 {
    LIBS += -ldwmapi
}

TEMPLATE = app

DEFINES += QT_TEST
message($$DEFINES)

SOURCES += $$files(TestCase/Component/JsonEditorWidget/*.cpp)
SOURCES += $$files(TestCase/DbManager/*.cpp)
SOURCES += $$files(TestCase/Json/*.cpp)
SOURCES += $$files(TestCase/pub/*.cpp)
SOURCES += $$files(TestCase/public/*.cpp)
SOURCES += $$files(TestCase/*.cpp)
SOURCES += $$files(public/*.cpp)
SOURCES += $$files(public/Memory/*.cpp)
SOURCES += $$files(FileOperation/*.cpp)
SOURCES += $$files(Tools/FileDescriptor/*.cpp)
SOURCES += $$files(Tools/Json/*.cpp)

SOURCES += \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    Actions/LogActions.cpp \
    Actions/MovieDBActions.cpp \
    Component/NotificatorPrivate.cpp \
    Component/Notificator.cpp \
    Component/QuickWhereClause.cpp \
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/StateLabel.cpp \
    Component/SyncModifiyFileSystem.cpp \
    Component/JsonEditorWidget/LineEditCSV.cpp \
    model/ConflictsFileSystemModel.cpp \
    Tools/AIMediaDuplicate.cpp \
    Tools/Classify/ItemsClassifier.cpp \
    Tools/Classify/SceneMixed.cpp \
    Tools/ConflictsItemHelper.cpp \
    Tools/ConvertUnicodeCharsetToAscii.cpp \
    Tools/LowResImgsRemover.cpp \
    Tools/ExtractPileItemsOutFolder.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/FolderNxtAndLastIterator.cpp \
    Tools/ItemsPileCategory.cpp \
    Tools/ItemsPileCategory.cpp \
    Tools/LongPathFinder.cpp \
    Tools/MD5Calculator.cpp \
    Tools/NameSectionArrange.cpp \
    Tools/NameStandardizer.cpp \
    Tools/NameTool.cpp\
    Tools/PerformersAkaManager.cpp \
    Tools/CastManager.cpp \
    Tools/StudiosManager.cpp \
    Tools/QMediaInfo.cpp \
    Tools/RedundantFolderRemove.cpp \
    Tools/RenameHelper.cpp \
    Tools/RenameNamesUnique.cpp \
    Tools/SceneInfoManager.cpp \
    Tools/ThumbnailProcesser.cpp \
    Tools/ToConsecutiveFileNameNo.cpp \
    Tools/SplitterInsertIndexHelper.cpp \
    Tools/LogHandler.cpp \
    Tools/FileSystemItemFilter.cpp \
    Tools/RedunImgLibs.cpp \

HEADERS += $$files(TestCase/pub/*.h)
HEADERS += $$files(TestCase/*.h)
HEADERS += $$files(public/*.h)
HEADERS += $$files(public/Memory/*.h)
HEADERS += $$files(FileOperation/*.h)
HEADERS += $$files(Tools/FileDescriptor/*.h)
HEADERS += $$files(Tools/Json/*.h)

HEADERS += \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Actions/LogActions.h \
    Actions/MovieDBActions.h \
    Component/NotificatorPrivate.h \
    Component/Notificator.h \
    Component/QuickWhereClause.h \
    Component/RenameWidgets/AdvanceRenamer.h \
    Component/StateLabel.h \
    Component/SyncModifiyFileSystem.h \
    Component/JsonEditorWidget/LineEditCSV.h \
    model/ConflictsFileSystemModel.h \
    Tools/AIMediaDuplicate.h \
    Tools/Classify/ItemsClassifier.h \
    Tools/Classify/SceneMixed.h \
    Tools/ConflictsItemHelper.h \
    Tools/ConvertUnicodeCharsetToAscii.h\
    Tools/LowResImgsRemover.h \
    Tools/ExtractPileItemsOutFolder.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/FolderNxtAndLastIterator.h \
    Tools/ItemsPileCategory.h \
    Tools/ItemsPileCategory.h \
    Tools/LongPathFinder.h \
    Tools/MD5Calculator.h \
    Tools/NameSectionArrange.h \
    Tools/NameStandardizer.h \
    Tools/NameTool.h\
    Tools/PerformersAkaManager.h \
    Tools/CastManager.h \
    Tools/StudiosManager.h \
    Tools/QMediaInfo.h \
    Tools/RedundantFolderRemove.h \
    Tools/RenameHelper.h \
    Tools/RenameNamesUnique.h \
    Tools/SceneInfoManager.h \
    Tools/ThumbnailProcesser.h \
    Tools/ToConsecutiveFileNameNo.h \
    Tools/SplitterInsertIndexHelper.h \
    Tools/LogHandler.h \
    Tools/FileSystemItemFilter.h \
    Tools/RedunImgLibs.h \

DISTFILES += \
    readme.md

RESOURCES += \
    bin/themes/res.qrc
