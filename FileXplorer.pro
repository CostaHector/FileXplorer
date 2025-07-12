QT       += core gui sql multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
LIBS += -L$$PWD/lib
win32 {
    LIBS += -ldwmapi
    LIBS += -lMediaInfo
    # FFmpeg headers path
    INCLUDEPATH += "C:/home/ffmpeg/include"
    # FFmpeg libs path
    LIBS += -L"C:/home/ffmpeg/lib" -lavformat -lavcodec -lavutil -lswscale -lws2_32 # Windows network avformat_network_init need
}

CONFIG(debug, debug|release) {
    DEFINES += QT_MESSAGELOGCONTEXT
}

CONFIG(release, debug|release) {
#DEFINES -= QT_NO_MESSAGELOGCONTEXT
#DEFINES += QT_MESSAGELOGCONTEXT
## GCC/Clang
#QMAKE_CXXFLAGS_RELEASE += -g
## MSVC
#QMAKE_CXXFLAGS_RELEASE += /Zi
}

SOURCES += $$files(Actions/*.cpp)
SOURCES += $$files(Model/*.cpp)
SOURCES += $$files(View/*.cpp)
SOURCES += $$files(Component/*.cpp)
SOURCES += $$files(Component/FolderPreview/*.cpp)
SOURCES += $$files(Component/JsonEditorWidget/*.cpp)
SOURCES += $$files(Component/RenameWidgets/*.cpp)
SOURCES += $$files(Tools/*.cpp)
SOURCES += $$files(Tools/Classify/*.cpp)
SOURCES += $$files(Tools/FileDescriptor/*.cpp)
SOURCES += $$files(Tools/Json/*.cpp)
SOURCES += $$files(FileOperation/*.cpp)
SOURCES += $$files(public/*.cpp)
SOURCES += $$files(public/Memory/*.cpp)
SOURCES += $$files(*.cpp)

HEADERS += $$files(Actions/*.h)
HEADERS += $$files(Model/*.h)
HEADERS += $$files(View/*.h)
HEADERS += $$files(Component/*.h)
HEADERS += $$files(Component/FolderPreview/*.h)
HEADERS += $$files(Component/JsonEditorWidget/*.h)
HEADERS += $$files(Component/RenameWidgets/*.h)
HEADERS += $$files(Tools/*.h)
HEADERS += $$files(Tools/Classify/*.h)
HEADERS += $$files(Tools/FileDescriptor/*.h)
HEADERS += $$files(Tools/Json/*.h)
HEADERS += $$files(FileOperation/*.h)
HEADERS += $$files(public/*.h)
HEADERS += $$files(public/Memory/*.h)
HEADERS += $$files(bin/*.h)
HEADERS += $$files(*.h)

TRANSLATIONS += \
    Translate/FileXplorer_zh_CN.ts

CONFIG += lupdate
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    bin/themes/res.qrc \
    bin/themes/styles/dark/darkstyle.qrc\
    bin/themes/styles/default/defaultstyle.qrc\

DISTFILES += \
    Experiment.py \
    Translate/FileXplorer_zh_CN.ts \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.bat \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.sh \
    readme.md
