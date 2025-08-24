#include "RibbonMenu.h"
#include <QMenu>
#include <QTabBar>
#include <QToolButton>
#include "ActionsRecorder.h"
#include "ActionsSearcher.h"
#include "ArchiveFilesActions.h"
#include "ArrangeActions.h"
#include "FileBasicOperationsActions.h"
#include "FileLeafAction.h"
#include "FolderPreviewActions.h"
#include "FramelessWindowActions.h"
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
#include "DropListToolButton.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

RibbonMenu::RibbonMenu(QWidget* parent)
    : QTabWidget{parent}  //
{
  m_leafFile = LeafFile();
  m_leafHome = LeafHome();
  m_leafView = LeafView();
  m_leafDatabase = LeafDatabase();
  m_leafCast = LeafCast();
  m_leafJson = LeafJson();
  m_leafScenes = LeafScenesTools();
  m_leafMedia = LeafMediaTools();

  const auto& viewIns = g_viewActions();

  addTab(m_leafFile, "&FILE");
  addTab(m_leafHome, "&HOME");
  addTab(m_leafView, "&VIEW");
  addTab(m_leafDatabase, "&" + viewIns._MOVIE_VIEW->text());
  addTab(m_leafCast, "&" + viewIns._CAST_VIEW->text());
  addTab(m_leafJson, "&" + viewIns._JSON_VIEW->text());
  addTab(m_leafScenes, "&" + viewIns._SCENE_VIEW->text());
  addTab(m_leafMedia, "&ARRANGE");

  auto& inst = ActionsRecorder::GetInst();
  inst.FromToolbar(m_leafFile);
  inst.FromToolbar(m_leafView);
  inst.FromToolbar(m_leafMedia);

  m_corner = GetMenuRibbonCornerWid();
  setCornerWidget(m_corner, Qt::Corner::TopRightCorner);

  Subscribe();
  setCurrentIndex(Configuration().value(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.v).toInt());
}

QToolBar* RibbonMenu::GetMenuRibbonCornerWid(QWidget* attached) {
  QToolBar* menuRibbonCornerWid{new (std::nothrow) QToolBar{"corner tools", attached}};
  CHECK_NULLPTR_RETURN_NULLPTR(menuRibbonCornerWid);
  ActionsSearcher* mActSearcher{new (std::nothrow) ActionsSearcher{menuRibbonCornerWid}};
  CHECK_NULLPTR_RETURN_NULLPTR(mActSearcher);
  menuRibbonCornerWid->addWidget(mActSearcher);
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_rightClickActions()._SEARCH_IN_NET_EXPLORER);
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_LogActions()._LOG_FILE);
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_framelessWindowAg()._EXPAND_RIBBONS);
  menuRibbonCornerWid->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
  return menuRibbonCornerWid;
}

QToolBar* RibbonMenu::LeafFile() const {
  QToolBar* syncSwitchToolBar = g_syncFileSystemModificationActions().GetSyncSwitchToolbar();
  QToolBar* syncPathToolBar = g_syncFileSystemModificationActions().GetSyncPathToolbar();
  SetLayoutAlightment(syncPathToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* styleToolButton =
      new (std::nothrow) DropdownToolButton(g_PreferenceActions().PREFERENCE_LIST, QToolButton::InstantPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16);
  styleToolButton->SetCaption(QIcon{":img/STYLE_SETTING"}, "Change Style");
  QToolButton* logToolButton =
      new (std::nothrow) DropdownToolButton(g_LogActions()._DROPDOWN_LIST, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16);
  logToolButton->setDefaultAction(g_LogActions()._LOG_FILE);

  QToolBar* leafFileWid{new (std::nothrow) QToolBar};
  CHECK_NULLPTR_RETURN_NULLPTR(leafFileWid);
  leafFileWid->addActions(g_fileLeafActions()._LEAF_FILE->actions());
  leafFileWid->addSeparator();
  leafFileWid->addWidget(syncSwitchToolBar);
  leafFileWid->addWidget(syncPathToolBar);
  leafFileWid->addSeparator();
  leafFileWid->addWidget(styleToolButton);
  leafFileWid->addSeparator();
  leafFileWid->addWidget(logToolButton);
  leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafFileWid;
}

QToolBar* RibbonMenu::LeafHome() const {
  QToolBar* openItemsTB = new (std::nothrow) QToolBar("Open");
  CHECK_NULLPTR_RETURN_NULLPTR(openItemsTB);
  {
    openItemsTB->addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
    openItemsTB->addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
    openItemsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    openItemsTB->setOrientation(Qt::Orientation::Vertical);
    openItemsTB->setStyleSheet("QToolBar { max-width: 256px; }");
    openItemsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    SetLayoutAlightment(openItemsTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* copyTB = new DropdownToolButton(g_fileBasicOperationsActions().COPY_PATH_AG->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  copyTB->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_COPY_CHOICE.name, MemoryKey::DEFAULT_COPY_CHOICE.v).toString());
  copyTB->MemorizeCurrentAction(MemoryKey::DEFAULT_COPY_CHOICE.name);

  QToolBar* propertiesTB = new (std::nothrow) QToolBar("Properties");
  CHECK_NULLPTR_RETURN_NULLPTR(propertiesTB);
  {
    propertiesTB->addWidget(copyTB);
    propertiesTB->addAction(g_rightClickActions()._CALC_MD5_ACT);
    propertiesTB->addAction(g_rightClickActions()._PROPERTIES);
    propertiesTB->setOrientation(Qt::Orientation::Vertical);
    propertiesTB->setStyleSheet("QToolBar { max-width: 256px; }");
    propertiesTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    propertiesTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    SetLayoutAlightment(propertiesTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* newItemsTB = new (std::nothrow) DropdownToolButton(g_fileBasicOperationsActions().NEW->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  newItemsTB->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_NEW_CHOICE.name, MemoryKey::DEFAULT_NEW_CHOICE.v).toString());
  newItemsTB->MemorizeCurrentAction(MemoryKey::DEFAULT_NEW_CHOICE.name);

  QToolBar* moveCopyItemsToTB = new (std::nothrow) QToolBar("Move/Copy item(s) To ToolBar");
  CHECK_NULLPTR_RETURN_NULLPTR(moveCopyItemsToTB);
  {
    const auto& _MOVE_TO_HIST_LIST = g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY->actions();
    const auto& _COPY_TO_HIST_LIST = g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY->actions();
    auto* pMoveToToolButton =
        new DropdownToolButton(_MOVE_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_24);
    pMoveToToolButton->setDefaultAction(g_fileBasicOperationsActions()._MOVE_TO);
    auto* pCopyToToolButton =
        new DropdownToolButton(_COPY_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_24);
    pCopyToToolButton->setDefaultAction(g_fileBasicOperationsActions()._COPY_TO);

    moveCopyItemsToTB->addWidget(pMoveToToolButton);
    moveCopyItemsToTB->addWidget(pCopyToToolButton);
    moveCopyItemsToTB->setOrientation(Qt::Orientation::Horizontal);
    SetLayoutAlightment(moveCopyItemsToTB->layout(), Qt::AlignmentFlag::AlignTop);
  }

  QToolBar* cutCopyPasterTb = g_fileBasicOperationsActions().GetCutCopyPasteTb();
  QToolBar* folderOpModeTb = g_fileBasicOperationsActions().GetFolderOperationModeTb();

  QToolButton* recycleItemsTB = new (std::nothrow) DropdownToolButton(g_fileBasicOperationsActions().DELETE_ACTIONS->actions(), QToolButton::MenuButtonPopup);
  recycleItemsTB->setDefaultAction(g_fileBasicOperationsActions().MOVE_TO_TRASHBIN);

  QToolBar* archievePreviewToolBar = new (std::nothrow) QToolBar{"ArchievePreview"};
  CHECK_NULLPTR_RETURN_NULLPTR(archievePreviewToolBar);
  archievePreviewToolBar->addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  archievePreviewToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  archievePreviewToolBar->setStyleSheet("QToolBar { max-width: 256px; }");

  QToolBar* selectionToolBar = new (std::nothrow) QToolBar{"Selection"};
  CHECK_NULLPTR_RETURN_NULLPTR(selectionToolBar);
  {
    selectionToolBar->addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
    selectionToolBar->setOrientation(Qt::Orientation::Vertical);
    selectionToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    selectionToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_16, IMAGE_SIZE::TABS_ICON_IN_MENU_16));
    selectionToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(selectionToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* compressToolBar = new (std::nothrow) QToolBar{"Compress/Decompress"};
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

  auto* renameItemsTB = new DropdownToolButton(g_renameAg().RENAME_RIBBONS->actions(), QToolButton::MenuButtonPopup);
  renameItemsTB->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_RENAME_CHOICE.name, MemoryKey::DEFAULT_RENAME_CHOICE.v).toString());
  renameItemsTB->MemorizeCurrentAction(MemoryKey::DEFAULT_RENAME_CHOICE.name);
  QToolBar* advanceSearchToolBar = new (std::nothrow) QToolBar("AdvanceSearch");
  CHECK_NULLPTR_RETURN_NULLPTR(advanceSearchToolBar);
  advanceSearchToolBar->addAction(g_viewActions()._ADVANCE_SEARCH_VIEW);
  advanceSearchToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  advanceSearchToolBar->setStyleSheet("QToolBar { max-width: 256px; }");

  QToolBar* leafHomeWid = new (std::nothrow) QToolBar{"LeafHome"};
  CHECK_NULLPTR_RETURN_NULLPTR(leafHomeWid);
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
  auto* leafViewWid = new (std::nothrow) QToolBar("Leaf View");
  CHECK_NULLPTR_RETURN_NULLPTR(leafViewWid);

  auto* fileSystemView = g_viewActions().GetFileSystemViewTB(leafViewWid);
  CHECK_NULLPTR_RETURN_NULLPTR(fileSystemView);

  auto* folderPreviewToolBar = g_folderPreviewActions().GetPreviewsToolbar(leafViewWid);
  CHECK_NULLPTR_RETURN_NULLPTR(folderPreviewToolBar);
  SetLayoutAlightment(folderPreviewToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  leafViewWid->setToolTip("View Leaf");
  leafViewWid->addAction(g_viewActions().NAVIGATION_PANE);
  leafViewWid->addWidget(fileSystemView);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(folderPreviewToolBar);
  leafViewWid->addSeparator();
  leafViewWid->addAction(g_viewActions()._HAR_VIEW);
  leafViewWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafViewWid;
}

QToolBar* RibbonMenu::LeafDatabase() const {
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

QToolBar* RibbonMenu::LeafMediaTools() const {
  const auto& fileOpAgInst = g_fileBasicOperationsActions();

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

  auto& arrangeIns = g_ArrangeActions();
  QList<QAction*> studiosActions{arrangeIns._EDIT_STUDIOS,  arrangeIns._RELOAD_STUDIOS,  nullptr, arrangeIns._EDIT_PERFS,      arrangeIns._RELOAD_PERFS, nullptr,
                                 arrangeIns._EDIT_PERF_AKA, arrangeIns._RELOAD_PERF_AKA, nullptr, arrangeIns._RENAME_RULE_STAT};
  QToolButton* nameRulerToolButton =
      new (std::nothrow) DropdownToolButton(studiosActions, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16);
  CHECK_NULLPTR_RETURN_NULLPTR(nameRulerToolButton);
  nameRulerToolButton->setDefaultAction(fileOpAgInst._NAME_RULER);

  auto& thumbnailIns = g_ThumbnailProcessActions();
  QList<QAction*> crtThumbnailActions;
  crtThumbnailActions += thumbnailIns._CREATE_THUMBNAIL_AG->actions();
  crtThumbnailActions.push_back(nullptr);
  crtThumbnailActions.push_back(thumbnailIns._THUMBNAIL_SAMPLE_PERIOD);
  auto* createThumbnailToolButton =
      new (std::nothrow) DropdownToolButton{crtThumbnailActions, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16};
  CHECK_NULLPTR_RETURN_NULLPTR(createThumbnailToolButton);
  createThumbnailToolButton->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_THUMBNAILS_DIMENSION.name, MemoryKey::DEFAULT_THUMBNAILS_DIMENSION.v).toString());
  createThumbnailToolButton->MemorizeCurrentAction(MemoryKey::DEFAULT_THUMBNAILS_DIMENSION.name);

  QList<QAction*> extractThumbnailActions;
  extractThumbnailActions += thumbnailIns._EXTRACT_THUMBNAIL_AG->actions();
  extractThumbnailActions.push_back(nullptr);
  extractThumbnailActions.push_back(thumbnailIns._CUSTOM_RANGE_IMGS);
  extractThumbnailActions.push_back(nullptr);
  extractThumbnailActions.push_back(thumbnailIns._SKIP_IF_ALREADY_EXIST);
  auto* extractThumbnailToolButton =
      new (std::nothrow) DropdownToolButton{extractThumbnailActions, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_16};
  CHECK_NULLPTR_RETURN_NULLPTR(extractThumbnailToolButton);
  extractThumbnailToolButton->FindAndSetDefaultAction(Configuration().value(MemoryKey::DEFAULT_EXTRACT_CHOICE.name, MemoryKey::DEFAULT_EXTRACT_CHOICE.v).toString());
  extractThumbnailToolButton->MemorizeCurrentAction(MemoryKey::DEFAULT_EXTRACT_CHOICE.name);

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
  archiveVidsTB->addWidget(createThumbnailToolButton);
  archiveVidsTB->addWidget(extractThumbnailToolButton);
  return archiveVidsTB;
}

QToolBar* RibbonMenu::LeafScenesTools() const {
  auto& ag = g_SceneInPageActions();
  if (!ag.InitWidget()) {
    return nullptr;
  }

  auto* sceneTB = new (std::nothrow) QToolBar("scene toolbar");
  sceneTB->addAction(g_viewActions()._SCENE_VIEW);
  sceneTB->addSeparator();
  sceneTB->addAction(ag._COMBINE_MEDIAINFOS_JSON);
  sceneTB->addAction(ag._UPDATE_SCN_ONLY);
  sceneTB->addSeparator();
  sceneTB->addWidget(ag.mOrderTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(ag.mEnablePageTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(ag.mImageSizeTB);
  sceneTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return sceneTB;
}

void RibbonMenu::Subscribe() {
  connect(g_framelessWindowAg()._EXPAND_RIBBONS, &QAction::triggered, this, &RibbonMenu::on_officeStyleWidgetVisibilityChanged);
  on_officeStyleWidgetVisibilityChanged(g_framelessWindowAg()._EXPAND_RIBBONS->isChecked());
  connect(this, &QTabWidget::currentChanged, this, &RibbonMenu::on_currentTabChangedRecordIndex);
}

void RibbonMenu::on_officeStyleWidgetVisibilityChanged(const bool vis) {
  Configuration().setValue(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, vis);
  setMaximumHeight(vis ? RibbonMenu::MAX_WIDGET_HEIGHT : tabBar()->height());
}

void RibbonMenu::on_currentTabChangedRecordIndex(const int tabIndex) {
  Configuration().setValue(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, tabIndex);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

#include <QMainWindow>
#include <QToolBar>

class RibbonMenuIllu : public QMainWindow {
 public:
  explicit RibbonMenuIllu(QWidget* parent = nullptr) : QMainWindow(parent) {
    setWindowFlag(Qt::WindowType::WindowStaysOnTopHint);

    RibbonMenu* osm = new RibbonMenu;
    setMenuWidget(osm);
    QToolBar* tb = new QToolBar("tb");
    tb->addAction("Here is Toolbar");

    addToolBar(Qt::ToolBarArea::TopToolBarArea, tb);
    setWindowTitle("Ribbon Menu");
    setMinimumWidth(1024);
  }
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RibbonMenuIllu ribbonMenuExample(nullptr);
  ribbonMenuExample.show();
  return a.exec();
}
#endif
