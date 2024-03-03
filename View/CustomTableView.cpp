#include "CustomTableView.h"
#include "PublicVariable.h"
#include "ViewStyleSheet.h"

#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>

QSet<QString> CustomTableView::TABLES_SET;

CustomTableView::CustomTableView(const QString& name, QWidget* parent)
    : QTableView(parent),
      m_name{name},
      m_columnVisibiltyKey{m_name + "_COLUMN_VISIBILITY"},
      m_stretchLastSectionKey{m_name + "_STRETCH_LAST_SECTION"},
      m_defaultSectionSizeKey{m_name + "_DEFAULT_SECTION_SIZE"},
      m_horizontalHeaderStateKey{m_name + "_HEADER_GEOMETRY"},
      m_showVerticalHeaderKey{m_name + "_SHOW_VERTICAL_HEADER"},
      m_defaultTableRowHeight{MemoryKey::TABLE_DEFAULT_SECTION_SIZE.v.toInt()},
      m_columnsShowSwitch{"11111,11111,11111,11111,11111,11111"} {
  if (isNameExists(m_name)) {
    qWarning("Instance Name[%s] already exist", qPrintable(m_name));
    return;
  }
  TABLES_SET.insert(m_name);
  if (PreferenceSettings().contains(m_defaultSectionSizeKey)) {
    m_defaultTableRowHeight = PreferenceSettings().value(m_defaultSectionSizeKey).toInt();
  }
  if (PreferenceSettings().contains(m_columnVisibiltyKey)) {
    m_columnsShowSwitch = PreferenceSettings().value(m_columnVisibiltyKey).toString();
  }

  setAlternatingRowColors(true);
  setShowGrid(false);

  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

  setDragDropMode(QAbstractItemView::NoDragDrop);
  setEditTriggers(QAbstractItemView::EditKeyPressed);

  const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  STRETCH_DETAIL_SECTION->setCheckable(true);
  STRETCH_DETAIL_SECTION->setChecked(PreferenceSettings().value(m_stretchLastSectionKey, true).toBool());
  horizontalHeader()->setStretchLastSection(STRETCH_DETAIL_SECTION->isChecked());
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  SHOW_VERTICAL_HEADER->setCheckable(true);
  SHOW_VERTICAL_HEADER->setChecked(PreferenceSettings().value(m_showVerticalHeaderKey, true).toBool());
  SHOW_VERTICAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide or Show the number vertical header")
                                       .arg(SHOW_VERTICAL_HEADER->text(), SHOW_VERTICAL_HEADER->shortcut().toString()));

  RESIZE_ROWS_TO_CONTENT->setCheckable(true);
  RESIZE_ROWS_TO_CONTENT->setChecked(false);
  RESIZE_ROWS_TO_CONTENT->setToolTip(QString("<b>%1 (%2)</b><br/> Resize row to content when enabled. row height interactive when disabled")
                                         .arg(RESIZE_ROWS_TO_CONTENT->text(), RESIZE_ROWS_TO_CONTENT->shortcut().toString()));

  connect(COLUMNS_VISIBILITY, &QAction::triggered, this, &CustomTableView::onShowHideColumn);
  connect(HIDE_THIS_COLUMN, &QAction::triggered, this, &CustomTableView::onHideThisColumn);
  connect(SHOW_ALL_COLUMNS, &QAction::triggered, this, &CustomTableView::onShowAllColumn);
  connect(STRETCH_DETAIL_SECTION, &QAction::triggered, this, &CustomTableView::onStretchLastSection);

  connect(SHOW_VERTICAL_HEADER, &QAction::triggered, this, &CustomTableView::onShowVerticalHeader);
  connect(RESIZE_ROWS_TO_CONTENT, &QAction::triggered, this, &CustomTableView::onResizeRowToContents);
  connect(RESIZE_ROWS_DEFAULT_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onResizeRowDefaultSectionSize);

  m_horMenu->addActions({SHOW_VERTICAL_HEADER, HIDE_THIS_COLUMN, COLUMNS_VISIBILITY, SHOW_ALL_COLUMNS, STRETCH_DETAIL_SECTION});
  m_horMenu->setToolTipsVisible(true);
  m_verMenu->addActions({SHOW_VERTICAL_HEADER, RESIZE_ROWS_DEFAULT_SECTION_SIZE, RESIZE_ROWS_TO_CONTENT});
  m_verMenu->setToolTipsVisible(true);

  verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(verticalHeader(), &QHeaderView::customContextMenuRequested, this,
          [this](const QPoint pnt) { m_verMenu->popup(viewport()->mapToGlobal(pnt)); });

  horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, [this](const QPoint pnt) {
    m_horizontalHeaderSectionClicked = horizontalHeader()->logicalIndexAt(pnt);
    m_horMenu->popup(viewport()->mapToGlobal(pnt));
  });
  connect(horizontalHeader(), &QHeaderView::sectionResized, this,
          [this]() { PreferenceSettings().setValue(m_horizontalHeaderStateKey, horizontalHeader()->saveState()); });

  setStyleSheet(ViewStyleSheet::TABLEVIEW_STYLESHEET);
}

void CustomTableView::contextMenuEvent(QContextMenuEvent* event) {
  if (m_menu) {
    m_menu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
    return;
  }
  QTableView::contextMenuEvent(event);
}

void CustomTableView::BindMenu(QMenu* menu) {
  if (menu == nullptr) {
    return;
  }
  m_menu = menu;
}

void CustomTableView::AppendVerticalHeaderMenuAGS(QActionGroup* extraAgs) {
  if (extraAgs == nullptr or extraAgs->actions().isEmpty()) {
    qDebug("Skip. nullptr or no actions in ActionGroup.");
    return;
  }
  m_verMenu->addSeparator();
  m_verMenu->addActions(extraAgs->actions());
}

void CustomTableView::AppendHorizontalHeaderMenuAGS(QActionGroup* extraAgs) {
  if (extraAgs == nullptr or extraAgs->actions().isEmpty()) {
    qDebug("Skip. nullptr or no actions in ActionGroup.");
    return;
  }
  m_horMenu->addSeparator();
  m_horMenu->addActions(extraAgs->actions());
}

bool CustomTableView::ShowOrHideColumnCore() {
  if (m_columnsShowSwitch.isEmpty()) {
    qDebug("Skip set visibility of horizontal header. switch batch is empty.");
    return false;
  }
  auto* model_ = this->model();
  if (model_ == nullptr) {
    qDebug("Skip set visibility of horizontal header. model is nullptr.");
    return false;
  }
  const int tableColumnsCount = model_->columnCount();
  if (Column2RelIndex(tableColumnsCount - 1) >= m_columnsShowSwitch.size()) {
    qDebug("Skip set visibility of horizontal header. N + N/BATCH_CNT must < lens of column show switch.");
    return false;
  }
  for (int c = 0; c < tableColumnsCount; ++c) {
    setColumnHidden(c, isColumnithHidden(c));
  }

  PreferenceSettings().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  return true;
}

bool CustomTableView::onShowHideColumn() {
  auto* model_ = this->model();
  if (model_ == nullptr) {
    qDebug("Skip set visibility of horizontal header. model is nullptr.");
    return false;
  }
  bool ok = false;
  const QString& showHideSwitchArray = QInputDialog::getText(this, "Performer table column visibility(0:hide, 1:show)",
                                                             "Extra element will be dismissed simply", QLineEdit::Normal, m_columnsShowSwitch, &ok);
  if (not ok) {
    qInfo("User cancel change horizontal header visibility");
    return false;
  }
  if (Column2RelIndex(model_->columnCount() - 1) >= showHideSwitchArray.size()) {
    QMessageBox::warning(this, "Change horizontal header visibility failed", "must meet: N+N/5 < lens of input array");
    qDebug("Skip Change horizontal header visibility. N(%d) + N/BATCH_CNT(%d) must < lens of column show switch(%d).", model_->columnCount() - 1,
           SWITCHS_BATCH_COUNT, showHideSwitchArray.size());
    return false;
  }
  m_columnsShowSwitch = showHideSwitchArray;
  ShowOrHideColumnCore();
  return true;
}

bool CustomTableView::onHideThisColumn() {
  int c = GetClickedHorIndex();
  int relIndex = Column2RelIndex(c);
  if (relIndex < 0 or relIndex >= m_columnsShowSwitch.size()) {
    qDebug("invalid %dth column => %d not in [0, %d)", c, relIndex, m_columnsShowSwitch.size());
    QMessageBox::warning(this, "Invalid column index", "Skip HideThisColumn");
    return false;
  }
  if (m_columnsShowSwitch[relIndex] == '0') {
    qDebug("Column[%dth=>%d] already hide. Select another column to hide", c, relIndex);
    QMessageBox::warning(this, QString("Column[%1th=>%2] already hide").arg(c).arg(relIndex), "Select another column to hide");
    return true;
  }
  m_columnsShowSwitch[relIndex] = '0';
  setColumnHidden(c, isColumnithHidden(c));
  PreferenceSettings().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  return true;
}

void CustomTableView::onStretchLastSection(const bool checked) {
  horizontalHeader()->setStretchLastSection(checked);
  PreferenceSettings().setValue(m_stretchLastSectionKey, checked);
}

void CustomTableView::onResizeRowToContents(const bool checked) {
  if (checked) {
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  } else {
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    verticalHeader()->setDefaultSectionSize(m_defaultTableRowHeight);
  }
}

void CustomTableView::onResizeRowDefaultSectionSize() {
  const int size = QInputDialog::getInt(this, "Resize Row Default Section size >=0 ", QString("default size:%1").arg(m_defaultTableRowHeight),
                                        m_defaultTableRowHeight);
  if (size < 0) {
    qDebug("User cancel resize row height");
    return;
  }
  m_defaultTableRowHeight = size;
  verticalHeader()->setDefaultSectionSize(size);
  PreferenceSettings().setValue(m_defaultSectionSizeKey, size);
}

void CustomTableView::onShowVerticalHeader(bool showChecked) {
  verticalHeader()->setVisible(showChecked);
  PreferenceSettings().setValue(m_showVerticalHeaderKey, showChecked);
}

void CustomTableView::InitTableView() {
  verticalHeader()->setVisible(SHOW_VERTICAL_HEADER->isChecked());
  verticalHeader()->setDefaultSectionSize(m_defaultTableRowHeight);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  horizontalHeader()->restoreState(PreferenceSettings().value(m_horizontalHeaderStateKey, QByteArray()).toByteArray());
  // when table changed. the header may changed at same time. at a factor of that, call this func again
  ShowOrHideColumnCore();
}
