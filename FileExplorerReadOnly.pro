QT       += core gui sql multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Actions/DataBaseActions.cpp \
    Actions/FileBasicOperationsActions.cpp \
    Actions/FramelessWindowActions.cpp \
    Actions/JsonEditorActions.cpp \
    Actions/PerformersManagerActions.cpp \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    Actions/FileLeafAction.cpp \
    Actions/RightClickMenuActions.cpp \
    Actions/VideoPlayerActions.cpp \
    Actions/ViewActions.cpp \
    AddressELineEdit.cpp \
    AllQtLearning/Learning_Database.cpp \
    Component/AdvanceSearchWindow.cpp \
    Component/ClickableSlider.cpp \
    Component/CustomStatusBar.cpp \
    Component/DBRightClickMenu.cpp \
    Component/DatabaseToolBar.cpp \
    Component/JsonEditor.cpp \
    Component/JsonPerformersListInputer.cpp \
    Component/PerformersManagerWidget.cpp \
    Component/PerformersPreviewTextBrowser.cpp \
    Component/PerformersStringParser.cpp \
    Component/ProductionStudioManager.cpp \
    Component/QuickWhereClause.cpp \
    Component/RatingSqlTableModel.cpp \
    Component/RenameConflicts.cpp \
    Component/RightClickMenu.cpp \
    Component/StateLabel.cpp \
    Component/Toaster.cpp \
    Component/VideoPlayer.cpp \
    ContentPanel.cpp \
    FileOperation/FileOperation.cpp \
    MyQSqlTableModel.cpp \
    PublicTool.cpp \
    RibbonMenu.cpp \
    Tools/Categorizer.cpp \
    Tools/ConflictsItemHelper.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/JsonFileHelper.cpp \
    Tools/MimeDataCX.cpp \
    Tools/NameStandardizer.cpp \
    Tools/PerformerJsonFileHelper.cpp \
    Tools/PlayVideo.cpp \
    Tools/RenameWidget.cpp \
    Tools/SubscribeDatabase.cpp \
    UndoRedo.cpp \
    View/DatabaseTableView.cpp \
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
    View/ViewHelper.cpp \
    main.cpp \
    FileExplorerReadOnly.cpp

HEADERS += \
    Actions/DataBaseActions.h \
    Actions/FileBasicOperationsActions.h \
    Actions/FramelessWindowActions.h \
    Actions/JsonEditorActions.h \
    Actions/PerformersManagerActions.h \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Actions/FileLeafAction.h \
    Actions/RightClickMenuActions.h \
    Actions/VideoPlayerActions.h \
    Actions/ViewActions.h \
    AddressELineEdit.h \
    AllQtLearning/Learning_Database.h \
    Component/AdvanceSearchWindow.h \
    Component/ClickableSlider.h \
    Component/CustomStatusBar.h \
    Component/DBRightClickMenu.h \
    Component/DatabaseToolBar.h \
    Component/JsonEditor.h \
    Component/JsonPerformersListInputer.h \
    Component/PerformersManagerWidget.h \
    Component/PerformersPreviewTextBrowser.h \
    Component/PerformersStringParser.h \
    Component/ProductionStudioManager.h \
    Component/QuickWhereClause.h \
    Component/RatingSqlTableModel.h \
    Component/RenameConflicts.h \
    Component/RightClickMenu.h \
    Component/StateLabel.h \
    Component/Toaster.h \
    Component/VideoPlayer.h \
    ContentPanel.h \
    FileOperation/FileOperation.h \
    MyQSqlTableModel.h \
    PublicTool.h \
    RibbonMenu.h \
    Tools/Categorizer.h \
    Tools/ConflictsItemHelper.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/JsonFileHelper.h \
    Tools/MimeDataCX.h \
    Tools/NameStandardizer.h \
    Tools/PerformerJsonFileHelper.h \
    Tools/PlayVideo.h \
    Tools/RenameWidget.h \
    Tools/SubscribeDatabase.h \
    UndoRedo.h \
    View/DatabaseTableView.h \
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
    RightClickableToolBar.h \
    View/ViewHelper.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    themes/res.qrc

DISTFILES += \
    bin/PerformerHtmlTemplate.html \
    bin/STANDARD_STUDIO_NAME_JSON.json \
    readme.md

QMAKE_POST_LINK +=
    \

FORMS +=
