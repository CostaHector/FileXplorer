#include "CustomTableView.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "ViewHelper.h"
#include <QContextMenuEvent>

QSet<QString> CustomTableView::TABLES_SET;
// a bunch of widget with same model should share the only one setting. e.g., HAR_TABLEVIEW

CustomTableView::CustomTableView(const QString& name, QWidget* parent)
  : QTableView(parent)
  , m_name{name}
  , m_showHorizontalHeaderKey{m_name + "_SHOW_HORIZONTAL_HEADER"}
  , m_showVerticalHeaderKey{m_name + "_SHOW_VERTICAL_HEADER"}
  , m_autoScrollKey{m_name + "_AUTO_SCROLL"}
  , m_alternatingRowColorsKey{m_name + "_ALTERNATING_ROW_COLORS"}
  , m_showGridKey{m_name + "_SHOW_GRID"} {
  if (isNameExists(m_name)) { // not in sharing list, but name already find
    LOG_D("Instance Name[%s] already exist, QSetting may conflict", qPrintable(m_name));
  }
  TABLES_SET.insert(m_name);

  setAlternatingRowColors(true);
  setShowGrid(false);

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  const int fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);
  // 1.
  _SHOW_HORIZONTAL_HEADER = new (std::nothrow) QAction(QIcon{":img/HORIZONTAL_HEADER"}, "Show Horizontal Header", this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_HORIZONTAL_HEADER);
  _SHOW_HORIZONTAL_HEADER->setCheckable(true);
  _SHOW_HORIZONTAL_HEADER->setChecked(Configuration().value(m_showHorizontalHeaderKey, true).toBool());
  _SHOW_HORIZONTAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide/Show the horizontal header")
                                          .arg(_SHOW_HORIZONTAL_HEADER->text(), _SHOW_HORIZONTAL_HEADER->shortcut().toString()));
  m_horHeader = new DoubleRowHeader{m_name + "_HorHeader", this};
  CHECK_NULLPTR_RETURN_VOID(m_horHeader);
  if (!_SHOW_HORIZONTAL_HEADER->isChecked()) {
    m_horHeader->setVisible(false);
  }
  setHorizontalHeader(m_horHeader);
  setSortingEnabled(m_horHeader->isSortingEnabled());

  // 2.
  _SHOW_VERTICAL_HEADER = new (std::nothrow) QAction(QIcon{":img/VERTICAL_HEADER"}, "Show Vertical Header", this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_VERTICAL_HEADER);
  _SHOW_VERTICAL_HEADER->setCheckable(true);
  _SHOW_VERTICAL_HEADER->setChecked(Configuration().value(m_showVerticalHeaderKey, true).toBool());
  _SHOW_VERTICAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide/Show the vertical header")
                                        .arg(_SHOW_VERTICAL_HEADER->text(), _SHOW_VERTICAL_HEADER->shortcut().toString()));
  m_verHeader = new (std::nothrow) VerMenuInHeader{m_name + "_VerHeader", this};
  CHECK_NULLPTR_RETURN_VOID(m_verHeader);
  if (!_SHOW_VERTICAL_HEADER->isChecked()) {
    m_verHeader->setVisible(false);
  }
  setVerticalHeader(m_verHeader);

  _RESIZE_ROW_TO_CONTENTS = new (std::nothrow) QAction(QIcon(":img/RESIZE_ROW_TO_CONTENTS"), "Resize Rows to Contents", this);
  _RESIZE_ROW_TO_CONTENTS->setToolTip(
      QString("<b>%1</b><br/>Adjust row heights to fit content (one-time operation)").arg(_RESIZE_ROW_TO_CONTENTS->text()));

  _RESIZE_COLUMN_TO_CONTENTS = new (std::nothrow) QAction(QIcon(":img/RESIZE_COLUMN_TO_CONTENTS"), "Resize Columns to Contents", this);
  _RESIZE_COLUMN_TO_CONTENTS->setToolTip(
      QString("<b>%1</b><br/>Adjust column widths to fit content (one-time operation)").arg(_RESIZE_COLUMN_TO_CONTENTS->text()));

  // 3.
  _AUTO_SCROLL = new (std::nothrow) QAction(QIcon{":img/AUTO_SCROLL"}, "Auto Scroll", this);
  CHECK_NULLPTR_RETURN_VOID(_AUTO_SCROLL);
  _AUTO_SCROLL->setCheckable(true);
  _AUTO_SCROLL->setChecked(Configuration().value(m_autoScrollKey, true).toBool());
  setAutoScroll(_AUTO_SCROLL->isChecked());

  // 4.
  _ALTERNATING_ROW_COLORS = new (std::nothrow) QAction(QIcon{":img/ALTERNATING_ROW_COLORS"}, "Alternating row colors", this);
  CHECK_NULLPTR_RETURN_VOID(_ALTERNATING_ROW_COLORS);
  _ALTERNATING_ROW_COLORS->setCheckable(true);
  _ALTERNATING_ROW_COLORS->setChecked(Configuration().value(m_alternatingRowColorsKey, true).toBool());
  setAlternatingRowColors(_ALTERNATING_ROW_COLORS->isChecked());

  // 5.
  _SHOW_GRID = new (std::nothrow) QAction(QIcon{":img/VIEW_GRID_ON"}, "Show grid", this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_GRID);
  _SHOW_GRID->setCheckable(true);
  _SHOW_GRID->setChecked(Configuration().value(m_showGridKey, false).toBool());
  setShowGrid(_SHOW_GRID->isChecked());

  // 6.
  m_horScrollBarPolicyMenu = new (std::nothrow) ScrollBarPolicyMenu{m_name + "_HorScrollBarPolicy", this};
  CHECK_NULLPTR_RETURN_VOID(m_horScrollBarPolicyMenu);
  m_horScrollBarPolicyMenu->setIcon(QIcon{":img/SCROLL_BAR_POLICY_HOR"});
  setHorizontalScrollBarPolicy(m_horScrollBarPolicyMenu->GetScrollBarPolicy());

  m_verScrollBarPolicyMenu = new (std::nothrow) ScrollBarPolicyMenu{m_name + "_VerScrollBarPolicy", this};
  CHECK_NULLPTR_RETURN_VOID(m_verScrollBarPolicyMenu);
  m_verScrollBarPolicyMenu->setIcon(QIcon{":img/SCROLL_BAR_POLICY_VER"});
  setVerticalScrollBarPolicy(m_verScrollBarPolicyMenu->GetScrollBarPolicy());

  m_menu = new (std::nothrow) AddableMenu{m_name + "_menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->setToolTipsVisible(true);
  SubscribeHeaderActions();
}

CustomTableView::~CustomTableView() {
  Configuration().setValue(m_showHorizontalHeaderKey, _SHOW_HORIZONTAL_HEADER->isChecked());
  Configuration().setValue(m_showVerticalHeaderKey, _SHOW_VERTICAL_HEADER->isChecked());
  Configuration().setValue(m_autoScrollKey, _AUTO_SCROLL->isChecked());
  Configuration().setValue(m_alternatingRowColorsKey, _ALTERNATING_ROW_COLORS->isChecked());
  Configuration().setValue(m_showGridKey, _SHOW_GRID->isChecked());
}

void CustomTableView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  if (m_menu != nullptr) {
    QPoint pnt = event->globalPos();
#ifndef RUNNING_UNIT_TESTS
    m_menu->popup(pnt); // or QCursor::pos()
#endif
    event->accept();
    return;
  }
  QTableView::contextMenuEvent(event);
}

void CustomTableView::BindMenu(QMenu* menu) {
  CHECK_NULLPTR_RETURN_VOID(menu);
  m_menu->operator+=(*menu);
  AddItselfAction2Menu();
}

void CustomTableView::AddItselfAction2Menu() {
  m_menu->addSeparator();
  m_menu->addAction(_SHOW_HORIZONTAL_HEADER);
  m_menu->addAction(_RESIZE_COLUMN_TO_CONTENTS);
  m_menu->addMenu(m_horScrollBarPolicyMenu);
  m_menu->addSeparator();
  m_menu->addAction(_SHOW_VERTICAL_HEADER);
  m_menu->addAction(_RESIZE_ROW_TO_CONTENTS);
  m_menu->addMenu(m_verScrollBarPolicyMenu);
  m_menu->addSeparator();
  m_menu->addAction(_AUTO_SCROLL);
  m_menu->addAction(_ALTERNATING_ROW_COLORS);
  m_menu->addAction(_SHOW_GRID);
}

bool CustomTableView::ShowOrHideColumnCore() {
  const QString& switches = m_horHeader->GetColumnsShowSwitch();
  if (switches.isEmpty()) {
    LOG_D("Skip set visibility of horizontal header. switch batch is empty.");
    return false;
  }
  auto* model_ = this->model();
  CHECK_NULLPTR_RETURN_FALSE(model_)
  const int tableColumnsCount = model_->columnCount();
  if (tableColumnsCount > switches.size()) {
    LOG_W("Skip set visibility of horizontal header. switchs count less than columns count.");
    return false;
  }
  for (int c = 0; c < tableColumnsCount; ++c) {
    setColumnHidden(c, switches[c] == '0');
  }
  return true;
}

void CustomTableView::InitTableView(const bool bHideShowCol) {
  if (bHideShowCol) {
    ShowOrHideColumnCore();
  }
  m_horHeader->InitFilterEditors();
  m_horHeader->RestoreHeaderState();
}

void CustomTableView::SubscribeHeaderActions() {
  connect(_SHOW_HORIZONTAL_HEADER, &QAction::toggled, m_horHeader, &QHeaderView::setVisible);
  connect(_SHOW_VERTICAL_HEADER, &QAction::toggled, m_verHeader, &QHeaderView::setVisible);
  connect(_AUTO_SCROLL, &QAction::toggled, this, &QTableView::setAutoScroll);
  connect(_ALTERNATING_ROW_COLORS, &QAction::toggled, this, &QTableView::setAlternatingRowColors);
  connect(_SHOW_GRID, &QAction::toggled, this, &QTableView::setShowGrid);
  connect(m_horScrollBarPolicyMenu, &ScrollBarPolicyMenu::reqScrollBarPolicyChanged, this, &QTableView::setHorizontalScrollBarPolicy);
  connect(m_verScrollBarPolicyMenu, &ScrollBarPolicyMenu::reqScrollBarPolicyChanged, this, &QTableView::setVerticalScrollBarPolicy);

  connect(m_horHeader, &DoubleRowHeader::reqParentTableUpdateGeometries, this, &CustomTableView::updateGeometries);
  connect(m_horHeader, &DoubleRowHeader::reqHideAColumn, this, &QTableView::setColumnHidden);
  connect(m_horHeader, &DoubleRowHeader::reqUpdateColumnVisibilty, this, &CustomTableView::ShowOrHideColumnCore);
  connect(m_horHeader, &DoubleRowHeader::searchStatementChanged, this, &CustomTableView::searchSqlStatementChanged);

  connect(_RESIZE_ROW_TO_CONTENTS, &QAction::triggered, this, &QTableView::resizeRowsToContents);
  connect(_RESIZE_COLUMN_TO_CONTENTS, &QAction::triggered, this, &QTableView::resizeColumnsToContents);

  connect(m_horHeader, &HorMenuInHeader::reqSortEnabled, this, &QTableView::setSortingEnabled);
}

void CustomTableView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->modifiers(), event->button())) {
    event->accept();
    return;
  }
  QTableView::mousePressEvent(event);
}

void CustomTableView::scrollContentsBy(int dx, int dy) {
  QTableView::scrollContentsBy(dx, dy);
  m_horHeader->updateFilterEditorsGeometry();
}
