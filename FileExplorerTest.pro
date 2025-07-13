QT += testlib gui core sql widgets
CONFIG += qt warn_on depend_includepath testcase
CONFIG += c++11
win32 {
    LIBS += -ldwmapi
    LIBS += -lshlwapi
    # target_link_libraries(project_name Shlwapi) in CMakeLists
    # FFmpeg headers path
    INCLUDEPATH += "C:/home/ffmpeg/include"
    # FFmpeg libs path
    LIBS += -L"C:/home/ffmpeg/lib" -lavformat -lavcodec -lavutil -lswscale -lws2_32 # Windows network avformat_network_init need
}
linux {
    LIBS += -lavformat -lavcodec -lavutil -lswscale
}

TEMPLATE = app

DEFINES += QT_TEST
message($$DEFINES)

SOURCES += $$files(TestCase/Component/JsonEditorWidget/*.cpp) \
    Tools/VideoDurationGetter.cpp
SOURCES += $$files(TestCase/DbManager/*.cpp)
SOURCES += $$files(TestCase/Json/*.cpp)
SOURCES += $$files(TestCase/pub/*.cpp)
SOURCES += $$files(TestCase/public/*.cpp)
SOURCES += $$files(TestCase/FileOperation/*.cpp)
SOURCES += $$files(TestCase/rename/*.cpp)
SOURCES += $$files(TestCase/*.cpp)
SOURCES += $$files(public/*.cpp)
SOURCES += $$files(public/Memory/*.cpp)
SOURCES += $$files(FileOperation/*.cpp)
SOURCES += $$files(Tools/FileDescriptor/*.cpp)
SOURCES += $$files(Tools/Classify/*.cpp)
SOURCES += $$files(Tools/Json/*.cpp)

SOURCES += \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    Actions/LogActions.cpp \
    Actions/MovieDBActions.cpp \
    Actions/ActionsRecorder.cpp \
    Component/NotificatorPrivate.cpp \
    Component/Notificator.cpp \
    Component/QuickWhereClause.cpp \
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/RenameWidgets/CommandsPreview.cpp \
    Component/StateLabel.cpp \
    Component/JsonEditorWidget/LineEditCSV.cpp \
    Tools/AIMediaDuplicate.cpp \
    Tools/ConvertUnicodeCharsetToAscii.cpp \
    Tools/LowResImgsRemover.cpp \
    Tools/FolderNxtAndLastIterator.cpp \
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
    Tools/LogHandler.cpp \
    Tools/FileSystemItemFilter.cpp \
    Tools/RedunImgLibs.cpp \
    Tools/SyncModifiyFileSystem.cpp \
    Tools/WidgetReorderHelper.cpp

HEADERS += $$files(TestCase/pub/*.h) \
    Tools/VideoDurationGetter.h
HEADERS += $$files(TestCase/*.h)
HEADERS += $$files(public/*.h)
HEADERS += $$files(public/Memory/*.h)
HEADERS += $$files(FileOperation/*.h)
HEADERS += $$files(Tools/FileDescriptor/*.h)
SOURCES += $$files(Tools/Classify/*.h)
HEADERS += $$files(Tools/Json/*.h)
HEADERS += $$files(Tools/*.h)

HEADERS += \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Actions/LogActions.h \
    Actions/MovieDBActions.h \
    Actions/ActionsRecorder.h \
    Component/NotificatorPrivate.h \
    Component/Notificator.h \
    Component/QuickWhereClause.h \
    Component/RenameWidgets/AdvanceRenamer.h \
    Component/RenameWidgets/CommandsPreview.h \
    Component/StateLabel.h \
    Component/JsonEditorWidget/LineEditCSV.h \
    Tools/AIMediaDuplicate.h \
    Tools/ConvertUnicodeCharsetToAscii.h\
    Tools/LowResImgsRemover.h \
    Tools/FolderNxtAndLastIterator.h \
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
    Tools/LogHandler.h \
    Tools/FileSystemItemFilter.h \
    Tools/RedunImgLibs.h \
    Tools/SyncModifiyFileSystem.h \
    Tools/WidgetReorderHelper.h

DISTFILES += \
    readme.md

RESOURCES += \
    bin/themes/res.qrc
