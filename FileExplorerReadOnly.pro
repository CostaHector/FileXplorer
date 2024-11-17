QT       += core gui sql multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
#for windows
LIBS += -lz
# for linux
#LIBS += -ldl
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Actions/AddressBarActions.cpp \
    Actions/ArchiveFilesActions.cpp \
    Actions/ArrangeActions.cpp \
    Actions/ConflictsSolveActions.cpp \
    Actions/DataBaseActions.cpp \
    Actions/DuplicateVideosFinderActions.cpp \
    Actions/FileBasicOperationsActions.cpp \
    Actions/FolderPreviewActions.cpp \
    Actions/FramelessWindowActions.cpp \
    Actions/JsonEditorActions.cpp \
    Actions/PerformersManagerActions.cpp \
    Actions/PropertiesWindowActions.cpp \
    Actions/QuickWhereActions.cpp \
    Actions/RenameActions.cpp \
    Actions/FileLeafAction.cpp \
    Actions/RightClickMenuActions.cpp \
    Actions/SceneInPageActions.cpp \
    Actions/SyncFileSystemModificationActions.cpp \
    Actions/TorrentsManagerActions.cpp \
    Actions/VideoPlayerActions.cpp \
    Actions/ViewActions.cpp \
    AdvanceSearchModel.cpp \
    Component/AddressELineEdit.cpp \
    AllQtLearning/Learning_Database.cpp \
    Component/AdvanceSearchMenu.cpp \
    Component/AdvanceSearchToolBar.cpp \
    Component/AlertSystem.cpp \
    Component/Archiver.cpp \
    Component/ClickableSlider.cpp \
    Component/ConflictsRecycle.cpp \
    Component/CustomStatusBar.cpp \
    Component/DatabaseToolBar.cpp \
    Component/DropListToolButton.cpp \
    Component/FileSystemMenu.cpp \
    Component/FileSystemTypeFilter.cpp \
    Component/JsonEditor.cpp \
    Component/JsonPerformersListInputer.cpp \
    Component/MD5Window.cpp \
    Component/MovieDatabaseMenu.cpp \
    Component/NotificatorFrame.cpp \
    Component/PerformersPreviewTextBrowser.cpp \
    Component/PerformersWidget.cpp \
    Component/FolderPreview/PreviewFolder.cpp \
    Component/FolderPreview/PreviewBrowser.cpp \
    Component/FolderPreview/PreviewLabels.cpp \
    Component/FolderPreview/PreviewLists.cpp \
    Component/PropertiesWindow.cpp \
    Component/QuickWhereClause.cpp \
    Component/RatingSqlTableModel.cpp \
    Component/RedundantImageFinder.cpp \
    Component/RenameWidgets/LongPathFolderRenamer.cpp \
    Component/RenameWidgets/RenameWidget_ArrangeSection.cpp \
    Component/SearchCaseMatterToolButton.cpp \
    Component/SearchModeComboBox.cpp \
    Component/StackedToolBar.cpp \
    Component/StateLabel.cpp \
    Component/SyncModifiyFileSystem.cpp \
    Component/Toaster.cpp \
    ContentPanelImplementer.cpp \
    FileOperation/FileOperatorPub.cpp \
    Model/AiMediaTablesModel.cpp \
    Model/ConflictsFileSystemModel.cpp \
    Model/DifferRootFileSystemModel.cpp \
    Model/DuplicateVideoModel.cpp \
    Model/JsonModel.cpp \
    Model/LogProxyModel.cpp \
    Model/PreferenceModel.cpp \
    Model/ScenesTableModel.cpp \
    Model/VidModel.cpp \
    Tools/AIMediaDuplicate.cpp \
    Tools/ArchiveFiles.cpp \
    Tools/ConvertUnicodeCharsetToAscii.cpp \
    Tools/ExtractPileItemsOutFolder.cpp \
    Tools/FolderNxtAndLastIterator.cpp \
    Tools/FolderPreviewSwitcher.cpp \
    Tools/MessageOutput.cpp \
    Tools/NameSectionArrange.cpp \
    Tools/NameTool.cpp \
    Tools/QMediaInfo.cpp \
    Tools/RenameHelper.cpp \
    Tools/RenameNamesUnique.cpp \
    Tools/SceneInfoManager.cpp \
    Tools/SearchProxyModel.cpp \
    Tools/MyClipboard.cpp \
    Tools/ToConsecutiveFileNameNo.cpp \
    Tools/ViewTypeTool.cpp \
    Tools/VidsDurationDisplayString.cpp \
    Tools/LongPathFinder.cpp \
    View/AdvanceSearchTableView.cpp \
    View/AiMediaDupTableView.cpp \
    View/CustomListView.cpp \
    View/DuplicateVideosFinder.cpp \
    View/JsonListView.cpp \
    View/MovieDBView.cpp \
    View/SceneActionsSubscribe.cpp \
    View/SceneTableView.cpp \
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
    Component/RenameWidgets/AdvanceRenamer.cpp \
    Component/RenameWidgets/RenameWidget_ConvertBoldUnicodeCharset2Ascii.cpp \
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
    View/VidsPlayListView.cpp \
    View/ViewHelper.cpp \
    View/ViewStyleSheet.cpp \
    main.cpp \
    FileExplorerReadOnly.cpp

HEADERS += \
    Actions/AddressBarActions.h \
    Actions/ArchiveFilesActions.h \
    Actions/ArrangeActions.h \
    Actions/ConflictsSolveActions.h \
    Actions/DataBaseActions.h \
    Actions/DuplicateVideosFinderActions.h \
    Actions/FileBasicOperationsActions.h \
    Actions/FolderPreviewActions.h \
    Actions/FramelessWindowActions.h \
    Actions/JsonEditorActions.h \
    Actions/PerformersManagerActions.h \
    Actions/PropertiesWindowActions.h \
    Actions/QuickWhereActions.h \
    Actions/RenameActions.h \
    Actions/FileLeafAction.h \
    Actions/RightClickMenuActions.h \
    Actions/SceneInPageActions.h \
    Actions/SyncFileSystemModificationActions.h \
    Actions/TorrentsManagerActions.h \
    Actions/VideoPlayerActions.h \
    Actions/ViewActions.h \
    AdvanceSearchModel.h \
    Component/AddressELineEdit.h \
    AllQtLearning/Learning_Database.h \
    Component/AdvanceSearchMenu.h \
    Component/AdvanceSearchToolBar.h \
    Component/AlertSystem.h \
    Component/Archiver.h \
    Component/ClickableSlider.h \
    Component/ConflictsRecycle.h \
    Component/CustomStatusBar.h \
    Component/DatabaseToolBar.h \
    Component/DropListToolButton.h \
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
    Component/FolderPreview/PreviewBrowser.h \
    Component/FolderPreview/PreviewFolder.h \
    Component/FolderPreview/PreviewLabels.h \
    Component/FolderPreview/PreviewLists.h \
    Component/PropertiesWindow.h \
    Component/QuickWhereClause.h \
    Component/RatingSqlTableModel.h \
    Component/RedundantImageFinder.h \
    Component/RenameWidgets/LongPathFolderRenamer.h \
    Component/RenameWidgets/RenameWidget_ArrangeSection.h \
    Component/SearchCaseMatterToolButton.h \
    Component/SearchModeComboBox.h \
    Component/StackedToolBar.h \
    Component/StateLabel.h \
    Component/SyncModifiyFileSystem.h \
    Component/Toaster.h \
    FileOperation/FileOperatorPub.h \
    Model/AiMediaTablesModel.h \
    Model/ConflictsFileSystemModel.h \
    Model/DifferRootFileSystemModel.h \
    Model/DuplicateVideoModel.h \
    Model/JsonModel.h \
    Model/LogProxyModel.h \
    Model/PreferenceModel.h \
    Model/ScenesTableModel.h \
    Model/VidModel.h \
    Tools/AIMediaDuplicate.h \
    Tools/ArchiveFiles.h \
    Tools/ConvertUnicodeCharsetToAscii.h \
    Tools/ExtractPileItemsOutFolder.h \
    Tools/FolderNxtAndLastIterator.h \
    Tools/FolderPreviewSwitcher.h \
    Tools/MediaInfoDLL.h \
    Tools/MessageOutput.h \
    Tools/NameSectionArrange.h \
    Tools/NameTool.h \
    Tools/QAbstractTableModelPub.h \
    Tools/QMediaInfo.h \
    Tools/RenameHelper.h \
    Tools/RenameNamesUnique.h \
    Tools/SceneInfoManager.h \
    Tools/SearchProxyModel.h \
    Tools/MyClipboard.h \
    Tools/ToConsecutiveFileNameNo.h \
    Tools/ViewTypeTool.h \
    Tools/VidsDurationDisplayString.h \
    Tools/LongPathFinder.h \
    View/AdvanceSearchTableView.h \
    View/AiMediaDupTableView.h \
    View/CustomListView.h \
    View/DuplicateVideosFinder.h \
    View/JsonListView.h \
    View/MovieDBView.h \
    View/SceneActionsSubscribe.h \
    View/SceneTableView.h \
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
    Component/RenameWidgets/AdvanceRenamer.h \
    Component/RenameWidgets/RenameWidget_ConvertBoldUnicodeCharset2Ascii.h \
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
    View/VidsPlayListView.h \
    View/ViewHelper.h \
    View/ViewStyleSheet.h \
    public/DisplayEnhancement.h

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
    bin/themes/res.qrc

DISTFILES += \
    Experiment.py \
    Translate/FileExplorerReadOnly_zh_CN.ts \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.bat \
    bin/TERMINAL_OPEN_BATCH_FILE_PATH.sh \
    bin/PerformerHtmlTemplate.html \
    readme.md

QMAKE_POST_LINK +=
    \

FORMS +=
