#include "CustomTableView.h"
#include "VerMenuInHeader.h"
#include "DoubleRowHeader.h"
#include "ScrollBarPolicyMenu.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "ViewHelper.h"
#include "StyleSheet.h"
#include <QContextMenuEvent>

QSet<QString> CustomTableView::mTableInstSet;
// a bunch of widget with same model should share the only one setting. e.g., HAR_TABLEVIEW

CustomTableView::CustomTableView(const QString& instName, QWidget* parent)
    : QTableView(parent),
      m_name{instName},
      m_showHorizontalHeaderKey{instName + "/SHOW_HORIZONTAL_HEADER"},
      m_showVerticalHeaderKey{instName + "/SHOW_VERTICAL_HEADER"},
      m_autoScrollKey{instName + "/AUTO_SCROLL"},
      m_alternatingRowColorsKey{instName + "/ALTERNATING_ROW_COLORS"},
      m_showGridKey{instName + "/SHOW_GRID"} {
  if (isNameExists(GetName())) {  // not in sharing list, but name already find
    LOG_D("Instance table name[%s] already exist, QSetting may conflict", qPrintable(GetName()));
  }
  mTableInstSet.insert(GetName());

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);


  // 0.
  _SHOW_ALL_HORIZONTAL_COLUMNS = new (std::nothrow) QAction(QIcon{":img/SHOW_ALL_COLUMNS"}, tr("Show All Columns"), this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_ALL_HORIZONTAL_COLUMNS);

  // 1.
  _SHOW_HORIZONTAL_HEADER = new (std::nothrow) QAction(QIcon{":img/HORIZONTAL_HEADER"}, tr("Show Horizontal Header"), this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_HORIZONTAL_HEADER);
  _SHOW_HORIZONTAL_HEADER->setCheckable(true);
  _SHOW_HORIZONTAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide/Show the horizontal header")
                                          .arg(_SHOW_HORIZONTAL_HEADER->text(), _SHOW_HORIZONTAL_HEADER->shortcut().toString()));
  m_horHeader = new DoubleRowHeader{GetName() + "/HorHeader", this};
  CHECK_NULLPTR_RETURN_VOID(m_horHeader);
  if (!_SHOW_HORIZONTAL_HEADER->isChecked()) {
    m_horHeader->setVisible(false);
  }
  setHorizontalHeader(m_horHeader);
  setSortingEnabled(m_horHeader->isSortingEnabled());

  // 2.
  _SHOW_VERTICAL_HEADER = new (std::nothrow) QAction(QIcon{":img/VERTICAL_HEADER"}, tr("Show Vertical Header"), this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_VERTICAL_HEADER);
  _SHOW_VERTICAL_HEADER->setCheckable(true);
  _SHOW_VERTICAL_HEADER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Hide/Show the vertical header").arg(_SHOW_VERTICAL_HEADER->text(), _SHOW_VERTICAL_HEADER->shortcut().toString()));
  m_verHeader = new (std::nothrow) VerMenuInHeader{GetName() + "/VerHeader", this};
  CHECK_NULLPTR_RETURN_VOID(m_verHeader);
  if (!_SHOW_VERTICAL_HEADER->isChecked()) {
    m_verHeader->setVisible(false);
  }
  setVerticalHeader(m_verHeader);

  _RESIZE_ROW_TO_CONTENTS = new (std::nothrow) QAction(QIcon(":img/RESIZE_ROW_TO_CONTENTS"), tr("Resize Rows to Contents"), this);
  _RESIZE_ROW_TO_CONTENTS->setToolTip(
      QString("<b>%1</b><br/>Adjust row heights to fit content (one-time operation)").arg(_RESIZE_ROW_TO_CONTENTS->text()));

  _RESIZE_COLUMN_TO_CONTENTS = new (std::nothrow) QAction(QIcon(":img/RESIZE_COLUMN_TO_CONTENTS"), tr("Resize Columns to Contents"), this);
  _RESIZE_COLUMN_TO_CONTENTS->setToolTip(
      QString("<b>%1</b><br/>Adjust column widths to fit content (one-time operation)").arg(_RESIZE_COLUMN_TO_CONTENTS->text()));

  // 3.
  _AUTO_SCROLL = new (std::nothrow) QAction(QIcon{":img/AUTO_SCROLL"}, tr("Auto Scroll"), this);
  CHECK_NULLPTR_RETURN_VOID(_AUTO_SCROLL);
  _AUTO_SCROLL->setCheckable(true);
  _AUTO_SCROLL->setChecked(Configuration().value(m_autoScrollKey, true).toBool());
  setAutoScroll(_AUTO_SCROLL->isChecked());

  // 4.
  _ALTERNATING_ROW_COLORS = new (std::nothrow) QAction(QIcon{":img/ALTERNATING_ROW_COLORS"}, tr("Alternating row colors"), this);
  CHECK_NULLPTR_RETURN_VOID(_ALTERNATING_ROW_COLORS);
  _ALTERNATING_ROW_COLORS->setCheckable(true);
  _ALTERNATING_ROW_COLORS->setChecked(Configuration().value(m_alternatingRowColorsKey, true).toBool());
  setAlternatingRowColors(_ALTERNATING_ROW_COLORS->isChecked());

  // 5.
  _SHOW_GRID = new (std::nothrow) QAction(QIcon{":img/VIEW_GRID_ON"}, tr("Show grid"), this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_GRID);
  _SHOW_GRID->setCheckable(true);
  _SHOW_GRID->setChecked(Configuration().value(m_showGridKey, false).toBool());
  setShowGrid(_SHOW_GRID->isChecked());

  // 6.
  const QString horMenuName{GetName() + " " + tr("Horizontal scroll bar policy")};
  m_horScrollBarPolicyMenu = new (std::nothrow) ScrollBarPolicyMenu{horMenuName, GetName() + "/Horizontal", this};
  CHECK_NULLPTR_RETURN_VOID(m_horScrollBarPolicyMenu);
  setHorizontalScrollBarPolicy(m_horScrollBarPolicyMenu->GetScrollBarPolicy());

  const QString verMenuName{GetName() + " " + tr("Vertical scroll bar policy")};
  m_verScrollBarPolicyMenu = new (std::nothrow) ScrollBarPolicyMenu{verMenuName, GetName() + "/Vertical", this};
  CHECK_NULLPTR_RETURN_VOID(m_verScrollBarPolicyMenu);
  setVerticalScrollBarPolicy(m_verScrollBarPolicyMenu->GetScrollBarPolicy());

  m_menu = new (std::nothrow) AddableMenu{GetName() + "_menu", this};
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->setToolTipsVisible(true);
  AddItselfAction2Menu();

  SubscribeHeaderActions();
}

CustomTableView::~CustomTableView() {
  Configuration().setValue(m_showHorizontalHeaderKey, _SHOW_HORIZONTAL_HEADER->isChecked());
  Configuration().setValue(m_showVerticalHeaderKey, _SHOW_VERTICAL_HEADER->isChecked());
  Configuration().setValue(m_autoScrollKey, _AUTO_SCROLL->isChecked());
  Configuration().setValue(m_alternatingRowColorsKey, _ALTERNATING_ROW_COLORS->isChecked());
  Configuration().setValue(m_showGridKey, _SHOW_GRID->isChecked());
}

void CustomTableView::PushFrontExclusiveActions(const QList<QAction*>& acts) {
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->push_front(acts);
}

void CustomTableView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(m_menu);
#ifndef RUNNING_UNIT_TESTS
  QPoint pnt = event->globalPos();
  m_menu->popup(pnt);  // or QCursor::pos()
#endif
  event->accept();
  return;
}

void CustomTableView::AddItselfAction2Menu() {
  m_menu->addSeparator();
  m_menu->addAction(_SHOW_ALL_HORIZONTAL_COLUMNS);
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

void CustomTableView::SubscribeHeaderActions() {
  connect(_SHOW_ALL_HORIZONTAL_COLUMNS, &QAction::triggered, m_horHeader, &HorMenuInHeader::onShowAllColumns);
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

void CustomTableView::InitTableView() {
  initExclusivePreferenceSetting();
  StyleSheet::InitFontFamilyAndSize(this);

  _SHOW_HORIZONTAL_HEADER->setChecked(Configuration().value(m_showHorizontalHeaderKey, m_defaultShowHorizontalHeader).toBool());
  _SHOW_VERTICAL_HEADER->setChecked(Configuration().value(m_showVerticalHeaderKey, m_defaultShowVerticalHeader).toBool());

  ShowOrHideColumnCore();
  m_horHeader->InitFilterEditors();
  m_horHeader->RestoreHeaderState();
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
