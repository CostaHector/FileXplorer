#include "CustomTreeView.h"
#include "ViewHelper.h"
#include "DoubleRowHeader.h"
#include "ScrollBarPolicyMenu.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "PublicMacro.h"
#include "ViewItemDelegate.h"
#include "StyleSheet.h"

QSet<QString> CustomTreeView::mTreeInstSet;

CustomTreeView::CustomTreeView(const QString& instName, QWidget* parent)  //
    : QTreeView{parent},                                                  //
      m_name{instName},                                                   //
      m_showHorizontalHeaderKey{instName + "/SHOW_HORIZONTAL_HEADER"},    //
      m_autoScrollKey{instName + "/AUTO_SCROLL"},                         //
      m_alternatingRowColorsKey{instName + "/ALTERNATING_ROW_COLORS"}     //
{
  if (isNameExists(GetName())) {  // not in sharing list, but name already find
    LOG_D("Instance tree name[%s] already exist, QSetting may conflict", qPrintable(GetName()));
  }
  mTreeInstSet.insert(GetName());

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  ViewItemDelegate* pDelegate = new ViewItemDelegate{this};
  setItemDelegate(pDelegate);
  setUniformRowHeights(true);

  // 0.
  _SHOW_ALL_HORIZONTAL_COLUMNS = new (std::nothrow) QAction(QIcon{":img/SHOW_ALL_COLUMNS"}, tr("Show All Columns"), this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_ALL_HORIZONTAL_COLUMNS);

  // 1.
  _SHOW_HORIZONTAL_HEADER = new (std::nothrow) QAction(QIcon{":img/HORIZONTAL_HEADER"}, tr("Show Horizontal Header"), this);
  CHECK_NULLPTR_RETURN_VOID(_SHOW_HORIZONTAL_HEADER);
  _SHOW_HORIZONTAL_HEADER->setCheckable(true);
  _SHOW_HORIZONTAL_HEADER->setChecked(Configuration().value(m_showHorizontalHeaderKey, true).toBool());
  _SHOW_HORIZONTAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide/Show the horizontal header")
                                          .arg(_SHOW_HORIZONTAL_HEADER->text(), _SHOW_HORIZONTAL_HEADER->shortcut().toString()));

  m_horHeader = new DoubleRowHeader{GetName() + "/HorHeader", this};
  CHECK_NULLPTR_RETURN_VOID(m_horHeader);
  if (!_SHOW_HORIZONTAL_HEADER->isChecked()) {
    m_horHeader->setVisible(false);
  }
  setHeader(m_horHeader);
  setSortingEnabled(m_horHeader->isSortingEnabled());

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

CustomTreeView::~CustomTreeView() {
  Configuration().setValue(m_showHorizontalHeaderKey, _SHOW_HORIZONTAL_HEADER->isChecked());
  Configuration().setValue(m_autoScrollKey, _AUTO_SCROLL->isChecked());
  Configuration().setValue(m_alternatingRowColorsKey, _ALTERNATING_ROW_COLORS->isChecked());
}

void CustomTreeView::PushFrontExclusiveActions(const QList<QAction*>& acts) {
  CHECK_NULLPTR_RETURN_VOID(m_menu);
  m_menu->push_front(acts);
}

void CustomTreeView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(m_menu);
#ifndef RUNNING_UNIT_TESTS
  QPoint pnt = event->globalPos();
  m_menu->popup(pnt);  // or QCursor::pos()
#endif
  event->accept();
  return;
}

void CustomTreeView::AddItselfAction2Menu() {
  m_menu->addSeparator();
  m_menu->addAction(_SHOW_ALL_HORIZONTAL_COLUMNS);
  m_menu->addAction(_SHOW_HORIZONTAL_HEADER);
  m_menu->addMenu(m_horScrollBarPolicyMenu);
  m_menu->addSeparator();
  m_menu->addMenu(m_verScrollBarPolicyMenu);
  m_menu->addSeparator();
  m_menu->addAction(_AUTO_SCROLL);
  m_menu->addAction(_ALTERNATING_ROW_COLORS);
}

void CustomTreeView::SubscribeHeaderActions() {
  connect(_SHOW_ALL_HORIZONTAL_COLUMNS, &QAction::triggered, m_horHeader, &HorMenuInHeader::onShowAllColumns);
  connect(_SHOW_HORIZONTAL_HEADER, &QAction::toggled, m_horHeader, &QHeaderView::setVisible);
  connect(_AUTO_SCROLL, &QAction::toggled, this, &QTreeView::setAutoScroll);
  connect(_ALTERNATING_ROW_COLORS, &QAction::toggled, this, &QTreeView::setAlternatingRowColors);
  connect(m_horScrollBarPolicyMenu, &ScrollBarPolicyMenu::reqScrollBarPolicyChanged, this, &QTreeView::setHorizontalScrollBarPolicy);
  connect(m_verScrollBarPolicyMenu, &ScrollBarPolicyMenu::reqScrollBarPolicyChanged, this, &QTreeView::setVerticalScrollBarPolicy);

  connect(m_horHeader, &DoubleRowHeader::reqParentTableUpdateGeometries, this, &CustomTreeView::updateGeometries);
  connect(m_horHeader, &DoubleRowHeader::reqHideAColumn, this, &QTreeView::setColumnHidden);
  connect(m_horHeader, &DoubleRowHeader::reqUpdateColumnVisibilty, this, &CustomTreeView::ShowOrHideColumnCore);
  connect(m_horHeader, &DoubleRowHeader::searchStatementChanged, this, &CustomTreeView::searchSqlStatementChanged);

  connect(m_horHeader, &HorMenuInHeader::reqSortEnabled, this, &QTreeView::setSortingEnabled);
}

bool CustomTreeView::ShowOrHideColumnCore() {
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

void CustomTreeView::InitTreeView() {  //
  StyleSheet::InitFontFamilyAndSize(this);
  ShowOrHideColumnCore();
  m_horHeader->InitFilterEditors();
  m_horHeader->RestoreHeaderState();
}
