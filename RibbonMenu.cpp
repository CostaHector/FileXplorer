#include "RibbonMenu.h"
#include <QMenu>
#include <QTabBar>
#include <QToolButton>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/FileLeafAction.h"
#include "Actions/FramelessWindowActions.h"
#include "Actions/JsonEditorActions.h"
#include "Actions/RecycleBinActions.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
#include "Actions/VideoPlayerActions.h"
#include "Actions/ViewActions.h"
#include "Component/DatabaseToolBar.h"
#include "PublicTool.h"
#include "PublicVariable.h"

RibbonMenu::RibbonMenu()
    : menuRibbonCornerWid(GetMenuRibbonCornerWid()),
      leafFileWid(LeafFile()),
      leafHomeWid(LeafHome()),
      leafShareWid(LeafShare()),
      leafViewWid(LeafView()),
      leafDatabaseWid(LeafDatabase()),
      leafMediaWid(LeafMediaTools()) {
  addTab(leafFileWid, "&File");
  addTab(leafHomeWid, "&Home");
  addTab(leafShareWid, "&Share");
  addTab(leafViewWid, "&View");
  addTab(leafDatabaseWid, "&Database");
  addTab(leafMediaWid, "&Media");

  setCornerWidget(menuRibbonCornerWid, Qt::Corner::TopRightCorner);

  Subscribe();

  setCurrentIndex(PreferenceSettings().value(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.v).toInt());
}

QToolBar* RibbonMenu::GetMenuRibbonCornerWid(QWidget* attached) {
  QToolBar* menuRibbonCornerWid = new QToolBar("Frameless window menu bar", attached);
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

  QToolBar* selectionToolBar = new QToolBar("Selection");
  {
    selectionToolBar->addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
    selectionToolBar->setOrientation(Qt::Orientation::Vertical);
    selectionToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    selectionToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
    selectionToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
    SetLayoutAlightment(selectionToolBar->layout(), Qt::AlignmentFlag::AlignLeft);
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
  leafHomeWid->addWidget(advanceSearchToolBar);
  return leafHomeWid;
}

QToolBar* RibbonMenu::LeafShare() const {
  return new QToolBar();
}

QToolBar* RibbonMenu::LeafView() const {
  auto* NAVIGATION_PANE = g_viewActions().NAVIGATION_PANE;
  auto* PREVIEW_PANE_HTML = g_viewActions().PREVIEW_PANE_HTML;
  auto* JSON_EDITOR_PANE = g_viewActions()._JSON_EDITOR_PANE;

  auto* viewPaneToolBar = new QToolBar("View Pane Group");
  viewPaneToolBar->setOrientation(Qt::Orientation::Vertical);
  viewPaneToolBar->addActions({NAVIGATION_PANE, PREVIEW_PANE_HTML});
  viewPaneToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  viewPaneToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  viewPaneToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_2x1, TABS_ICON_IN_MENU_2x1));
  SetLayoutAlightment(viewPaneToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* jsonEditorTB = DropListToolButton(JSON_EDITOR_PANE, g_jsonEditorActions()._BATCH_EDIT_TOOL_ACTIONS->actions(), QToolButton::MenuButtonPopup,
                                          "", Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  auto* embeddedPlayerTB = DropListToolButton(g_viewActions()._VIDEO_PLAYER_EMBEDDED, g_videoPlayerActions()._BATCH_VIDEO_ACTIONS->actions(),
                                              QToolButton::MenuButtonPopup, "", Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  auto* leafViewWid = new QToolBar("Leaf View");
  leafViewWid->setToolTip("View Leaf");
  leafViewWid->addWidget(viewPaneToolBar);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(jsonEditorTB);
  leafViewWid->addSeparator();
  leafViewWid->addWidget(embeddedPlayerTB);
  return leafViewWid;
}

QToolBar* RibbonMenu::LeafDatabase() {
  auto* databaseToolBar = new DatabaseToolBar("Database Leaf", this);
  return databaseToolBar;
}

QToolBar* RibbonMenu::LeafMediaTools() const {
  auto* archiveVidsTB = new QToolBar("Achive vid/img/json files");
  archiveVidsTB->addActions(g_fileBasicOperationsActions().FOLDER_FILE_PROCESS->actions());
  archiveVidsTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return archiveVidsTB;
}

void RibbonMenu::Subscribe() {
  auto on_officeStyleWidget = [this](const bool vis) -> void {
    PreferenceSettings().setValue(MemoryKey::EXPAND_OFFICE_STYLE_MENUBAR.name, vis);
    if (vis) {
      setMaximumHeight(RibbonMenu::MAX_WIDGET_HEIGHT);
    } else {
      setMaximumHeight(tabBar()->height());
    }
  };
  connect(g_framelessWindowAg()._EXPAND_RIBBONS, &QAction::triggered, this, on_officeStyleWidget);

  emit g_framelessWindowAg()._EXPAND_RIBBONS->triggered(g_framelessWindowAg()._EXPAND_RIBBONS->isChecked());

  auto on_currentTabChanged = [](int tabInd) -> void { PreferenceSettings().setValue(MemoryKey::MENU_RIBBON_CURRENT_TAB_INDEX.name, tabInd); };
  connect(this, &QTabWidget::currentChanged, on_currentTabChanged);
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
