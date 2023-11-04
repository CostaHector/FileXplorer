#include "RibbonMenu.h"
#include <QMenu>
#include <QTabBar>
#include <QToolButton>
#include "Actions/FileBasicOperationsActions.h"
#include "Actions/FramelessWindowActions.h"
#include "Actions/RenameActions.h"
#include "Actions/RightClickMenuActions.h"
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
      sqlSearchLE(nullptr),
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
#include "Actions/FileLeafAction.h"
QToolBar* RibbonMenu::LeafFile() const {
  QToolBar* leafFileWid(new QToolBar);
  leafFileWid->addActions(g_fileLeafActions().LEAF_FILE->actions());
  leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return leafFileWid;
}

QToolButton* DropListToolButton(QAction* defaultAction,
                                QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                                const QString& updateToolTip = "",
                                const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                const int iconSide = TABS_ICON_IN_MENU_1x1) {
  QToolButton* tb = new QToolButton();
  if (dropdownActions.isEmpty()) {
    return nullptr;
  }
  if (defaultAction == nullptr) {
    defaultAction = dropdownActions[0];
  }

  tb->setDefaultAction(defaultAction);
  if (not updateToolTip.isEmpty()) {
    defaultAction->setToolTip(updateToolTip);
  }
  tb->setPopupMode(popupMode);
  tb->setToolButtonStyle(toolButtonStyle);
  tb->setAutoRaise(true);
  tb->setStyleSheet("QToolButton { max-width: 128px; }");
  tb->setIconSize(QSize(iconSide, iconSide));

  QMenu* mn = new QMenu(tb);
  mn->addActions(dropdownActions);
  mn->setToolTipsVisible(true);
  tb->setMenu(mn);
  return tb;
}

QToolBar* RibbonMenu::LeafHome() const {
  // Reveal in Explorer
  QToolBar* openToolBar = new QToolBar("Open");
  auto openActs = g_fileBasicOperationsActions().OPEN->actions();
  openToolBar->addActions(QList<QAction*>(openActs.cbegin() + 1, openActs.cend()));
  openToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  openToolBar->setOrientation(Qt::Orientation::Vertical);
  openToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  openToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  SetLayoutAlightment(openToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  const QString& defaultCopyActionName = PreferenceSettings().value(MemoryKey::DEFAULT_COPY_CHOICE.name, MemoryKey::DEFAULT_COPY_CHOICE.v).toString();
  QAction* defaultCopyAction = FindQActionFromQActionGroupByActionName(defaultCopyActionName, g_fileBasicOperationsActions().COPY_PATH);
  QToolButton* copyTB = DropListToolButton(defaultCopyAction, g_fileBasicOperationsActions().COPY_PATH->actions(), QToolButton::MenuButtonPopup, "",
                                           Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  auto ChangeDefaultCopyActionInToolButton = [copyTB](QAction* newDefaultAction) -> void {
    copyTB->setDefaultAction(newDefaultAction);
    PreferenceSettings().setValue(MemoryKey::DEFAULT_COPY_CHOICE.name, newDefaultAction->text());
  };
  connect(copyTB, &QToolButton::triggered, this, ChangeDefaultCopyActionInToolButton);

  QToolBar* propertiesTB = new QToolBar("Properties");
  propertiesTB->addWidget(copyTB);
  propertiesTB->addAction(g_rightClickActions()._CALC_MD5_ACT);
  propertiesTB->setOrientation(Qt::Orientation::Vertical);
  propertiesTB->setStyleSheet("QToolBar { max-width: 256px; }");
  propertiesTB->setIconSize(QSize(TABS_ICON_IN_MENU_2x1, TABS_ICON_IN_MENU_2x1));
  propertiesTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  SetLayoutAlightment(propertiesTB->layout(), Qt::AlignmentFlag::AlignLeft);

  const QString& defaultNewActionName = PreferenceSettings().value(MemoryKey::DEFAULT_NEW_CHOICE.name, MemoryKey::DEFAULT_NEW_CHOICE.v).toString();
  QAction* defaultNewAction = FindQActionFromQActionGroupByActionName(defaultNewActionName, g_fileBasicOperationsActions().NEW);
  QToolButton* newToolBar = DropListToolButton(defaultNewAction, g_fileBasicOperationsActions().NEW->actions(), QToolButton::MenuButtonPopup, "",
                                               Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  auto ChangeDefaultNewActionInToolButton = [newToolBar](QAction* newDefaultAction) -> void {
    newToolBar->setDefaultAction(newDefaultAction);
    PreferenceSettings().setValue(MemoryKey::DEFAULT_NEW_CHOICE.name, newDefaultAction->text());
  };
  connect(newToolBar, &QToolButton::triggered, this, ChangeDefaultNewActionInToolButton);

  QList<QAction*> MOVE_COPY_Acts = g_fileBasicOperationsActions().MOVE_COPY_TO->actions();
  auto* MOVE_TO = MOVE_COPY_Acts[0];
  auto* COPY_TO = MOVE_COPY_Acts[1];

  QToolBar* moveCopy = new QToolBar("Move/Copy");
  moveCopy->addWidget(DropListToolButton(MOVE_TO, g_fileBasicOperationsActions().MOVE_TO_PATH_HISTORY->actions(),
                                         QToolButton::ToolButtonPopupMode::MenuButtonPopup, "", Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                         TABS_ICON_IN_MENU_2x1));
  moveCopy->addWidget(DropListToolButton(COPY_TO, g_fileBasicOperationsActions().COPY_TO_PATH_HISTORY->actions(),
                                         QToolButton::ToolButtonPopupMode::MenuButtonPopup, "", Qt::ToolButtonStyle::ToolButtonTextBesideIcon,
                                         TABS_ICON_IN_MENU_2x1));
  moveCopy->setOrientation(Qt::Orientation::Vertical);
  moveCopy->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  moveCopy->setIconSize(QSize(TABS_ICON_IN_MENU_2x1, TABS_ICON_IN_MENU_2x1));
  moveCopy->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(moveCopy->layout(), Qt::AlignmentFlag::AlignLeft);

  QToolButton* recycleToolButton =
      DropListToolButton(nullptr, g_fileBasicOperationsActions().DELETE_ACTIONS->actions(), QToolButton::MenuButtonPopup);

  QToolBar* selectionToolBar = new QToolBar("Selection");
  selectionToolBar->addActions(g_fileBasicOperationsActions().SELECTION_RIBBONS->actions());
  selectionToolBar->setOrientation(Qt::Orientation::Vertical);
  selectionToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  selectionToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_3x1, TABS_ICON_IN_MENU_3x1));
  selectionToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  SetLayoutAlightment(selectionToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* defaultRenameAction = FindQActionFromQActionGroupByActionName(
      PreferenceSettings().value(MemoryKey::DEFAULT_RENAME_CHOICE.name, MemoryKey::DEFAULT_RENAME_CHOICE.v).toString(), g_renameAg().RENAME_RIBBONS);
  QToolButton* renameToolButton = DropListToolButton(defaultRenameAction, g_renameAg().RENAME_RIBBONS->actions(), QToolButton::MenuButtonPopup);
  auto ChangeDefaultRenameActionInToolButton = [renameToolButton](QAction* newDefaultAction) -> void {
    renameToolButton->setDefaultAction(newDefaultAction);
    PreferenceSettings().setValue(MemoryKey::DEFAULT_RENAME_CHOICE.name, newDefaultAction->text());
  };
  connect(renameToolButton, &QToolButton::triggered, this, ChangeDefaultRenameActionInToolButton);

  QToolBar* advanceSearchToolBar = new QToolBar("AdvanceSearch");
  advanceSearchToolBar->addActions(g_fileBasicOperationsActions().ADVANCE_SEARCH_RIBBON->actions());
  advanceSearchToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  advanceSearchToolBar->setStyleSheet("QToolBar { max-width: 256px; }");

  QToolBar* leafHomeWid = new QToolBar("LeafHome");
  leafHomeWid->addWidget(openToolBar);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(propertiesTB);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(newToolBar);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(moveCopy);
  leafHomeWid->addWidget(recycleToolButton);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(selectionToolBar);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(renameToolButton);
  leafHomeWid->addSeparator();
  leafHomeWid->addWidget(advanceSearchToolBar);
  return leafHomeWid;
}

QToolBar* RibbonMenu::LeafShare() const {
  return new QToolBar();
}

QToolBar* RibbonMenu::LeafView() const {
  const auto PANES_RIBBONSList = g_viewActions().PANES_RIBBONS->actions();
  auto* NAVIGATION_PANE = PANES_RIBBONSList[0];
  auto* PREVIEW_PANE_HTML = PANES_RIBBONSList[1];
  auto* PREVIEW_PANE_JSON = PANES_RIBBONSList[2];
  auto* ADD_TO_JSON_POOL = PANES_RIBBONSList[3];

  auto* viewPaneToolBar = new QToolBar("View Pane Group");
  viewPaneToolBar->setOrientation(Qt::Orientation::Vertical);
  viewPaneToolBar->addActions({NAVIGATION_PANE, PREVIEW_PANE_HTML});
  viewPaneToolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  viewPaneToolBar->setStyleSheet("QToolBar { max-width: 256px; }");
  viewPaneToolBar->setIconSize(QSize(TABS_ICON_IN_MENU_2x1, TABS_ICON_IN_MENU_2x1));
  SetLayoutAlightment(viewPaneToolBar->layout(), Qt::AlignmentFlag::AlignLeft);

  auto* jsonEditorTB =
      DropListToolButton(PREVIEW_PANE_JSON, {ADD_TO_JSON_POOL}, QToolButton::MenuButtonPopup, "", Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  auto* leafViewWid = new QToolBar("Leaf View");
  leafViewWid->addWidget(viewPaneToolBar);
  leafViewWid->addWidget(jsonEditorTB);

  return leafViewWid;
}

QToolBar* RibbonMenu::LeafDatabase() {
  auto* databaseToolBar = new DatabaseToolBar("Database Leaf", this);
  sqlSearchLE = databaseToolBar->sqlSearchLE;
  return databaseToolBar;
}

QToolBar* RibbonMenu::LeafMediaTools() const {
  auto* archiveVidsTB = new QToolBar("Achive vid/img/json files");
  archiveVidsTB->addActions(g_fileBasicOperationsActions().FOLDER_FILE_PROCESS->actions());
  archiveVidsTB->setIconSize(QSize(TABS_ICON_IN_MENU_1x1, TABS_ICON_IN_MENU_1x1));
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
