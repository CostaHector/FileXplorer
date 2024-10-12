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
      m_DEFAULT_SECTION_SIZE_KEY{m_name + "_DEFAULT_SECTION_SIZE"},
      m_DEFAULT_COLUMN_SECTION_SIZE_KEY{m_name + "_DEFAULT_COLUMN_SECTION_SIZE"},
      m_horizontalHeaderStateKey{m_name + "_HEADER_GEOMETRY"},
      m_showVerticalHeaderKey{m_name + "_SHOW_VERTICAL_HEADER"},
      m_sortByColumnSwitchKey{m_name + "_SORT_BY_COLUMN_SWITCH"},
      m_defaultTableRowHeight{MemoryKey::TABLE_DEFAULT_SECTION_SIZE.v.toInt()},
      m_defaultTableColumnWidth{MemoryKey::TABLE_DEFAULT_COLUMN_SECTION_SIZE.v.toInt()},
      m_columnsShowSwitch{"11111,11111,11111,11111,11111,11111"} {
  if (isNameExists(m_name)) {
    qWarning("Instance Name[%s] already exist", qPrintable(m_name));
    return;
  }
  TABLES_SET.insert(m_name);
  if (PreferenceSettings().contains(m_DEFAULT_SECTION_SIZE_KEY)) {
    m_defaultTableRowHeight = PreferenceSettings().value(m_DEFAULT_SECTION_SIZE_KEY).toInt();
  }
  if (PreferenceSettings().contains(m_DEFAULT_COLUMN_SECTION_SIZE_KEY)) {
    m_defaultTableColumnWidth = PreferenceSettings().value(m_DEFAULT_COLUMN_SECTION_SIZE_KEY).toInt();
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

  const int fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  STRETCH_DETAIL_SECTION->setCheckable(true);
  STRETCH_DETAIL_SECTION->setChecked(PreferenceSettings().value(m_stretchLastSectionKey, true).toBool());
  horizontalHeader()->setStretchLastSection(STRETCH_DETAIL_SECTION->isChecked());
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
  horizontalHeader()->setDefaultSectionSize(m_defaultTableColumnWidth);

  verticalHeader()->setDefaultSectionSize(m_defaultTableRowHeight);

  ENABLE_COLUMN_SORT->setCheckable(true);
  ENABLE_COLUMN_SORT->setChecked(PreferenceSettings().value(m_sortByColumnSwitchKey, true).toBool());
  ENABLE_COLUMN_SORT->setToolTip(QString("<b>%1 (%2)</b><br/> Enable/Disable sort by click on horizontal header")
                                     .arg(ENABLE_COLUMN_SORT->text(), ENABLE_COLUMN_SORT->shortcut().toString()));
  setSortingEnabled(ENABLE_COLUMN_SORT->isChecked());

  SHOW_VERTICAL_HEADER->setCheckable(true);
  SHOW_VERTICAL_HEADER->setChecked(PreferenceSettings().value(m_showVerticalHeaderKey, true).toBool());
  SHOW_VERTICAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide or Show the number vertical header")
                                       .arg(SHOW_VERTICAL_HEADER->text(), SHOW_VERTICAL_HEADER->shortcut().toString()));

  RESIZE_COLUMN_TO_CONTENTS->setCheckable(true);
  RESIZE_COLUMN_TO_CONTENTS->setToolTip(
      QString("<b>%1 (%2)</b><br/> Resize column to contents when enabled. column width to default section  when disabled")
          .arg(RESIZE_COLUMN_TO_CONTENTS->text(), RESIZE_COLUMN_TO_CONTENTS->shortcut().toString()));

  RESIZE_ROW_TO_CONTENTS->setCheckable(true);
  RESIZE_ROW_TO_CONTENTS->setChecked(false);
  RESIZE_ROW_TO_CONTENTS->setToolTip(QString("<b>%1 (%2)</b><br/> Resize row to contents when enabled. row height interactive when disabled")
                                         .arg(RESIZE_ROW_TO_CONTENTS->text(), RESIZE_ROW_TO_CONTENTS->shortcut().toString()));

  m_horMenu->addAction(SHOW_VERTICAL_HEADER);
  m_horMenu->addSeparator();
  m_horMenu->addAction(HIDE_THIS_COLUMN);
  m_horMenu->addAction(COLUMNS_VISIBILITY);
  m_horMenu->addAction(SHOW_ALL_COLUMNS);
  m_horMenu->addSeparator();
  m_horMenu->addAction(STRETCH_DETAIL_SECTION);
  m_horMenu->addAction(ENABLE_COLUMN_SORT);
  m_horMenu->addSeparator();
  m_horMenu->addAction(RESIZE_COLUMN_TO_CONTENTS);
  m_horMenu->addAction(SET_COLS_DEFAULT_SECTION_SIZE);
  m_horMenu->setToolTipsVisible(true);

  m_verMenu->addAction(SHOW_VERTICAL_HEADER);
  m_verMenu->addSeparator();
  m_verMenu->addAction(RESIZE_ROW_TO_CONTENTS);
  m_verMenu->addAction(SET_ROWS_DEFAULT_SECTION_SIZE);
  m_verMenu->addAction(SET_MAX_ROWS_SECTION_SIZE);
  m_verMenu->setToolTipsVisible(true);

  verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

  setStyleSheet(ViewStyleSheet::TABLEVIEW_STYLESHEET);

  SubscribeHeaderActions();
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
    qWarning("Don't bind a nullptr menu");
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
    qWarning("Skip set visibility of horizontal header. model is nullptr.");
    return false;
  }
  const int tableColumnsCount = model_->columnCount();
  if (tableColumnsCount > m_columnsShowSwitch.size()) {
    qWarning("Skip set visibility of horizontal header. switchs count less than columns count.");
    return false;
  }
  for (int c = 0; c < tableColumnsCount; ++c) {
    setColumnHidden(c, m_columnsShowSwitch[c] == '0');
  }
  PreferenceSettings().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  return true;
}

bool CustomTableView::onShowHideColumn() {
  auto* model_ = this->model();
  if (model_ == nullptr) {
    qWarning("Skip set visibility of horizontal header. model is nullptr.");
    return false;
  }
  const int tableColumnsCount = model_->columnCount();
  if (m_columnsShowSwitch.size() < tableColumnsCount) {
    qWarning("switches must contain at least %d element[actual: %d]", tableColumnsCount, m_columnsShowSwitch.size());
    return false;
  }

  QString switches;
  for (int i = 0; i < horizontalHeader()->count(); ++i) {
    const QChar SEP = (i != 0 and i % SWITCHS_BATCH_COUNT == 0) ? '\n' : ',';
    if (not m_horHeaderTitlesInit) {
      m_horHeaderTitles += model_->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString() + SEP;
    }
    switches += m_columnsShowSwitch[i] + SEP;
  }
  m_horHeaderTitlesInit = true;

  bool ok = false;
  QString userInputArray =
      QInputDialog::getText(this, QString("Table column visibility(0:hide, 1:show) at least %1 element(s)\n").arg(tableColumnsCount),
                            m_horHeaderTitles, QLineEdit::Normal, switches, &ok);
  if (not ok) {
    qInfo("User cancel change horizontal header visibility");
    return false;
  }
  userInputArray.remove(',').remove('\n');
  if (userInputArray.size() < tableColumnsCount) {
    qWarning("Input must contain at least %d element[actual: %d]", tableColumnsCount, userInputArray.size());
    QMessageBox::warning(this, "Skip",
                         QString("Input must contain at least %1 element(s)[actual: %2]").arg(tableColumnsCount).arg(userInputArray.size()));
    return false;
  }
  m_columnsShowSwitch = userInputArray;
  ShowOrHideColumnCore();
  return true;
}

bool CustomTableView::onHideThisColumn() {
  const int c = GetClickedHorIndex();
  if (not(0 <= c and c < m_columnsShowSwitch.size())) {
    qWarning("invalid %dth column  not in [0, %d)", c, m_columnsShowSwitch.size());
    QMessageBox::warning(this, "Invalid column index", "Skip HideThisColumn");
    return false;
  }
  if (m_columnsShowSwitch[c] == '0') {
    qDebug("Column[%dth] is already hide. Select another column to hide", c);
    QMessageBox::information(this, QString("Column[%1th] is already hide").arg(c), "Select another column to hide");
    return true;
  }
  m_columnsShowSwitch[c] = '0';
  setColumnHidden(c, m_columnsShowSwitch[c] == '0');
  PreferenceSettings().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  return true;
}

void CustomTableView::onStretchLastSection(const bool checked) {
  horizontalHeader()->setStretchLastSection(checked);
  PreferenceSettings().setValue(m_stretchLastSectionKey, checked);
}
void CustomTableView::onEnableColumnSort(const bool enableChecked) {
  setSortingEnabled(enableChecked);
  PreferenceSettings().setValue(m_sortByColumnSwitchKey, enableChecked);
}

void CustomTableView::onResizeRowToContents(const bool checked) {
  if (checked) {
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  } else {
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    verticalHeader()->setDefaultSectionSize(m_defaultTableRowHeight);
  }
}

void CustomTableView::onResizeColumnToContents(const bool checked) {
  if (checked) {
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  } else {
    horizontalHeader()->setDefaultSectionSize(m_defaultTableColumnWidth);
  }
}

void CustomTableView::onSetRowMaxHeight() {
  bool setOk{false};
  static const int INIT_MAX_ROW_HEIGHT = verticalHeader()->maximumSectionSize();
  const int size = QInputDialog::getInt(this, "Set row max height size >=0 ", QString("current max height:%1 px").arg(INIT_MAX_ROW_HEIGHT),
                                        m_defaultTableRowHeight, 0, 10000, 20, &setOk);
  if (!setOk) {
    qWarning("User cancel resize row height");
    return;
  }
  verticalHeader()->setMaximumSectionSize(size);
  qDebug("Max row height set to %d", size);
}

void CustomTableView::onSetRowDefaultSectionSize() {
  bool setOk{false};
  const int size = QInputDialog::getInt(this, "Set default row section size >=0 ", QString("current row:%1 px").arg(m_defaultTableRowHeight),
                                        m_defaultTableRowHeight, 0, 10000, 20, &setOk);
  if (!setOk) {
    qWarning("User cancel resize row height");
    return;
  }
  m_defaultTableRowHeight = size;
  verticalHeader()->setDefaultSectionSize(size);
  PreferenceSettings().setValue(m_DEFAULT_SECTION_SIZE_KEY, size);
}

void CustomTableView::onSetColumnDefaultSectionSize() {
  bool setOk{false};
  const int size = QInputDialog::getInt(this, "Set default column section size >=0 ", QString("current column:%1 px").arg(m_defaultTableRowHeight),
                                        m_defaultTableColumnWidth, 0, 10000, 20, &setOk);
  if (!setOk) {
    qWarning("User cancel resize column height");
    return;
  }
  m_defaultTableColumnWidth = size;
  horizontalHeader()->setDefaultSectionSize(size);
  PreferenceSettings().setValue(m_DEFAULT_COLUMN_SECTION_SIZE_KEY, size);
}

void CustomTableView::onShowVerticalHeader(bool showChecked) {
  verticalHeader()->setVisible(showChecked);
  PreferenceSettings().setValue(m_showVerticalHeaderKey, showChecked);
}

void CustomTableView::onSortIndicatorChanged(int logicalIndex, Qt::SortOrder order) {
  if (not isIndicatorHoldByRestoredStateTrustable) {
    qInfo(
        "At first time, QHeaderView.restoreState() would restore state with bugs(always being descending) "
        "and triggered indicator changed. So instead, we trust value more from PreferenceSetting."
        "The first time. we trust onSortIndicatorChanged more");
    int logicalIndex =
        PreferenceSettings().value(MemoryKey::HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, MemoryKey::HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX.v).toInt();
    const QString& orderString(
        PreferenceSettings().value(MemoryKey::HEADVIEW_SORT_INDICATOR_ORDER.name, MemoryKey::HEADVIEW_SORT_INDICATOR_ORDER.v).toString());
    horizontalHeader()->setSortIndicator(logicalIndex, HEADERVIEW_SORT_INDICATOR_ORDER::string2SortOrderEnumListTable[orderString]);
    return;
  }
  PreferenceSettings().setValue(MemoryKey::HEADVIEW_SORT_INDICATOR_LOGICAL_INDEX.name, logicalIndex);
  PreferenceSettings().setValue(MemoryKey::HEADVIEW_SORT_INDICATOR_ORDER.name, HEADERVIEW_SORT_INDICATOR_ORDER::SortOrderEnum2String(order));
  qDebug("logicalIndex:%d, order:%d", logicalIndex, order);
}

void CustomTableView::InitTableView() {
  verticalHeader()->setVisible(SHOW_VERTICAL_HEADER->isChecked());
  verticalHeader()->setDefaultSectionSize(m_defaultTableRowHeight);
  verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
  // when table changed. the header may changed at same time. at a factor of that, call this func again
  horizontalHeader()->restoreState(PreferenceSettings().value(m_horizontalHeaderStateKey, QByteArray()).toByteArray());
  ShowOrHideColumnCore();
  isIndicatorHoldByRestoredStateTrustable = true;
}

void CustomTableView::onHorizontalHeaderMenuRequest(const QPoint& pnt) {
  m_horizontalHeaderSectionClicked = horizontalHeader()->logicalIndexAt(pnt);
  m_horMenu->popup(viewport()->mapToGlobal(pnt));
}
void CustomTableView::onVerticalHeaderMenuRequest(const QPoint& pnt) {
  m_verMenu->popup(viewport()->mapToGlobal(pnt));
}

void CustomTableView::SubscribeHeaderActions() {
  connect(COLUMNS_VISIBILITY, &QAction::triggered, this, &CustomTableView::onShowHideColumn);
  connect(HIDE_THIS_COLUMN, &QAction::triggered, this, &CustomTableView::onHideThisColumn);
  connect(SHOW_ALL_COLUMNS, &QAction::triggered, this, &CustomTableView::onShowAllColumn);
  connect(STRETCH_DETAIL_SECTION, &QAction::triggered, this, &CustomTableView::onStretchLastSection);
  connect(ENABLE_COLUMN_SORT, &QAction::triggered, this, &CustomTableView::onEnableColumnSort);
  connect(RESIZE_COLUMN_TO_CONTENTS, &QAction::triggered, this, &CustomTableView::onResizeColumnToContents);
  connect(SET_COLS_DEFAULT_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onSetColumnDefaultSectionSize);

  connect(SHOW_VERTICAL_HEADER, &QAction::triggered, this, &CustomTableView::onShowVerticalHeader);
  connect(RESIZE_ROW_TO_CONTENTS, &QAction::triggered, this, &CustomTableView::onResizeRowToContents);
  connect(SET_ROWS_DEFAULT_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onSetRowDefaultSectionSize);
  connect(SET_MAX_ROWS_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onSetRowMaxHeight);

  connect(verticalHeader(), &QHeaderView::customContextMenuRequested, this, &CustomTableView::onVerticalHeaderMenuRequest);
  connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &CustomTableView::onHorizontalHeaderMenuRequest);
  connect(horizontalHeader(), &QHeaderView::sectionResized, this,
          [this]() { PreferenceSettings().setValue(m_horizontalHeaderStateKey, horizontalHeader()->saveState()); });

  connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &CustomTableView::onSortIndicatorChanged);
}
