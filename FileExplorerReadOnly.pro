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
    Actions/TorrentsManagerActions.cpp \
    Actions/VideoPlayerActions.cpp \
    Actions/ViewActions.cpp \
    AddressELineEdit.cpp \
    AllQtLearning/Learning_Database.cpp \
    Component/AdvanceSearchWindow.cpp \
    Component/AlertSystem.cpp \
    Component/ClickableSlider.cpp \
    Component/CustomStatusBar.cpp \
    Component/DBRightClickMenu.cpp \
    Component/DatabaseToolBar.cpp \
    Component/JsonEditor.cpp \
    Component/JsonPerformersListInputer.cpp \
    Component/MD5Window.cpp \
    Component/NotificatorFrame.cpp \
    Component/PerformersPreviewTextBrowser.cpp \
    Component/PerformersWidget.cpp \
    Component/PropertiesWindow.cpp \
    Component/QuickWhereClause.cpp \
    Component/RatingSqlTableModel.cpp \
    Component/RenameConflicts.cpp \
    Component/RightClickMenu.cpp \
    Component/StateLabel.cpp \
    Component/Toaster.cpp \
    Component/TorrentsManagerWidget.cpp \
    Component/VideoPlayer.cpp \
    ContentPanel.cpp \
    FileOperation/FileOperation.cpp \
    MyQSqlTableModel.cpp \
    PublicTool.cpp \
    RibbonMenu.cpp \
    SearchItemModel.cpp \
    Tools/ActionWithPath.cpp \
    Tools/Categorizer.cpp \
    Tools/ConflictsItemHelper.cpp \
    Tools/CopyItemPropertiesToClipboardIF.cpp \
    Tools/DuplicateImagesRemover.cpp \
    Tools/FileSystemItemFilter.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/JsonFileHelper.cpp \
    Tools/MD5Calculator.cpp \
    Tools/MP4DurationGetter.cpp \
    Tools/MimeDataCX.cpp \
    Tools/NameStandardizer.cpp \
    Tools/PathTool.cpp \
    Tools/PerformerJsonFileHelper.cpp \
    Tools/PerformersAkaManager.cpp \
    Tools/PerformersManager.cpp \
    Tools/ProductionStudioManager.cpp \
    Tools/PlayVideo.cpp \
    Tools/RedundantFolderRemove.cpp \
    Tools/RenameWidget.cpp \
    Tools/StringEditHelper.cpp \
    Tools/SubscribeDatabase.cpp \
    Tools/SysTerminal.cpp \
    Tools/TorrentsDatabaseHelper.cpp \
    Tools/VideoPlayerWatcher.cpp \
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
    View/SearchItemTableView.cpp \
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
    Actions/TorrentsManagerActions.h \
    Actions/VideoPlayerActions.h \
    Actions/ViewActions.h \
    AddressELineEdit.h \
    AllQtLearning/Learning_Database.h \
    Component/AdvanceSearchWindow.h \
    Component/AlertSystem.h \
    Component/ClickableSlider.h \
    Component/CustomStatusBar.h \
    Component/DBRightClickMenu.h \
    Component/DatabaseToolBar.h \
    Component/JsonEditor.h \
    Component/JsonPerformersListInputer.h \
    Component/MD5Window.h \
    Component/NotificatorFrame.h \
    Component/NotificatorFrame_p.h \
    Component/PerformersPreviewTextBrowser.h \
    Component/PerformersWidget.h \
    Component/PropertiesWindow.h \
    Component/QuickWhereClause.h \
    Component/RatingSqlTableModel.h \
    Component/RenameConflicts.h \
    Component/RightClickMenu.h \
    Component/StateLabel.h \
    Component/Toaster.h \
    Component/TorrentsManagerWidget.h \
    Component/VideoPlayer.h \
    ContentPanel.h \
    FileOperation/FileOperation.h \
    MyQSqlTableModel.h \
    PublicTool.h \
    RibbonMenu.h \
    SearchItemModel.h \
    Tools/ActionWithPath.h \
    Tools/Categorizer.h \
    Tools/ConflictsItemHelper.h \
    Tools/CopyItemPropertiesToClipboardIF.h \
    Tools/DuplicateImagesRemover.h \
    Tools/FileSystemItemFilter.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/JsonFileHelper.h \
    Tools/MD5Calculator.h \
    Tools/MP4DurationGetter.h \
    Tools/MimeDataCX.h \
    Tools/NameStandardizer.h \
    Tools/PathTool.h \
    Tools/PerformerJsonFileHelper.h \
    Tools/PerformersAkaManager.h \
    Tools/PerformersManager.h \
    Tools/ProductionStudioManager.h \
    Tools/PlayVideo.h \
    Tools/RedundantFolderRemove.h \
    Tools/RenameWidget.h \
    Tools/StringEditHelper.h \
    Tools/SubscribeDatabase.h \
    Tools/SysTerminal.h \
    Tools/TorrentsDatabaseHelper.h \
    Tools/VideoPlayerWatcher.h \
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
    View/SearchItemTableView.h \
    View/ViewHelper.h

TRANSLATIONS += \
    FileExplorerReadOnly_zh_CN.ts

CONFIG += lupdate
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    themes/res.qrc

DISTFILES += \
    FileExplorerReadOnly_zh_CN.ts \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.bat \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.sh \
    bin/PerformerHtmlTemplate.html \
    bin/JsonExample/StudioName - Movie Name - Performer 1, Performer 2.json \
    readme.md

QMAKE_POST_LINK +=
    \

FORMS +=
