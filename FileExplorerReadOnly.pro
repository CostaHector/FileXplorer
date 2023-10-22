QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Actions/FileBasicOperationsActions.cpp \
    Actions/FramelessWindowActions.cpp \
    Actions/RenameActions.cpp \
    Actions/FileLeafAction.cpp \
    AddressELineEdit.cpp \
    Component/CustomStatusBar.cpp \
    Component/RenameConflicts.cpp \
    Component/RightClickMenu.cpp \
    Component/StateLabel.cpp \
    Component/Toaster.cpp \
    ContentPanel.cpp \
    FileOperation/FileOperation.cpp \
    PublicTool.cpp \
    RibbonMenu.cpp \
    Tools/Categorizer.cpp \
    Tools/ConflictsItemHelper.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/MimeDataCX.cpp \
    Tools/NameStandardizer.cpp \
    Tools/PlayVideo.cpp \
    Tools/RenameWidget.cpp \
    UndoRedo.cpp \
    View/DragDropTableView.cpp \
    FileExplorerEvent.cpp \
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
    PathUndoRedoer.cpp \
    PublicVariable.cpp \
    RightClickableToolBar.cpp \
    main.cpp \
    FileExplorerReadOnly.cpp

HEADERS += \
    Actions/FileBasicOperationsActions.h \
    Actions/FramelessWindowActions.h \
    Actions/RenameActions.h \
    Actions/FileLeafAction.h \
    AddressELineEdit.h \
    Component/CustomStatusBar.h \
    Component/RenameConflicts.h \
    Component/RightClickMenu.h \
    Component/StateLabel.h \
    Component/Toaster.h \
    ContentPanel.h \
    FileOperation/FileOperation.h \
    PublicTool.h \
    RibbonMenu.h \
    Tools/Categorizer.h \
    Tools/ConflictsItemHelper.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/MimeDataCX.h \
    Tools/NameStandardizer.h \
    Tools/PlayVideo.h \
    Tools/RenameWidget.h \
    UndoRedo.h \
    View/DragDropTableView.h \
    FileExplorerEvent.h \
    FileExplorerReadOnly.h \
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

QMAKE_POST_LINK +=
    \
