QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AddressELineEdit.cpp \
    ContentPane.cpp \
    DragDropTableView.cpp \
    FileExplorerEvent.cpp \
    FileLeafAction.cpp \
    FolderPreviewComponent/FolderListView.cpp \
    FolderPreviewComponent/ImagesFileSystemModel.cpp \
    FolderPreviewComponent/ImagesListPreview.cpp \
    FolderPreviewComponent/OtherItemFileSystemModel.cpp \
    FolderPreviewComponent/OtherItemsListPreview.cpp \
    FolderPreviewComponent/VideosFileSystemModel.cpp \
    FolderPreviewComponent/VideosListPreview.cpp \
    FolderPreviewHTML.cpp \
    FolderPreviewWidget.cpp \
    MyQFileSystemModel.cpp \
    NavigationAndAddressBar.cpp \
    NavigationToolBar.cpp \
    OfficeStyleMenu.cpp \
    PathUndoRedoer.cpp \
    PublicVariable.cpp \
    RightClickableToolBar.cpp \
    main.cpp \
    FileExplorerReadOnly.cpp

HEADERS += \
    AddressELineEdit.h \
    ContentPane.h \
    DragDropTableView.h \
    FileExplorerEvent.h \
    FileExplorerReadOnly.h \
    FileLeafAction.h \
    FolderPreviewComponent/FolderListView.h \
    FolderPreviewComponent/ImagesFileSystemModel.h \
    FolderPreviewComponent/ImagesListPreview.h \
    FolderPreviewComponent/OtherItemFileSystemModel.h \
    FolderPreviewComponent/OtherItemsListPreview.h \
    FolderPreviewComponent/VideosFileSystemModel.h \
    FolderPreviewComponent/VideosListPreview.h \
    FolderPreviewHTML.h \
    FolderPreviewWidget.h \
    MyQFileSystemModel.h \
    NavigationAndAddressBar.h \
    NavigationToolBar.h \
    OfficeStyleMenu.h \
    PathUndoRedoer.h \
    PublicVariable.h \
    RightClickableToolBar.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    themes/res.qrc

DISTFILES += \
    readme.md
