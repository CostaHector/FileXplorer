#include "Component/RibbonMenu.h"
#include <QMenu>
#include <QTabBar>
#include <QToolButton>
#include "Actions/ArchiveFilesActions.h"
#include "Actions/ArrangeActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/FileLeafAction.h"
#include "Actions/FolderPreviewActions.h"
#include "Actions/FramelessWindowActions.h"
#include "Actions/JsonEditorActions.h"
#include "Actions/PreferenceActions.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
#include "Actions/SceneInPageActions.h"
#include "Actions/SyncFileSystemModificationActions.h"
#include "Actions/VideoPlayerActions.h"
#include "Actions/ViewActions.h"
#include "Actions/ThumbnailProcessActions.h"
#include "Actions/LogActions.h"
#include "Component/DatabaseToolBar.h"
#include "Component/DropListToolButton.h"
#include "public/PublicTool.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"

RibbonMenu::RibbonMenu(QWidget* parent)
    : QTabWidget{parent},
      m_corner(GetMenuRibbonCornerWid()),
      m_leafFile(LeafFile()),
      m_leafHome(LeafHome()),
      m_leafView(LeafView()),
      m_leafDatabase(LeafDatabase()),
      m_leafScenes(LeafScenesTools()),
      m_leafMedia(LeafMediaTools()) {
  addTab(m_leafFile, "&File");
  addTab(m_leafHome, "&Home");
  addTab(m_leafView, "&View");
  addTab(m_leafDatabase, "&Database");
  addTab(m_leafScenes, "&Scene");
  addTab(m_leafMedia, "&Arrange");

  setCornerWidget(m_corner, Qt::Corner::TopRightCorner);

  Subscribe();

  setCurrentIndex(PreferenceSettings().value(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.v).toInt());
}

QToolBar* RibbonMenu::GetMenuRibbonCornerWid(QWidget* attached) {
  QToolBar* menuRibbonCornerWid{new (std::nothrow) QToolBar("corner tools", attached)};
  CHECK_NULLPTR_RETURN_NULLPTR(menuRibbonCornerWid);
  menuRibbonCornerWid->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_framelessWindowAg()._EXPAND_RIBBONS);
  menuRibbonCornerWid->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
  return menuRibbonCornerWid;
}

QToolBar* RibbonMenu::LeafFile() const {
  auto* styleToolButton = new DropdownToolButton(g_PreferenceActions().PREFERENCE_LIST, QToolButton::InstantPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1);
  styleToolButton->SetCaption(QIcon{":img/STYLE_SETTING"}, "style");
  QToolButton* logToolButton = new DropdownToolButton(g_LogActions()._DROPDOWN_LIST, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1);
  logToolButton->setDefaultAction(g_LogActions()._LOG_FILE);

  QToolBar* leafFileWid{new (std::nothrow) QToolBar};
  CHECK_NULLPTR_RETURN_NULLPTR(leafFileWid);
  leafFileWid->addActions(g_fileLeafActions()._LEAF_FILE->actions());
  leafFileWid->addSeparator();
  leafFileWid->addWidget(styleToolButton);
  leafFileWid->addSeparator();
  leafFileWid->addWidget(logToolButton);
  leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  return leafFileWid;
}

QToolBar* RibbonMenu::LeafHome() const {
  auto* playTB = new DropdownToolButton(g_viewActions()._VIDEO_PLAYERS->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  playTB->FindAndSetDefaultAction(PreferenceSettings().value(MemoryKey::DEFAULT_VIDEO_PLAYER.name, MemoryKey::DEFAULT_VIDEO_PLAYER.v).toString());
  playTB->MemorizeCurrentAction(MemoryKey::DEFAULT_VIDEO_PLAYER.name);

  QToolBar* openItemsTB = new (std::nothrow) QToolBar("Open");
  CHECK_NULLPTR_RETURN_NULLPTR(openItemsTB);
  {
    openItemsTB->addWidget(playTB);
    openItemsTB->addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
    openItemsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    openItemsTB->setOrientation(Qt::Orientation::Vertical);
    openItemsTB->setStyleSheet("QToolBar { max-width: 256px; }");
    openItemsTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
    SetLayoutAlightment(openItemsTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* copyTB = new DropdownToolButton(g_fileBasicOperationsActions().COPY_PATH_AG->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  copyTB->FindAndSetDefaultAction(PreferenceSettings().value(MemoryKey::DEFAULT_COPY_CHOICE.name, MemoryKey::DEFAULT_COPY_CHOICE.v).toString());
  copyTB->MemorizeCurrentAction(MemoryKey::DEFAULT_COPY_CHOICE.name);

  QToolBar* propertiesTB = new (std::nothrow) QToolBar("Properties");
  CHECK_NULLPTR_RETURN_NULLPTR(propertiesTB);
  {
    propertiesTB->addWidget(copyTB);
    propertiesTB->addAction(g_rightClickActions()._CALC_MD5_ACT);
    propertiesTB->addAction(g_rightClickActions()._PROPERTIES);
    propertiesTB->setOrientation(Qt::Orientation::Vertical);
    propertiesTB->setStyleSheet("QToolBar { max-width: 256px; }");
    propertiesTB->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
    propertiesTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    SetLayoutAlightment(propertiesTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* newItemsTB = new DropdownToolButton(g_fileBasicOperationsActions().NEW->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  newItemsTB->FindAndSetDefaultAction(PreferenceSettings().value(MemoryKey::DEFAULT_NEW_CHOICE.name, MemoryKey::DEFAULT_NEW_CHOICE.v).toString());
  newItemsTB->MemorizeCurrentAction(MemoryKey::DEFAULT_NEW_CHOICE.name);

  QToolBar* moveCopyItemsToTB = new (std::nothrow) QToolBar("Move/Copy item(s) To ToolBar");
  CHECK_NULLPTR_RETURN_NULLPTR(moveCopyItemsToTB);
  {
    const auto& _MOVE_TO_HIST_LIST = g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY->actions();
    const auto& _COPY_TO_HIST_LIST = g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY->actions();
    auto* pMoveToToolButton =
        new DropdownToolButton(_MOVE_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_2x1);
    pMoveToToolButton->setDefaultAction(g_fileBasicOperationsActions()._MOVE_TO);
    auto* pCopyToToolButton =
        new DropdownToolButton(_COPY_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_2x1);
    pCopyToToolButton->setDefaultAction(g_fileBasicOperationsActions()._COPY_TO);

    moveCopyItemsToTB->addWidget(pMoveToToolButton);
    moveCopyItemsToTB->addWidget(pCopyToToolButton);
    moveCopyItemsToTB->setOrientation(Qt::Orientation::Horizontal);
    SetLayoutAlightment(moveCopyItemsToTB->layout(), Qt::AlignmentFlag::AlignTop);
  }

  QToolButton* recycleItemsTB = new DropdownToolButton(g_fileBasicOperationsActions().DELETE_ACTIONS->actions(), QToolButton::MenuButtonPopup);
  recycleItemsTB->setDefaultAction(g_fileBasicOperationsActions().MOVE_TO_TRASHBIN);

  QToolBar* archievePreviewToolBar = new (std::nothrow) QToolBar("ArchievePreview");
  CHECK_NULLPTR_RETURN_NULLPTR(archievePreviewToolBar);
  archievePreviewToolBar->addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  archievePreviewToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  archievePreviewToolBar->setStyleSheet("QToolBar { max-width: 256px; }");

  QToolBar* selectionToolBar = new (std::nothrow) QToolBar("Selection");
  CHECK_NULLPTR_RETURN_NULLPTR(selectionToolBar);
  {
    selectionToolBar->addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
    selectionToolBar->setOrientation(Qt::Orientation::Vertical);
    selectionToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    selectionToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
    selectionToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(selectionToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* compressToolBar = new (std::nothrow) QToolBar("Compress/Decompress");
  CHECK_NULLPTR_RETURN_NULLPTR(compressToolBar);
  {
    compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_HERE);
    compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_IMAGES);
    compressToolBar->addAction(g_AchiveFilesActions().DECOMPRESSED_HERE);
    compressToolBar->setOrientation(Qt::Orientation::Vertical);
    compressToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    compressToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
    compressToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(compressToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  auto* renameItemsTB = new DropdownToolButton(g_renameAg().RENAME_RIBBONS->actions(), QToolButton::MenuButtonPopup);
  renameItemsTB->FindAndSetDefaultAction(PreferenceSettings().value(MemoryKey::DEFAULT_RENAME_CHOICE.name, MemoryKey::DEFAULT_RENAME_CHOICE.v).toString());
  renameItemsTB->MemorizeCurrentAction(MemoryKey::DEFAULT_RENAME_CHOICE.name);
  QToolBar* advanceSearchToolBar = new (std::nothrow) QToolBar("AdvanceSearch");
  CHECK_NULLPTR_RETURN_NULLPTR(advanceSearchToolBar);
  advanceSearchToolBar->addAction(g_viewActions()._ADVANCE_SEARCH_VIEW);
  advanceSearchToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  advanceSearchToolBar->setStyleSheet("QToolBar { max-width: 256px; }");

  QToolBar* syncSwitchToolBar = g_syncFileSystemModificationActions().GetSyncSwitchToolbar();
  {
    syncSwitchToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    syncSwitchToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  }
  QToolBar* syncPathToolBar = g_syncFileSystemModificationActions().GetSyncPathToolbar();
  {
    syncPathToolBar->setOrientation(Qt::Orientation::Vertical);
    syncPathToolBar->setStyleSheet("QToolBar { max-width: 512px; }");
    syncPathToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
    syncPathToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    SetLayoutAlightment(syncPathToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* leafHomeWid = new (std::nothrow) QToolBar("LeafHome");
  CHECK_NULLPTR_RETURN_NULLPTR(leafHomeWid);
  leafHomeWid->setToolTip("Home Leaf ToolBar");
  leafHomeWid->addWidget(openItemsTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(propertiesTB);
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
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(syncSwitchToolBar);
  leafHomeWid->addWidget(syncPathToolBar);
  return leafHomeWid;
}

QToolBar* RibbonMenu::LeafView() const {
  auto* leafViewWid = new (std::nothrow) QToolBar("Leaf View");
  CHECK_NULLPTR_RETURN_NULLPTR(leafViewWid);

  auto* fileSystemView = new (std::nothrow) QToolBar("Navigation Preview Switch");
  CHECK_NULLPTR_RETURN_NULLPTR(fileSystemView);
  fileSystemView->setOrientation(Qt::Orientation::Vertical);
  fileSystemView->addActions({g_viewActions()._LIST_VIEW, g_viewActions()._TABLE_VIEW, g_viewActions()._TREE_VIEW});
  fileSystemView->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  fileSystemView->setStyleSheet("QToolBar { max-width: 256px; }");
  fileSystemView->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
  SetLayoutAlightment(fileSystemView->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* folderPreviewToolBar = g_folderPreviewActions().GetPreviewsToolbar(leafViewWid);
  folderPreviewToolBar->setOrientation(Qt::Orientation::Vertical);
  folderPreviewToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  folderPreviewToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  folderPreviewToolBar->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
  SetLayoutAlightment(folderPreviewToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* embeddedPlayerTB = new DropdownToolButton(g_videoPlayerActions()._BATCH_VIDEO_ACTIONS->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  embeddedPlayerTB->setDefaultAction(g_viewActions()._VIDEO_PLAYER_EMBEDDED);
  leafViewWid->setToolTip("View Leaf");
  leafViewWid->addAction(g_viewActions().NAVIGATION_PANE);
  leafViewWid->addWidget(fileSystemView);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(folderPreviewToolBar);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(embeddedPlayerTB);
  leafViewWid->addSeparator();
  leafViewWid->addAction(g_viewActions()._HAR_VIEW);
  leafViewWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafViewWid;
}

QToolBar* RibbonMenu::LeafDatabase() const {
  auto* databaseToolBar{new (std::nothrow) DatabaseToolBar("Leaf Database")};
  CHECK_NULLPTR_RETURN_NULLPTR(databaseToolBar);
  return databaseToolBar;
}

QToolBar* RibbonMenu::LeafMediaTools() const {
  QToolBar* folderRmv{new (std::nothrow) QToolBar{"Folder Remover"}};
  CHECK_NULLPTR_RETURN_NULLPTR(folderRmv);
  folderRmv->setOrientation(Qt::Orientation::Vertical);
  folderRmv->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  folderRmv->setStyleSheet("QToolBar { max-width: 256px; }");
  folderRmv->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
  folderRmv->addAction(g_fileBasicOperationsActions()._RMV_REDUN_PARENT_FOLDER);
  folderRmv->addAction(g_fileBasicOperationsActions()._RMV_EMPTY_FOLDER_R);
  folderRmv->addAction(g_fileBasicOperationsActions()._RMV_FOLDER_BY_KEYWORD);

  QToolBar* mediaDupFinder{new (std::nothrow) QToolBar{"Duplicate Medias Finder"}};
  CHECK_NULLPTR_RETURN_NULLPTR(mediaDupFinder);
  mediaDupFinder->setOrientation(Qt::Orientation::Vertical);
  mediaDupFinder->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mediaDupFinder->setStyleSheet("QToolBar { max-width: 256px; }");
  mediaDupFinder->setIconSize(QSize(IMAGE_SIZE::TABS_ICON_IN_MENU_3x1, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1));
  mediaDupFinder->addAction(g_fileBasicOperationsActions()._DUPLICATE_ITEMS_REMOVER);
  mediaDupFinder->addAction(g_fileBasicOperationsActions()._REDUNDANT_IMAGES_FINDER);
  mediaDupFinder->addAction(g_fileBasicOperationsActions()._DUPLICATE_VIDEOS_FINDER);

  auto& arrangeIns = g_ArrangeActions();
  QList<QAction*> studiosActions{arrangeIns._EDIT_STUDIOS,  arrangeIns._RELOAD_STUDIOS,  nullptr, arrangeIns._EDIT_PERFS,      arrangeIns._RELOAD_PERFS, nullptr,
                                 arrangeIns._EDIT_PERF_AKA, arrangeIns._RELOAD_PERF_AKA, nullptr, arrangeIns._RENAME_RULE_STAT};
  QToolButton* nameRulerToolButton =
      new (std::nothrow) DropdownToolButton(studiosActions, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1);
  CHECK_NULLPTR_RETURN_NULLPTR(nameRulerToolButton);
  nameRulerToolButton->setDefaultAction(g_fileBasicOperationsActions()._NAME_RULER);
  auto& viewIns = g_viewActions();
  auto& jsonIns = g_jsonEditorActions();
  auto* jsonEditorTB = new (std::nothrow) DropdownToolButton(jsonIns._BATCH_EDIT_TOOL_ACTIONS->actions(), QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  CHECK_NULLPTR_RETURN_NULLPTR(jsonEditorTB);
  jsonEditorTB->setDefaultAction(viewIns._JSON_EDITOR_PANE);

  auto& thumbnailIns = g_ThumbnailProcessActions();
  QList<QAction*> thumbnailActions{thumbnailIns._EXTRACT_1ST_IMG,      thumbnailIns._EXTRACT_2ND_IMGS, thumbnailIns._EXTRACT_4TH_IMGS, nullptr, thumbnailIns._CUSTOM_RANGE_IMGS, nullptr,
                                   thumbnailIns._SKIP_IF_ALREADY_EXIST};
  auto* thumbnailToolButton = new (std::nothrow) DropdownToolButton(thumbnailActions, QToolButton::MenuButtonPopup, Qt::ToolButtonStyle::ToolButtonTextUnderIcon, IMAGE_SIZE::TABS_ICON_IN_MENU_3x1);
  CHECK_NULLPTR_RETURN_NULLPTR(thumbnailToolButton);
  thumbnailToolButton->FindAndSetDefaultAction(PreferenceSettings().value(MemoryKey::DEFAULT_EXTRACT_CHOICE.name, MemoryKey::DEFAULT_EXTRACT_CHOICE.v).toString());
  thumbnailToolButton->MemorizeCurrentAction(MemoryKey::DEFAULT_EXTRACT_CHOICE.name);

  QToolBar* archiveVidsTB{new (std::nothrow) QToolBar("Leaf Arrange Files")};
  CHECK_NULLPTR_RETURN_NULLPTR(archiveVidsTB);
  archiveVidsTB->addWidget(nameRulerToolButton);
  archiveVidsTB->addAction(g_fileBasicOperationsActions()._PACK_FOLDERS);
  archiveVidsTB->addAction(g_fileBasicOperationsActions()._UNPACK_FOLDERS);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(jsonEditorTB);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addAction(g_fileBasicOperationsActions()._LONG_PATH_FINDER);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(folderRmv);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(mediaDupFinder);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(thumbnailToolButton);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addAction(g_rightClickActions()._SEARCH_IN_NET_EXPLORER);
  archiveVidsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
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
  sceneTB->addSeparator();
  sceneTB->addAction(g_viewActions()._FLOATING_PREVIEW);
  sceneTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return sceneTB;
}

void RibbonMenu::Subscribe() {
  connect(g_framelessWindowAg()._EXPAND_RIBBONS, &QAction::triggered, this, &RibbonMenu::on_officeStyleWidgetVisibilityChanged);
  on_officeStyleWidgetVisibilityChanged(g_framelessWindowAg()._EXPAND_RIBBONS->isChecked());
  connect(this, &QTabWidget::currentChanged, this, &RibbonMenu::on_currentTabChangedRecordIndex);
}

void RibbonMenu::on_officeStyleWidgetVisibilityChanged(const bool vis) {
  PreferenceSettings().setValue(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, vis);
  setMaximumHeight(vis ? RibbonMenu::MAX_WIDGET_HEIGHT : tabBar()->height());
}

void RibbonMenu::on_currentTabChangedRecordIndex(const int tabIndex) {
  PreferenceSettings().setValue(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, tabIndex);
}

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

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RibbonMenuIllu ribbonMenuExample(nullptr);
  ribbonMenuExample.show();
  return a.exec();
}
#endif
