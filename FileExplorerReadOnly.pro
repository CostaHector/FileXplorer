QT       += core gui sql multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Actions/AddressBarActions.cpp \
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
    AdvanceSearchModel.cpp \
    Component/AddressELineEdit.cpp \
    AllQtLearning/Learning_Database.cpp \
    Component/AdvanceSearchMenu.cpp \
    Component/AdvanceSearchToolBar.cpp \
    Component/AlertSystem.cpp \
    Component/ClickableSlider.cpp \
    Component/CustomStatusBar.cpp \
    Component/DatabaseToolBar.cpp \
    Component/FileSystemMenu.cpp \
    Component/FileSystemTypeFilter.cpp \
    Component/JsonEditor.cpp \
    Component/JsonPerformersListInputer.cpp \
    Component/MD5Window.cpp \
    Component/MovieDatabaseMenu.cpp \
    Component/NotificatorFrame.cpp \
    Component/PerformersPreviewTextBrowser.cpp \
    Component/PerformersWidget.cpp \
    Component/PropertiesWindow.cpp \
    Component/QuickWhereClause.cpp \
    Component/RatingSqlTableModel.cpp \
    Component/RenameConflicts.cpp \
    Component/SearchCaseMatterToolButton.cpp \
    Component/SearchModeComboBox.cpp \
    Component/StackedToolBar.cpp \
    Component/StateLabel.cpp \
    Component/Toaster.cpp \
    ContentPanelImplementer.cpp \
    Tools/SearchProxyModel.cpp \
    Tools/MyClipboard.cpp \
    View/AdvanceSearchTableView.cpp \
    View/MovieDBView.cpp \
    View/TorrentsManagerWidget.cpp \
    Component/VideoPlayer.cpp \
    ContentPanel.cpp \
    Component/DatabaseSearchToolBar.cpp \
    FileOperation/FileOperation.cpp \
    MyQSqlTableModel.cpp \
    PublicTool.cpp \
    RibbonMenu.cpp \
    Tools/ActionWithPath.cpp \
    Tools/Categorizer.cpp \
    Tools/ConflictsItemHelper.cpp \
    Tools/CopyItemPropertiesToClipboardIF.cpp \
    Tools/DuplicateImagesRemover.cpp \
    Tools/ExtraViewVisibilityControl.cpp \
    Tools/FileSystemItemFilter.cpp \
    Tools/FilesNameBatchStandardizer.cpp \
    Tools/JsonFileHelper.cpp \
    Tools/MD5Calculator.cpp \
    Tools/MP4DurationGetter.cpp \
    Tools/MimeDataCX.cpp \
    Tools/NameStandardizer.cpp \
    Tools/NavigationViewSwitcher.cpp \
    Tools/PathTool.cpp \
    Tools/PerformerJsonFileHelper.cpp \
    Tools/PerformersAkaManager.cpp \
    Tools/PerformersManager.cpp \
    Tools/ProductionStudioManager.cpp \
    Tools/PlayVideo.cpp \
    Tools/RedundantFolderRemove.cpp \
    Tools/RenameWidget.cpp \
    Tools/StringEditHelper.cpp \
    Tools/SysTerminal.cpp \
    Tools/TorrentsDatabaseHelper.cpp \
    Tools/VideoPlayerWatcher.cpp \
    Tools/ViewSelection.cpp \
    UndoRedo.cpp \
    View/CustomTableView.cpp \
    FileExplorerEvent.cpp \
    FolderPreviewComponent/FolderListView.cpp \
    FolderPreviewComponent/ImagesFileSystemModel.cpp \
    FolderPreviewComponent/ImagesListPreview.cpp \
    FolderPreviewComponent/OtherItemFileSystemModel.cpp \
    FolderPreviewComponent/OtherItemsListPreview.cpp \
    FolderPreviewComponent/VideosFileSystemModel.cpp \
    FolderPreviewComponent/VideosListPreview.cpp \
    Component/FolderPreviewHTML.cpp \
    Component/FolderPreviewWidget.cpp \
    MyQFileSystemModel.cpp \
    Component/NavigationAndAddressBar.cpp \
    Component/NavigationToolBar.cpp \
    PathUndoRedoer.cpp \
    PublicVariable.cpp \
    Component/RightClickableToolBar.cpp \
    View/FileSystemListView.cpp \
    View/FileSystemTableView.cpp \
    View/FileSystemTreeView.cpp \
    View/PerformersTableView.cpp \
    View/ViewHelper.cpp \
    View/ViewStyleSheet.cpp \
    main.cpp \
    FileExplorerReadOnly.cpp

HEADERS += \
    Actions/AddressBarActions.h \
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
    AdvanceSearchModel.h \
    Component/AddressELineEdit.h \
    AllQtLearning/Learning_Database.h \
    Component/AdvanceSearchMenu.h \
    Component/AdvanceSearchToolBar.h \
    Component/AlertSystem.h \
    Component/ClickableSlider.h \
    Component/CustomStatusBar.h \
    Component/DatabaseToolBar.h \
    Component/FileSystemMenu.h \
    Component/FileSystemTypeFilter.h \
    Component/JsonEditor.h \
    Component/JsonPerformersListInputer.h \
    Component/MD5Window.h \
    Component/MovieDatabaseMenu.h \
    Component/NotificatorFrame.h \
    Component/NotificatorFrame_p.h \
    Component/PerformersPreviewTextBrowser.h \
    Component/PerformersWidget.h \
    Component/PropertiesWindow.h \
    Component/QuickWhereClause.h \
    Component/RatingSqlTableModel.h \
    Component/RenameConflicts.h \
    Component/SearchCaseMatterToolButton.h \
    Component/SearchModeComboBox.h \
    Component/StackedToolBar.h \
    Component/StateLabel.h \
    Component/Toaster.h \
    Tools/SearchProxyModel.h \
    Tools/MyClipboard.h \
    View/AdvanceSearchTableView.h \
    View/MovieDBView.h \
    View/TorrentsManagerWidget.h \
    Component/VideoPlayer.h \
    ContentPanel.h \
    Component/DatabaseSearchToolBar.h \
    FileOperation/FileOperation.h \
    MyQSqlTableModel.h \
    PublicTool.h \
    RibbonMenu.h \
    Tools/ActionWithPath.h \
    Tools/Categorizer.h \
    Tools/ConflictsItemHelper.h \
    Tools/CopyItemPropertiesToClipboardIF.h \
    Tools/DuplicateImagesRemover.h \
    Tools/ExtraViewVisibilityControl.h \
    Tools/FileSystemItemFilter.h \
    Tools/FilesNameBatchStandardizer.h \
    Tools/JsonFileHelper.h \
    Tools/MD5Calculator.h \
    Tools/MP4DurationGetter.h \
    Tools/MimeDataCX.h \
    Tools/NameStandardizer.h \
    Tools/NavigationViewSwitcher.h \
    Tools/PathTool.h \
    Tools/PerformerJsonFileHelper.h \
    Tools/PerformersAkaManager.h \
    Tools/PerformersManager.h \
    Tools/ProductionStudioManager.h \
    Tools/PlayVideo.h \
    Tools/RedundantFolderRemove.h \
    Tools/RenameWidget.h \
    Tools/StringEditHelper.h \
    Tools/SysTerminal.h \
    Tools/TorrentsDatabaseHelper.h \
    Tools/VideoPlayerWatcher.h \
    Tools/ViewSelection.h \
    UndoRedo.h \
    View/CustomTableView.h \
    FileExplorerEvent.h \
    FileExplorerReadOnly.h \
    FolderPreviewComponent/FolderListView.h \
    FolderPreviewComponent/ImagesFileSystemModel.h \
    FolderPreviewComponent/ImagesListPreview.h \
    FolderPreviewComponent/OtherItemFileSystemModel.h \
    FolderPreviewComponent/OtherItemsListPreview.h \
    FolderPreviewComponent/VideosFileSystemModel.h \
    FolderPreviewComponent/VideosListPreview.h \
    Component/FolderPreviewHTML.h \
    Component/FolderPreviewWidget.h \
    MyQFileSystemModel.h \
    Component/NavigationAndAddressBar.h \
    Component/NavigationToolBar.h \
    PathUndoRedoer.h \
    PublicVariable.h \
    Component/RightClickableToolBar.h \
    View/FileSystemListView.h \
    View/FileSystemTableView.h \
    View/FileSystemTreeView.h \
    View/PerformersTableView.h \
    View/ViewHelper.h \
    View/ViewStyleSheet.h

TRANSLATIONS += \
    Translate/FileExplorerReadOnly_zh_CN.ts

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
    Translate/FileExplorerReadOnly_zh_CN.ts \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.bat \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.sh \
    bin/PerformerHtmlTemplate.html \
    bin/JsonExample/StudioName - Movie Name - Performer 1, Performer 2.json \
    readme.md

QMAKE_POST_LINK +=
    \

FORMS +=
