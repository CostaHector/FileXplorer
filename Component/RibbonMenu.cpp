#include "RibbonMenu.h"
#include <QMenu>
#include <QTabBar>
#include <QToolButton>
#include "ActionsRecorder.h"
#include "ActionsSearcher.h"
#include "ArchiveFilesActions.h"
#include "FileRenameRulerActions.h"
#include "FileOpActs.h"
#include "FileLeafAction.h"
#include "FolderPreviewActions.h"
#include "PreferenceActions.h"
#include "RenameActions.h"
#include "RightClickMenuActions.h"
#include "SceneInPageActions.h"
#include "SyncFileSystemModificationActions.h"
#include "ViewActions.h"
#include "ThumbnailProcessActions.h"
#include "LogActions.h"
#include "RibbonCastDB.h"
#include "RibbonJson.h"
#include "RibbonMovieDB.h"
#include "MenuToolButton.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

RibbonMenu::RibbonMenu(QWidget* parent)
    : QTabWidget{parent}  //
{
  m_scenePageControl = new (std::nothrow) ScenePageControl{"Pagination display", this};

  m_leafFile = LeafFile();
  m_leafHome = LeafHome();
  m_leafView = LeafView();
  m_leafMovie = LeafMovie();
  m_leafCast = LeafCast();
  m_leafScenes = LeafScenesTools();
  m_leafJson = LeafJson();
  m_leafMedia = LeafMediaTools();
  {
    addTab(m_leafFile, "&FILE");
    addTab(m_leafHome, "&HOME");
    addTab(m_leafView, "&VIEW");
    const auto& viewIns = g_viewActions();
    using namespace ViewTypeTool;
    std::fill(mViewType2LeafTabIndex, mViewType2LeafTabIndex + (int)ViewType::VIEW_TYPE_BUTT, -1);
    mViewType2LeafTabIndex[(int)ViewType::MOVIE] = addTab(m_leafMovie, "&" + viewIns._MOVIE_VIEW->text());
    mViewType2LeafTabIndex[(int)ViewType::CAST] = addTab(m_leafCast, "&" + viewIns._CAST_VIEW->text());
    mViewType2LeafTabIndex[(int)ViewType::SCENE] = addTab(m_leafScenes, "&" + viewIns._SCENE_VIEW->text());
    mViewType2LeafTabIndex[(int)ViewType::JSON] = addTab(m_leafJson, "&" + viewIns._JSON_VIEW->text());
    addTab(m_leafMedia, "&ARRANGE");
  }
  auto& inst = ActionsRecorder::GetInst();
  inst.FromToolbar(m_leafFile);
  inst.FromToolbar(m_leafView);
  inst.FromToolbar(m_leafMedia);

  _EXPAND_RIBBONS = new (std::nothrow) QAction{QIcon{":img/EXPAND_RIBBON"}, "Expand or Hide Ribbon Menu", this};
  CHECK_NULLPTR_RETURN_VOID(_EXPAND_RIBBONS)
  _EXPAND_RIBBONS->setShortcut(QKeySequence(Qt::KeyboardModifier::ControlModifier | Qt::Key::Key_1));
  _EXPAND_RIBBONS->setToolTip(QString("<b>%1 (%2)</b><br/>Displays the Ribbon Menu when enabled, or hides it when disabled.")
                                  .arg(_EXPAND_RIBBONS->text(), _EXPAND_RIBBONS->shortcut().toString()));
  _EXPAND_RIBBONS->setCheckable(true);
  _EXPAND_RIBBONS->setChecked(Configuration().value(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.v).toBool());
  m_corner = GetMenuRibbonCornerWid();
  setCornerWidget(m_corner, Qt::Corner::TopRightCorner);

  Subscribe();
  AfterSubscribeInitialSettings();
}

QToolBar* RibbonMenu::GetMenuRibbonCornerWid(QWidget* attached) {
  ActionsSearcher* mActSearcher{new (std::nothrow) ActionsSearcher{attached}};
  CHECK_NULLPTR_RETURN_NULLPTR(mActSearcher);

  QToolBar* menuRibbonCornerWid{new (std::nothrow) QToolBar{"corner tools", attached}};
  CHECK_NULLPTR_RETURN_NULLPTR(menuRibbonCornerWid);

  menuRibbonCornerWid->addWidget(mActSearcher);
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_rightClickActions()._SEARCH_IN_NET_EXPLORER);
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_LogActions()._LOG_FILE);
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addActions(FileOpActs::GetInst().UNDO_REDO_RIBBONS->actions());
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(_EXPAND_RIBBONS);
  menuRibbonCornerWid->setIconSize(QSize{IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16});
  return menuRibbonCornerWid;
}

QToolBar* RibbonMenu::LeafFile() const {
  QToolBar* leafFileWid{new (std::nothrow) QToolBar{"File"}};
  CHECK_NULLPTR_RETURN_NULLPTR(leafFileWid)

  QToolBar* syncSwitchToolBar = g_syncFileSystemModificationActions().GetSyncSwitchToolbar(leafFileWid);
  CHECK_NULLPTR_RETURN_NULLPTR(syncSwitchToolBar);

  QToolBar* syncPathToolBar = g_syncFileSystemModificationActions().GetSyncPathToolbar(leafFileWid);
  CHECK_NULLPTR_RETURN_NULLPTR(syncPathToolBar);

  QToolBar* styleAndStylesheetToolBar = g_PreferenceActions().GetStyleAndStyleSheetToolbar(leafFileWid);
  CHECK_NULLPTR_RETURN_NULLPTR(styleAndStylesheetToolBar);

  QToolButton* logToolButton = g_LogActions().GetLogToolButton(leafFileWid);
  CHECK_NULLPTR_RETURN_NULLPTR(logToolButton);

  auto& fileLeafInst = g_fileLeafActions();
  auto* systemContextMenuTb = fileLeafInst.GetSystemContextMenu(leafFileWid);
  CHECK_NULLPTR_RETURN_NULLPTR(systemContextMenuTb);

  leafFileWid->addAction(fileLeafInst._SETTINGS);
  leafFileWid->addAction(fileLeafInst._PWD_BOOK);
  leafFileWid->addAction(fileLeafInst._ABOUT_FILE_EXPLORER);
  leafFileWid->addAction(fileLeafInst._LANUAGE);
  leafFileWid->addWidget(systemContextMenuTb);
  leafFileWid->addSeparator();
  leafFileWid->addWidget(syncSwitchToolBar);
  leafFileWid->addWidget(syncPathToolBar);
  leafFileWid->addSeparator();
  leafFileWid->addWidget(styleAndStylesheetToolBar);
  leafFileWid->addSeparator();
  leafFileWid->addWidget(logToolButton);
  leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafFileWid;
}

QToolBar* RibbonMenu::LeafHome() const {
  auto& fileOpInst = FileOpActs::GetInst();

  QToolBar* leafHomeWid = new (std::nothrow) QToolBar{"LeafHome"};
  CHECK_NULLPTR_RETURN_NULLPTR(leafHomeWid);

  QToolBar* openItemsTB = new (std::nothrow) QToolBar{"Open", leafHomeWid};
  CHECK_NULLPTR_RETURN_NULLPTR(openItemsTB);
  {
    openItemsTB->addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
    openItemsTB->addActions(fileOpInst.OPEN_AG->actions());
    openItemsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    openItemsTB->setOrientation(Qt::Orientation::Vertical);
    openItemsTB->setStyleSheet("QToolBar { max-width: 256px; }");
    openItemsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    SetLayoutAlightment(openItemsTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* propertiesTB = new (std::nothrow) QToolBar{"Properties", leafHomeWid};
  CHECK_NULLPTR_RETURN_NULLPTR(propertiesTB);
  {
    auto* copyTB = new MenuToolButton(fileOpInst.COPY_PATH_AG->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                      IMAGE_SIZE::TABS_ICON_IN_MENU_16, propertiesTB);
    copyTB->InitDefaultActionFromQSetting(MemoryKey::DEFAULT_COPY_CHOICE, true);
    propertiesTB->addWidget(copyTB);
    propertiesTB->addAction(g_rightClickActions()._CALC_MD5_ACT);
    propertiesTB->addAction(g_rightClickActions()._PROPERTIES);
    propertiesTB->setOrientation(Qt::Orientation::Vertical);
    propertiesTB->setStyleSheet("QToolBar { max-width: 256px; }");
    propertiesTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    propertiesTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    SetLayoutAlightment(propertiesTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* newItemsTB = new (std::nothrow) MenuToolButton(fileOpInst.NEW->actions(), QToolButton::MenuButtonPopup,
                                                       Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_48, leafHomeWid);
  newItemsTB->InitDefaultActionFromQSetting(MemoryKey::DEFAULT_NEW_CHOICE, true);

  QToolBar* moveCopyItemsToTB = new (std::nothrow) QToolBar{"Move/Copy item(s) To ToolBar", leafHomeWid};
  CHECK_NULLPTR_RETURN_NULLPTR(moveCopyItemsToTB);
  {
    const auto& _MOVE_TO_HIST_LIST = fileOpInst.MOVE_TO_PATH_HISTORY->actions();
    const auto& _COPY_TO_HIST_LIST = fileOpInst.COPY_TO_PATH_HISTORY->actions();
    auto* pMoveToToolButton = new MenuToolButton(_MOVE_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup,
                                                 Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_24, moveCopyItemsToTB);
    pMoveToToolButton->setDefaultAction(fileOpInst._MOVE_TO);
    auto* pCopyToToolButton = new MenuToolButton(_COPY_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup,
                                                 Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_24, moveCopyItemsToTB);
    pCopyToToolButton->setDefaultAction(fileOpInst._COPY_TO);

    moveCopyItemsToTB->addWidget(pMoveToToolButton);
    moveCopyItemsToTB->addWidget(pCopyToToolButton);
    moveCopyItemsToTB->setOrientation(Qt::Orientation::Horizontal);
    SetLayoutAlightment(moveCopyItemsToTB->layout(), Qt::AlignmentFlag::AlignTop);
  }

  QToolBar* cutCopyPasterTb = fileOpInst.GetCutCopyPasteTb(leafHomeWid);
  CHECK_NULLPTR_RETURN_NULLPTR(cutCopyPasterTb);

  QToolBar* folderOpModeTb = fileOpInst.GetFolderOperationModeTb(leafHomeWid);
  CHECK_NULLPTR_RETURN_NULLPTR(folderOpModeTb);

  QToolButton* recycleItemsTB =
      new (std::nothrow) MenuToolButton(fileOpInst.DELETE_ACTIONS->actions(), QToolButton::MenuButtonPopup,
                                        Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_48, leafHomeWid);
  CHECK_NULLPTR_RETURN_NULLPTR(recycleItemsTB);
  recycleItemsTB->setDefaultAction(fileOpInst.MOVE_TO_TRASHBIN);

  QToolBar* archievePreviewToolBar = new (std::nothrow) QToolBar{"ArchievePreview", leafHomeWid};
  CHECK_NULLPTR_RETURN_NULLPTR(archievePreviewToolBar);
  {
    archievePreviewToolBar->addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
    archievePreviewToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    archievePreviewToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  }

  QToolBar* selectionToolBar = new (std::nothrow) QToolBar{"Selection", leafHomeWid};
  CHECK_NULLPTR_RETURN_NULLPTR(selectionToolBar);
  {
    selectionToolBar->addActions(fileOpInst.SELECTION_RIBBONS->actions());
    selectionToolBar->setOrientation(Qt::Orientation::Vertical);
    selectionToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    selectionToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    selectionToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(selectionToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* compressToolBar = new (std::nothrow) QToolBar{"Compress/Decompress", leafHomeWid};
  CHECK_NULLPTR_RETURN_NULLPTR(compressToolBar);
  {
    compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_HERE);
    compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_IMAGES);
    compressToolBar->addAction(g_AchiveFilesActions().DECOMPRESSED_HERE);
    compressToolBar->setOrientation(Qt::Orientation::Vertical);
    compressToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    compressToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    compressToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(compressToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* renameItemsTB = new MenuToolButton(g_renameAg().RENAME_RIBBONS->actions(), QToolButton::MenuButtonPopup,
                                           Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_48, leafHomeWid);
  renameItemsTB->InitDefaultActionFromQSetting(MemoryKey::DEFAULT_RENAME_CHOICE, true);

  QToolBar* advanceSearchToolBar = new (std::nothrow) QToolBar("AdvanceSearch");
  CHECK_NULLPTR_RETURN_NULLPTR(advanceSearchToolBar);
  {
    advanceSearchToolBar->addAction(g_viewActions()._ADVANCE_SEARCH_VIEW);
    advanceSearchToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    advanceSearchToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  }

  leafHomeWid->setToolTip("Home Leaf ToolBar");
  leafHomeWid->addWidget(openItemsTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(propertiesTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(folderOpModeTb);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(cutCopyPasterTb);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(moveCopyItemsToTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(recycleItemsTB);
  leafHomeWid->addWidget(renameItemsTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(newItemsTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(selectionToolBar);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(archievePreviewToolBar);
  leafHomeWid->addWidget(compressToolBar);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(advanceSearchToolBar);
  return leafHomeWid;
}

QToolBar* RibbonMenu::LeafView() const {
  auto* leafViewWid = new (std::nothrow) QToolBar{"Leaf View"};
  CHECK_NULLPTR_RETURN_NULLPTR(leafViewWid);

  auto* folderPreviewToolBar = g_folderPreviewActions().GetPreviewsToolbar(leafViewWid);
  CHECK_NULLPTR_RETURN_NULLPTR(folderPreviewToolBar);
  SetLayoutAlightment(folderPreviewToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto& viewInst = g_viewActions();

  leafViewWid->setToolTip("View Leaf");
  leafViewWid->addAction(viewInst.NAVIGATION_PANE);
  leafViewWid->addWidget(folderPreviewToolBar);
  leafViewWid->addSeparator();
  leafViewWid->addActions(viewInst._VIEWS_NAVIGATE);
  leafViewWid->addSeparator();
  leafViewWid->addActions(viewInst._ALL_VIEWS);
  leafViewWid->addSeparator();
  leafViewWid->addAction(viewInst._HAR_VIEW);
  leafViewWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafViewWid;
}

QToolBar* RibbonMenu::LeafMovie() const {
  auto* databaseToolBar{new (std::nothrow) RibbonMovieDB{"Leaf Movie Database"}};
  CHECK_NULLPTR_RETURN_NULLPTR(databaseToolBar);
  return databaseToolBar;
}

QToolBar* RibbonMenu::LeafCast() const {
  auto* castToolBar{new (std::nothrow) RibbonCastDB{"Leaf Cast Database"}};
  CHECK_NULLPTR_RETURN_NULLPTR(castToolBar);
  return castToolBar;
}

QToolBar* RibbonMenu::LeafJson() const {
  auto* jsonToolBar{new (std::nothrow) RibbonJson{"Leaf Json"}};
  CHECK_NULLPTR_RETURN_NULLPTR(jsonToolBar);
  return jsonToolBar;
}

QToolBar* RibbonMenu::LeafScenesTools() const {
  auto& ag = g_SceneInPageActions();

  auto* sceneTB = new (std::nothrow) QToolBar("scene toolbar");
  CHECK_NULLPTR_RETURN_NULLPTR(sceneTB);

  QToolBar* orderTB = ag.GetOrderToolBar(sceneTB);
  CHECK_NULLPTR_RETURN_NULLPTR(orderTB);

  sceneTB->addAction(g_viewActions()._SCENE_VIEW);
  sceneTB->addSeparator();
  sceneTB->addAction(ag._COMBINE_MEDIAINFOS_JSON);
  sceneTB->addSeparator();
  sceneTB->addWidget(orderTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(m_scenePageControl);
  sceneTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return sceneTB;
}

QToolBar* RibbonMenu::LeafMediaTools() const {
  const auto& fileOpAgInst = FileOpActs::GetInst();

  QToolBar* folderRmv{new (std::nothrow) QToolBar{"Folder Remover"}};
  CHECK_NULLPTR_RETURN_NULLPTR(folderRmv);
  folderRmv->setOrientation(Qt::Orientation::Vertical);
  folderRmv->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  folderRmv->setStyleSheet("QToolBar { max-width: 256px; }");
  folderRmv->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  folderRmv->addAction(fileOpAgInst._RMV_EMPTY_FOLDER);
  folderRmv->addAction(fileOpAgInst._RMV_01_FILE_FOLDER);
  folderRmv->addAction(fileOpAgInst._RMV_FOLDER_BY_KEYWORD);
  SetLayoutAlightment(folderRmv->layout(), Qt::AlignmentFlag::AlignLeft);

  QToolBar* mediaDupFinder{new (std::nothrow) QToolBar{"Duplicate Media Finder"}};
  CHECK_NULLPTR_RETURN_NULLPTR(mediaDupFinder);
  mediaDupFinder->setOrientation(Qt::Orientation::Vertical);
  mediaDupFinder->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mediaDupFinder->setStyleSheet("QToolBar { max-width: 256px; }");
  mediaDupFinder->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  mediaDupFinder->addAction(fileOpAgInst._DUPLICATE_IMAGES_FINDER);
  mediaDupFinder->addAction(fileOpAgInst._LOW_RESOLUTION_IMGS_RMV);
  mediaDupFinder->addAction(fileOpAgInst._DUPLICATE_VIDEOS_FINDER);
  SetLayoutAlightment(mediaDupFinder->layout(), Qt::AlignmentFlag::AlignLeft);

  auto& arrangeIns = g_NameRulerActions();
  QToolButton* nameRulerToolButton =
      new (std::nothrow) MenuToolButton(arrangeIns.NAME_RULES_ACTIONS_LIST, QToolButton::MenuButtonPopup,
                                        Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16);
  CHECK_NULLPTR_RETURN_NULLPTR(nameRulerToolButton);
  nameRulerToolButton->setDefaultAction(arrangeIns._NAME_RULER);

  auto& thumbInst = g_ThumbnailProcessActions();
  auto* thumbnailTb = thumbInst.GetThumbnailToolbar();

  QToolBar* archiveVidsTB{new (std::nothrow) QToolBar{"Leaf Arrange Files"}};
  CHECK_NULLPTR_RETURN_NULLPTR(archiveVidsTB);
  archiveVidsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  archiveVidsTB->addWidget(nameRulerToolButton);
  archiveVidsTB->addAction(fileOpAgInst._PACK_FOLDERS);
  archiveVidsTB->addAction(fileOpAgInst._UNPACK_FOLDERS);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addAction(fileOpAgInst._RETURN_ERROR_CODE_UPON_ANY_FAILURE);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addAction(fileOpAgInst._LONG_PATH_FINDER);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(folderRmv);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(mediaDupFinder);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(thumbnailTb);
  archiveVidsTB->addAction(fileOpAgInst._TS_FILES_MERGE);
  return archiveVidsTB;
}

void RibbonMenu::Subscribe() {
  connect(_EXPAND_RIBBONS, &QAction::toggled, this, &RibbonMenu::on_expandStackedWidget);
  connect(this, &QTabWidget::currentChanged, this, &RibbonMenu::on_currentTabChangedRecordIndex);
}

void RibbonMenu::AfterSubscribeInitialSettings() {
  setCurrentIndex(Configuration().value(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.v).toInt());
  on_expandStackedWidget(_EXPAND_RIBBONS->isChecked());
}

#include <QPropertyAnimation>
void RibbonMenu::on_expandStackedWidget(const bool vis) {
  Configuration().setValue(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, vis);
#ifdef RUNNING_UNIT_TESTS  // no need animation in testcase
  setMaximumHeight(vis ? sizeHint().height() : tabBar()->height());
#else
  QPropertyAnimation* animation = new (std::nothrow) QPropertyAnimation{this, "maximumHeight", this};
  if (animation == nullptr) {
    setMaximumHeight(vis ? sizeHint().height() : tabBar()->height());
    return;
  }
  animation->setDuration(200);
  animation->setStartValue(maximumHeight());
  animation->setEndValue(vis ? sizeHint().height() : tabBar()->height());
  animation->start(QAbstractAnimation::DeleteWhenStopped);
#endif
}

void RibbonMenu::on_currentTabChangedRecordIndex(const int tabIndex) {
  Configuration().setValue(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, tabIndex);
}

void RibbonMenu::whenViewTypeChanged(ViewTypeTool::ViewType vt) {
  const int destLeafTabIndex = mViewType2LeafTabIndex[(int)vt];
  if (destLeafTabIndex == -1) {
    return;
  }
  setCurrentIndex(destLeafTabIndex);
}
