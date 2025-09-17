#include "CustomTableView.h"
#include "ColumnVisibilityDialog.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "ViewHelper.h"

#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QActionGroup>

QSet<QString> CustomTableView::TABLES_SET;
// a bunch of widget with same model should share the only one setting. e.g., HAR_TABLEVIEW
QSet<QString> CustomTableView::SETTING_SHARING_WIDGET {"HAR_TABLEVIEW"};

CustomTableView::CustomTableView(const QString& name, QWidget* parent)
  : QTableView(parent),
  m_name{name},
  m_columnVisibiltyKey{m_name + "_COLUMN_VISIBILITY"},
  m_stretchLastSectionKey{m_name + "_STRETCH_LAST_SECTION"},
  m_DEFAULT_SECTION_SIZE_KEY{m_name + "_DEFAULT_SECTION_SIZE"},
  m_DEFAULT_COLUMN_SECTION_SIZE_KEY{m_name + "_DEFAULT_COLUMN_SECTION_SIZE"},
  m_horizontalHeaderStateKey{m_name + "_HEADER_GEOMETRY"},
  m_showHorizontalHeaderKey{m_name + "_SHOW_HORIZONTAL_HEADER"},
  m_showVerticalHeaderKey{m_name + "_SHOW_VERTICAL_HEADER"},
  m_sortByColumnSwitchKey{m_name + "_SORT_BY_COLUMN_SWITCH"},
  m_rowResizeToContentKey{m_name + "_ROW_RESIZE_TO_CONTENT"},
  m_defaultTableRowHeight{MemoryKey::TABLE_DEFAULT_SECTION_SIZE.v.toInt()},
  m_defaultTableColumnWidth{MemoryKey::TABLE_DEFAULT_COLUMN_SECTION_SIZE.v.toInt()},
  m_columnsShowSwitch{QString{50, QChar{'1'}}} {
  if (!SETTING_SHARING_WIDGET.contains(m_name) && isNameExists(m_name)) { // not in sharing list, but name already find
#ifdef RUNNING_UNIT_TESTS
    LOG_D("Instance Name[%s] already exist, QSetting may conflict", qPrintable(m_name));
#else
    LOG_W("Instance Name[%s] already exist, QSetting may conflict", qPrintable(m_name));
    return;
#endif
  }
  TABLES_SET.insert(m_name);
  if (Configuration().contains(m_DEFAULT_SECTION_SIZE_KEY)) {
    m_defaultTableRowHeight = Configuration().value(m_DEFAULT_SECTION_SIZE_KEY).toInt();
  }
  if (Configuration().contains(m_DEFAULT_COLUMN_SECTION_SIZE_KEY)) {
    m_defaultTableColumnWidth = Configuration().value(m_DEFAULT_COLUMN_SECTION_SIZE_KEY).toInt();
  }
  if (Configuration().contains(m_columnVisibiltyKey)) {
    m_columnsShowSwitch = Configuration().value(m_columnVisibiltyKey).toString();
  }

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

  STRETCH_DETAIL_SECTION->setCheckable(true);
  STRETCH_DETAIL_SECTION->setChecked(Configuration().value(m_stretchLastSectionKey, true).toBool());
  horizontalHeader()->setStretchLastSection(STRETCH_DETAIL_SECTION->isChecked());
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
  horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

  horizontalHeader()->setDefaultSectionSize(m_defaultTableColumnWidth);

  ENABLE_COLUMN_SORT->setCheckable(true);
  ENABLE_COLUMN_SORT->setChecked(Configuration().value(m_sortByColumnSwitchKey, true).toBool());
  ENABLE_COLUMN_SORT->setToolTip(QString("<b>%1 (%2)</b><br/> Enable/Disable sort by click on horizontal header").arg(ENABLE_COLUMN_SORT->text(), ENABLE_COLUMN_SORT->shortcut().toString()));
  setSortingEnabled(ENABLE_COLUMN_SORT->isChecked());

  SHOW_HORIZONTAL_HEADER->setCheckable(true);
  SHOW_HORIZONTAL_HEADER->setChecked(Configuration().value(m_showHorizontalHeaderKey, true).toBool());
  SHOW_HORIZONTAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide or Show the horizontal header").arg(SHOW_VERTICAL_HEADER->text(), SHOW_VERTICAL_HEADER->shortcut().toString()));

  SHOW_VERTICAL_HEADER->setCheckable(true);
  SHOW_VERTICAL_HEADER->setChecked(Configuration().value(m_showVerticalHeaderKey, true).toBool());
  SHOW_VERTICAL_HEADER->setToolTip(QString("<b>%1 (%2)</b><br/> Hide or Show the vertical header").arg(SHOW_VERTICAL_HEADER->text(), SHOW_VERTICAL_HEADER->shortcut().toString()));

  RESIZE_COLUMN_TO_CONTENTS->setCheckable(true);
  RESIZE_COLUMN_TO_CONTENTS->setToolTip(QString("<b>%1 (%2)</b><br/> Resize column to contents when enabled. column width to default section  when disabled")
                                            .arg(RESIZE_COLUMN_TO_CONTENTS->text(), RESIZE_COLUMN_TO_CONTENTS->shortcut().toString()));

  RESIZE_ROW_TO_CONTENTS->setCheckable(true);
  RESIZE_ROW_TO_CONTENTS->setChecked(Configuration().value(m_rowResizeToContentKey, true).toBool());
  RESIZE_ROW_TO_CONTENTS->setToolTip(
      QString("<b>%1 (%2)</b><br/> Resize row to contents when enabled. row height interactive when disabled").arg(RESIZE_ROW_TO_CONTENTS->text(), RESIZE_ROW_TO_CONTENTS->shortcut().toString()));

  m_horMenu->addAction(SHOW_VERTICAL_HEADER);
  m_horMenu->addAction(SHOW_HORIZONTAL_HEADER);
  m_horMenu->addAction(COLUMNS_VISIBILITY);
  m_horMenu->addSeparator();
  m_horMenu->addAction(HIDE_THIS_COLUMN);
  m_horMenu->addSeparator();
  m_horMenu->addAction(STRETCH_DETAIL_SECTION);
  m_horMenu->addAction(ENABLE_COLUMN_SORT);
  m_horMenu->addSeparator();
  m_horMenu->addAction(RESIZE_COLUMN_TO_CONTENTS);
  m_horMenu->addAction(SET_COLS_DEFAULT_SECTION_SIZE);
  m_horMenu->setToolTipsVisible(true);

  m_verMenu->addAction(SHOW_VERTICAL_HEADER);
  m_verMenu->addAction(SHOW_HORIZONTAL_HEADER);
  m_verMenu->addAction(COLUMNS_VISIBILITY);
  m_verMenu->addSeparator();
  m_verMenu->addAction(RESIZE_ROW_TO_CONTENTS);
  m_verMenu->addAction(SET_ROWS_DEFAULT_SECTION_SIZE);
  m_verMenu->addAction(SET_MAX_ROWS_SECTION_SIZE);
  m_verMenu->setToolTipsVisible(true);

  verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
  horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

  SubscribeHeaderActions();
}

void CustomTableView::contextMenuEvent(QContextMenuEvent* event) {
  if (m_menu != nullptr) {
    m_menu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
    return;
  }
  QTableView::contextMenuEvent(event);
}

void CustomTableView::BindMenu(QMenu* menu) {
  CHECK_NULLPTR_RETURN_VOID(menu)
  m_menu = menu;
}

void CustomTableView::AppendVerticalHeaderMenuAGS(QActionGroup* extraAgs) {
  if (extraAgs == nullptr or extraAgs->actions().isEmpty()) {
    LOG_D("Skip. nullptr or no actions in ActionGroup.");
    return;
  }
  m_verMenu->addSeparator();
  m_verMenu->addActions(extraAgs->actions());
}

void CustomTableView::AppendHorizontalHeaderMenuAGS(QActionGroup* extraAgs) {
  if (extraAgs == nullptr or extraAgs->actions().isEmpty()) {
    LOG_D("Skip. nullptr or no actions in ActionGroup.");
    return;
  }
  m_horMenu->addSeparator();
  m_horMenu->addActions(extraAgs->actions());
}

bool CustomTableView::ShowOrHideColumnCore() {
  if (m_columnsShowSwitch.isEmpty()) {
    LOG_D("Skip set visibility of horizontal header. switch batch is empty.");
    return false;
  }
  auto* model_ = this->model();
  CHECK_NULLPTR_RETURN_FALSE(model_)
  const int tableColumnsCount = model_->columnCount();
  if (tableColumnsCount > m_columnsShowSwitch.size()) {
    LOG_W("Skip set visibility of horizontal header. switchs count less than columns count.");
    return false;
  }
  for (int c = 0; c < tableColumnsCount; ++c) {
    setColumnHidden(c, m_columnsShowSwitch[c] == '0');
  }
  Configuration().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  return true;
}

bool CustomTableView::onShowHideColumn() {
  auto* model_ = this->model();
  CHECK_NULLPTR_RETURN_FALSE(model_)

  const int tableColumnsCount {horizontalHeader()->count()};
  QString tempSwitches;
  if (tableColumnsCount < m_columnsShowSwitch.size()) {
    tempSwitches = m_columnsShowSwitch.left(tableColumnsCount);
  } else {
    tempSwitches += m_columnsShowSwitch;
    tempSwitches += QString{tableColumnsCount - m_columnsShowSwitch.size(), QChar{'1'}};
  }
  // get column title(s)
  if (m_horHeaderTitles.size() != tableColumnsCount) {
    m_horHeaderTitles.clear();
    for (int i = 0; i < tableColumnsCount; ++i) {
      m_horHeaderTitles.push_back(model_->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
    }
  }

  ColumnVisibilityDialog dialog{m_horHeaderTitles, tempSwitches, m_name, this};
  if (dialog.exec() != QDialog::Accepted) {
    LOG_I("User canceled column visibility change");
    return false;
  }
  m_columnsShowSwitch = dialog.getSwitches();
  ShowOrHideColumnCore();
  return true;
}

bool CustomTableView::onHideThisColumn() {
  const int c = GetClickedHorIndex();
  if (!(0 <= c && c < m_columnsShowSwitch.size())) {
    LOG_WARN_P("[Skip] HideThisColumn", "Invalid column index[%d]", c);
    return false;
  }
  if (m_columnsShowSwitch[c] == '0') {
    LOG_INFO_P("[Skip] Column already hide", "Select another column[not %d] to hide", c);
    return true;
  }
  m_columnsShowSwitch[c] = '0';
  setColumnHidden(c, true);
  Configuration().setValue(m_columnVisibiltyKey, m_columnsShowSwitch);
  return true;
}

void CustomTableView::onStretchLastSection(const bool checked) {
  horizontalHeader()->setStretchLastSection(checked);
  Configuration().setValue(m_stretchLastSectionKey, checked);
}
void CustomTableView::onEnableColumnSort(const bool enableChecked) {
  setSortingEnabled(enableChecked);
  Configuration().setValue(m_sortByColumnSwitchKey, enableChecked);
}

void CustomTableView::onResizeRowToContents(const bool checked) {
  Configuration().setValue(m_rowResizeToContentKey, checked);
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
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    horizontalHeader()->setDefaultSectionSize(m_defaultTableColumnWidth);
  }
}

void CustomTableView::onSetRowMaxHeight() {
  bool setOk{false};
  static const int INIT_MAX_ROW_HEIGHT = verticalHeader()->maximumSectionSize();
  const int size = QInputDialog::getInt(this, "Set row max height size >=0 ", QString("current max height:%1 px").arg(INIT_MAX_ROW_HEIGHT), m_defaultTableRowHeight, 0, 10000, 20, &setOk);
  if (!setOk) {
    LOG_W("User cancel resize row height");
    return;
  }
  verticalHeader()->setMaximumSectionSize(size);
  LOG_D("Max row height set to %d", size);
}

void CustomTableView::onSetRowDefaultSectionSize() {
  bool setOk{false};
  const int size = QInputDialog::getInt(this, "Set default row section size >=0 ", QString("current row:%1 px").arg(m_defaultTableRowHeight), m_defaultTableRowHeight, 0, 10000, 20, &setOk);
  if (!setOk) {
    LOG_W("User cancel resize row height");
    return;
  }
  m_defaultTableRowHeight = size;
  verticalHeader()->setDefaultSectionSize(size);
  Configuration().setValue(m_DEFAULT_SECTION_SIZE_KEY, size);
}

void CustomTableView::onSetColumnDefaultSectionSize() {
  bool setOk{false};
  const int size = QInputDialog::getInt(this, "Set default column section size >=0 ", QString("current column:%1 px").arg(m_defaultTableRowHeight), m_defaultTableColumnWidth, 0, 10000, 20, &setOk);
  if (!setOk) {
    LOG_W("User cancel resize column height");
    return;
  }
  m_defaultTableColumnWidth = size;
  horizontalHeader()->setDefaultSectionSize(size);
  Configuration().setValue(m_DEFAULT_COLUMN_SECTION_SIZE_KEY, size);
}

void CustomTableView::onShowHorizontalHeader(bool showChecked) {
  horizontalHeader()->setVisible(showChecked);
  Configuration().setValue(m_showHorizontalHeaderKey, showChecked);
}

void CustomTableView::onShowVerticalHeader(bool showChecked) {
  verticalHeader()->setVisible(showChecked);
  Configuration().setValue(m_showVerticalHeaderKey, showChecked);
}

void CustomTableView::onHorizontalHeaderChanged() const {
  Configuration().setValue(m_horizontalHeaderStateKey, horizontalHeader()->saveState());
}
void CustomTableView::onVerticalHeaderChanged() const {}

void CustomTableView::InitTableView(const bool bHideShowCol) {
  horizontalHeader()->setVisible(SHOW_HORIZONTAL_HEADER->isChecked());
  verticalHeader()->setVisible(SHOW_VERTICAL_HEADER->isChecked());
  if (Configuration().value(m_rowResizeToContentKey, false).toBool()) {
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  } else {
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);
    verticalHeader()->setDefaultSectionSize(m_defaultTableRowHeight);
  }
  horizontalHeader()->restoreState(Configuration().value(m_horizontalHeaderStateKey, QByteArray{}).toByteArray());
  if (bHideShowCol) {
    ShowOrHideColumnCore();
  }
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
  connect(STRETCH_DETAIL_SECTION, &QAction::triggered, this, &CustomTableView::onStretchLastSection);
  connect(ENABLE_COLUMN_SORT, &QAction::triggered, this, &CustomTableView::onEnableColumnSort);
  connect(RESIZE_COLUMN_TO_CONTENTS, &QAction::triggered, this, &CustomTableView::onResizeColumnToContents);
  connect(SET_COLS_DEFAULT_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onSetColumnDefaultSectionSize);

  connect(SHOW_HORIZONTAL_HEADER, &QAction::triggered, this, &CustomTableView::onShowHorizontalHeader);
  connect(SHOW_VERTICAL_HEADER, &QAction::triggered, this, &CustomTableView::onShowVerticalHeader);
  connect(RESIZE_ROW_TO_CONTENTS, &QAction::triggered, this, &CustomTableView::onResizeRowToContents);
  connect(SET_ROWS_DEFAULT_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onSetRowDefaultSectionSize);
  connect(SET_MAX_ROWS_SECTION_SIZE, &QAction::triggered, this, &CustomTableView::onSetRowMaxHeight);

  connect(verticalHeader(), &QHeaderView::customContextMenuRequested, this, &CustomTableView::onVerticalHeaderMenuRequest);
  connect(horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &CustomTableView::onHorizontalHeaderMenuRequest);
  connect(horizontalHeader(), &QHeaderView::sectionResized, this, &CustomTableView::onHorizontalHeaderChanged);
  connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &CustomTableView::onHorizontalHeaderChanged);
}

void CustomTableView::mousePressEvent(QMouseEvent* event) {
  if (View::onMouseSidekeyBackwardForward(event->modifiers(), event->button())) {
    event->accept();
    return;
  }
  QTableView::mousePressEvent(event);
}
