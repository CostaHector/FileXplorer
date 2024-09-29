#include "RibbonMenu.h"
#include <QMenu>
#include <QTabBar>
#include <QToolButton>
#include "Actions/ArchiveFilesActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/FileLeafAction.h"
#include "Actions/FolderPreviewActions.h"
#include "Actions/FramelessWindowActions.h"
#include "Actions/JsonEditorActions.h"
#include "Actions/RecycleBinActions.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
#include "Actions/SceneInPageActions.h"
#include "Actions/SyncFileSystemModificationActions.h"
#include "Actions/VideoPlayerActions.h"
#include "Actions/ViewActions.h"
#include "Component/DatabaseToolBar.h"
#include "PublicTool.h"
#include "PublicVariable.h"

QToolButton* DropListToolButton(QAction* defaultAction,
                                QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                                const QString& updateToolTip = "",
                                const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                const int iconSize = TABS_ICON_IN_MENU_1x1) {
  if (dropdownActions.isEmpty()) {
    return nullptr;
  }
  if (defaultAction == nullptr) {
    defaultAction = dropdownActions[0];
  }

  QToolButton* tb = new QToolButton;
  tb->setDefaultAction(defaultAction);
  if (not updateToolTip.isEmpty()) {
    defaultAction->setToolTip(updateToolTip);
  }
  tb->setPopupMode(popupMode);
  tb->setToolButtonStyle(toolButtonStyle);
  tb->setAutoRaise(true);
  tb->setStyleSheet("QToolButton { max-width: 256px; }");
  tb->setIconSize(QSize(iconSize, iconSize));

  QMenu* mn = new QMenu(tb);
  mn->addActions(dropdownActions);
  mn->setToolTipsVisible(true);
  tb->setMenu(mn);
  return tb;
}

RibbonMenu::RibbonMenu(QWidget* parent)
    : QTabWidget{parent},
      m_corner(GetMenuRibbonCornerWid()),
      m_leafFile(LeafFile()),
      m_leafHome(LeafHome()),
      m_leafView(LeafView()),
      m_leafDatabase(LeafDatabase()),
      m_leafScenes(LeafScenesTools()),
      m_leafMore(LeafMediaTools()) {
  addTab(m_leafFile, "&File");
  addTab(m_leafHome, "&Home");
  addTab(m_leafView, "&View");
  addTab(m_leafDatabase, "&Database");
  addTab(m_leafScenes, "&Scene");
  addTab(m_leafMore, "&Arrange");

  setCornerWidget(m_corner, Qt::Corner::TopRightCorner);

  Subscribe();

  setCurrentIndex(PreferenceSettings().value(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.v).toInt());
}

QToolBar* RibbonMenu::GetMenuRibbonCornerWid(QWidget* attached) {
  QToolBar* menuRibbonCornerWid = new QToolBar("corner tools", attached);
  menuRibbonCornerWid->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  menuRibbonCornerWid->addSeparator();
  menuRibbonCornerWid->addAction(g_framelessWindowAg()._EXPAND_RIBBONS);
  menuRibbonCornerWid->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  return menuRibbonCornerWid;
}

QToolBar* RibbonMenu::LeafFile() const {
  QToolBar* leafFileWid(new QToolBar);
  leafFileWid->addActions(g_fileLeafActions().LEAF_FILE->actions());
  leafFileWid->addSeparator();
  leafFileWid->addAction(g_recycleBinAg().RECYLE_BIN_WIDGET);
  leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafFileWid;
}

QToolBar* RibbonMenu::LeafHome() const {
  const QString& _defPlayActName = PreferenceSettings().value(MemoryKey::DEFAULT_VIDEO_PLAYER.name, MemoryKey::DEFAULT_VIDEO_PLAYER.v).toString();
  QAction* _defPlayAct = FindQActionFromQActionGroupByActionName(_defPlayActName, g_viewActions()._VIDEO_PLAYERS);
  QToolButton* playTB = DropListToolButton(_defPlayAct, g_viewActions()._VIDEO_PLAYERS->actions(), QToolButton::MenuButtonPopup, "",
                                           Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  {
    auto onDefPlayActChanged = [playTB](QAction* triggeredAct) -> void {
      playTB->setDefaultAction(triggeredAct);
      PreferenceSettings().setValue(MemoryKey::DEFAULT_VIDEO_PLAYER.name, triggeredAct->text());
    };
    connect(playTB, &QToolButton::triggered, this, onDefPlayActChanged);
  }

  QToolBar* openItemsTB = new QToolBar("Open");
  {
    openItemsTB->addWidget(playTB);
    openItemsTB->addActions(g_fileBasicOperationsActions().OPEN_AG->actions());
    openItemsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    openItemsTB->setOrientation(Qt::Orientation::Vertical);
    openItemsTB->setStyleSheet("QToolBar { max-width: 256px; }");
    openItemsTB->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
    SetLayoutAlightment(openItemsTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  const QString& defaultCopyActionName = PreferenceSettings().value(MemoryKey::DEFAULT_COPY_CHOICE.name, MemoryKey::DEFAULT_COPY_CHOICE.v).toString();
  QAction* defaultCopyAction = FindQActionFromQActionGroupByActionName(defaultCopyActionName, g_fileBasicOperationsActions().COPY_PATH_AG);
  QToolButton* copyTB = DropListToolButton(defaultCopyAction, g_fileBasicOperationsActions().COPY_PATH_AG->actions(), QToolButton::MenuButtonPopup,
                                           "", Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  {
    auto onDefCopyActChanged = [copyTB](QAction* triggeredAct) -> void {
      copyTB->setDefaultAction(triggeredAct);
      PreferenceSettings().setValue(MemoryKey::DEFAULT_COPY_CHOICE.name, triggeredAct->text());
    };
    connect(copyTB, &QToolButton::triggered, this, onDefCopyActChanged);
  }

  QToolBar* propertiesTB = new QToolBar("Properties");
  {
    propertiesTB->addWidget(copyTB);
    propertiesTB->addAction(g_rightClickActions()._CALC_MD5_ACT);
    propertiesTB->addAction(g_rightClickActions()._PROPERTIES);
    propertiesTB->setOrientation(Qt::Orientation::Vertical);
    propertiesTB->setStyleSheet("QToolBar { max-width: 256px; }");
    propertiesTB->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
    propertiesTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    SetLayoutAlightment(propertiesTB->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  const QString& _defNewActName = PreferenceSettings().value(MemoryKey::DEFAULT_NEW_CHOICE.name, MemoryKey::DEFAULT_NEW_CHOICE.v).toString();
  QAction* _defaultNewAction = FindQActionFromQActionGroupByActionName(_defNewActName, g_fileBasicOperationsActions().NEW);
  QToolButton* newItemsTB = DropListToolButton(_defaultNewAction, g_fileBasicOperationsActions().NEW->actions(), QToolButton::MenuButtonPopup, "",
                                               Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  {
    auto onDefNewActChanged = [newItemsTB](QAction* triggeredAct) -> void {
      newItemsTB->setDefaultAction(triggeredAct);
      PreferenceSettings().setValue(MemoryKey::DEFAULT_NEW_CHOICE.name, triggeredAct->text());
    };
    connect(newItemsTB, &QToolButton::triggered, this, onDefNewActChanged);
  }

  QToolBar* moveCopyItemsToTB = new QToolBar("Move/Copy item(s) To ToolBar");
  {
    auto* const _MOVE_TO = g_fileBasicOperationsActions()._MOVE_TO;
    auto* const _COPY_TO = g_fileBasicOperationsActions()._COPY_TO;
    const auto& _MOVE_TO_HIST_LIST = g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY->actions();
    const auto& _COPY_TO_HIST_LIST = g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY->actions();
    moveCopyItemsToTB->addWidget(DropListToolButton(_MOVE_TO, _MOVE_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup, "",
                                                    Qt::ToolButtonStyle::ToolButtonTextUnderIcon, TABS_ICON_IN_MENU_2x1));
    moveCopyItemsToTB->addWidget(DropListToolButton(_COPY_TO, _COPY_TO_HIST_LIST, QToolButton::ToolButtonPopupMode::MenuButtonPopup, "",
                                                    Qt::ToolButtonStyle::ToolButtonTextUnderIcon, TABS_ICON_IN_MENU_2x1));
    moveCopyItemsToTB->setOrientation(Qt::Orientation::Horizontal);
    SetLayoutAlightment(moveCopyItemsToTB->layout(), Qt::AlignmentFlag::AlignTop);
  }

  QToolButton* recycleItemsTB = DropListToolButton(nullptr, g_fileBasicOperationsActions().DELETE_ACTIONS->actions(), QToolButton::MenuButtonPopup);

  QToolBar* archievePreviewToolBar = new QToolBar("ArchievePreview");
  archievePreviewToolBar->addAction(g_AchiveFilesActions().ARCHIVE_PREVIEW);
  archievePreviewToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  archievePreviewToolBar->setStyleSheet("QToolBar { max-width: 256px; }");

  QToolBar* selectionToolBar = new QToolBar("Selection");
  {
    selectionToolBar->addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
    selectionToolBar->setOrientation(Qt::Orientation::Vertical);
    selectionToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    selectionToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
    selectionToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(selectionToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* compressToolBar = new QToolBar("Compress/Decompress");
  {
    compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_HERE);
    compressToolBar->addAction(g_AchiveFilesActions().COMPRESSED_IMAGES);
    compressToolBar->addAction(g_AchiveFilesActions().DECOMPRESSED_HERE);
    compressToolBar->setOrientation(Qt::Orientation::Vertical);
    compressToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    compressToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
    compressToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(compressToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  const QString& _defRenameActName = PreferenceSettings().value(MemoryKey::DEFAULT_RENAME_CHOICE.name, MemoryKey::DEFAULT_RENAME_CHOICE.v).toString();
  QAction* _defRenameAct = FindQActionFromQActionGroupByActionName(_defRenameActName, g_renameAg().RENAME_RIBBONS);
  QToolButton* renameItemsTB = DropListToolButton(_defRenameAct, g_renameAg().RENAME_RIBBONS->actions(), QToolButton::MenuButtonPopup);
  {
    auto onDefRenameActChanged = [renameItemsTB](QAction* triggeredAct) -> void {
      renameItemsTB->setDefaultAction(triggeredAct);
      PreferenceSettings().setValue(MemoryKey::DEFAULT_RENAME_CHOICE.name, triggeredAct->text());
    };
    connect(renameItemsTB, &QToolButton::triggered, this, onDefRenameActChanged);
  }

  QToolBar* advanceSearchToolBar = new QToolBar("AdvanceSearch");
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
    syncPathToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
    syncPathToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    SetLayoutAlightment(syncPathToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
  }

  QToolBar* leafHomeWid = new QToolBar("LeafHome");
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
  auto* leafViewWid = new QToolBar("Leaf View");

  auto* fileSystemView = new QToolBar("Navigation Preview Switch");
  fileSystemView->setOrientation(Qt::Orientation::Vertical);
  fileSystemView->addActions({g_viewActions()._LIST_VIEW, g_viewActions()._TABLE_VIEW, g_viewActions()._TREE_VIEW});
  fileSystemView->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  fileSystemView->setStyleSheet("QToolBar { max-width: 256px; }");
  fileSystemView->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  SetLayoutAlightment(fileSystemView->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* folderPreviewToolBar = g_folderPreviewActions().GetPreviewsToolbar(leafViewWid);
  folderPreviewToolBar->setOrientation(Qt::Orientation::Vertical);
  folderPreviewToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  folderPreviewToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  folderPreviewToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  SetLayoutAlightment(folderPreviewToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* JSON_EDITOR_PANE = g_viewActions()._JSON_EDITOR_PANE;
  auto* jsonEditorTB = DropListToolButton(JSON_EDITOR_PANE, g_jsonEditorActions()._BATCH_EDIT_TOOL_ACTIONS->actions(), QToolButton::MenuButtonPopup,
                                          "", Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  auto* embeddedPlayerTB = DropListToolButton(g_viewActions()._VIDEO_PLAYER_EMBEDDED, g_videoPlayerActions()._BATCH_VIDEO_ACTIONS->actions(),
                                              QToolButton::MenuButtonPopup, "", Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  leafViewWid->setToolTip("View Leaf");
  leafViewWid->addAction(g_viewActions().NAVIGATION_PANE);
  leafViewWid->addWidget(fileSystemView);
  leafViewWid->addSeparator();
  leafViewWid->addAction(g_viewActions().PREVIEW_PANE_HTML);
  leafViewWid->addWidget(folderPreviewToolBar);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(jsonEditorTB);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(embeddedPlayerTB);
  leafViewWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafViewWid;
}

QToolBar* RibbonMenu::LeafDatabase() const {
  auto* databaseToolBar = new DatabaseToolBar("Leaf Database");
  return databaseToolBar;
}

QToolBar* RibbonMenu::LeafMediaTools() const {
  auto* folderRmv{new QToolBar{"Folder Remover"}};
  folderRmv->setOrientation(Qt::Orientation::Vertical);
  folderRmv->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  folderRmv->setStyleSheet("QToolBar { max-width: 256px; }");
  folderRmv->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  folderRmv->addAction(g_fileBasicOperationsActions()._RMV_REDUN_PARENT_FOLDER);
  folderRmv->addAction(g_fileBasicOperationsActions()._RMV_EMPTY_FOLDER_R);
  folderRmv->addAction(g_fileBasicOperationsActions()._RMV_FOLDER_BY_KEYWORD);

  auto* mediaDupFinder{new QToolBar{"Duplicate Medias Finder"}};
  mediaDupFinder->setOrientation(Qt::Orientation::Vertical);
  mediaDupFinder->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mediaDupFinder->setStyleSheet("QToolBar { max-width: 256px; }");
  mediaDupFinder->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  mediaDupFinder->addAction(g_fileBasicOperationsActions()._DUPLICATE_ITEMS_REMOVER);
  mediaDupFinder->addAction(g_fileBasicOperationsActions()._REDUNDANT_IMAGES_FINDER);
  mediaDupFinder->addAction(g_fileBasicOperationsActions()._DUPLICATE_VIDEOS_FINDER);

  auto* archiveVidsTB = new QToolBar("Leaf Arrange Files");
  archiveVidsTB->addAction(g_fileBasicOperationsActions()._NAME_STANDARDLIZER);
  archiveVidsTB->addAction(g_fileBasicOperationsActions()._CLASSIFIER);
  archiveVidsTB->addAction(g_fileBasicOperationsActions()._LONG_PATH_FINDER);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(folderRmv);
  archiveVidsTB->addSeparator();
  archiveVidsTB->addWidget(mediaDupFinder);
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
  sceneTB->addSeparator();
  sceneTB->addWidget(ag.mOrderTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(ag.mEnablePageTB);
  sceneTB->addSeparator();
  sceneTB->addWidget(ag.mPagesSelectTB);
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
