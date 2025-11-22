#include "DoubleRowHeader.h"
#include "MemoryKey.h"
#include "NoEnterLineEdit.h"
#include "PublicMacro.h"

DoubleRowHeader::DoubleRowHeader(const QString &parentTableName, QWidget *parent)
  : HorMenuInHeader{parentTableName, parent}
  , m_enableFilterKey{GetName() + "_ENABLE_FILTERS"} //
{
  _ENABLE_FILTERS = new (std::nothrow) QAction(QIcon(":img/QUICK_WHERE_FILTERS"), "Enable filters for each column", this);
  _ENABLE_FILTERS->setCheckable(true);
  _ENABLE_FILTERS->setChecked(Configuration().value(m_enableFilterKey, false).toBool());
  AddActionToMenu(_ENABLE_FILTERS);

  setSectionsClickable(true);
  setSortIndicatorShown(true);

  onToggleEnableOrDisableFilter(isFilterEnabled());
  connect(_ENABLE_FILTERS, &QAction::toggled, this, &DoubleRowHeader::onToggleEnableOrDisableFilter);
}

DoubleRowHeader::~DoubleRowHeader() {
  Configuration().setValue(m_enableFilterKey, isFilterEnabled());
}

QString DoubleRowHeader::GetColumn2SearchTemplate(const QString &columnName) {
  static QMap<QString, QString> columnName2SearchTemplate{
      //
      {"Name", R"(`Name` LIKE "%%1%")"},   //
      {"Rate", R"(`Rate` %1)"},            //
      {"Tags", R"(`Tags` LIKE "%%1%")"},   //
      {"Ori", R"(`Ori` LIKE "%%1%")"},     //
      {"Height", R"(`Height` %1)"},        //
      {"Size", R"(`Size` %1)"},            //
      {"Birth", R"(`Birth` LIKE "%%1%")"}, //
  };
  return columnName2SearchTemplate.value(columnName, "");
}

void DoubleRowHeader::UpdateSearchStatement() {
  if (!isFilterEnabled()) {
    return;
  }

  QStringList searchStatements;
  searchStatements.reserve(m_filterEditors.size());
  for (const QLineEdit *pLe : m_filterEditors) {
    const QString fieldFilterStatementTemplate = pLe->placeholderText();
    const QString fieldValue = pLe->text();
    if (fieldValue.isEmpty() || fieldFilterStatementTemplate.isEmpty()) {
      continue;
    }
    searchStatements.push_back(fieldFilterStatementTemplate.arg(fieldValue));
  }

  QString searchCmd;
  if (!searchStatements.isEmpty()) {
    searchCmd += "(";
    searchCmd += searchStatements.join(") AND (");
    searchCmd += ")";
  }
  emit searchStatementChanged(searchCmd);
}

void DoubleRowHeader::updateFilterEditorsGeometry() {
  if (!isFilterEnabled()) {
    return;
  }
  // below are when filter enabled
  if (m_filterEditors.isEmpty()) {
    return;
  }
  const int yPos = 0 + height() / 2;
  const int editorHeight = height() - yPos;
  for (int col = 0; col < m_filterEditors.size(); ++col) {
    QLineEdit *editor = m_filterEditors[col];
    if (isColumnHidden(col)) {
      editor->hide();
      continue;
    }
    const int xPos = sectionViewportPosition(col);
    editor->setGeometry(xPos, yPos, sectionSize(col), editorHeight);
    if (editor->isHidden()) { // show it if column not hide, and filter is hidden
      editor->show();
    }
  }
}

void DoubleRowHeader::onToggleEnableOrDisableFilter(bool bEnableFilter) {
  if (bEnableFilter) {
    m_connSectionResized = connect(this, &QHeaderView::sectionResized, this, &DoubleRowHeader::updateFilterEditorsGeometry);
    InitFilterEditors();
    updateFilterEditorsGeometry();
  } else {
    if (m_connSectionResized) {
      disconnect(m_connSectionResized);
    }
    hideAllLineEdit();
  }

  emit reqParentTableUpdateGeometries();
  // if (QTableView* tableView = qobject_cast<QTableView*>(parentWidget())) {
  //  QMetaObject::invokeMethod(tableView, "updateGeometries", Qt::QueuedConnection);
  // }
}

void DoubleRowHeader::resizeEvent(QResizeEvent *event) {
  QHeaderView::resizeEvent(event);
  updateFilterEditorsGeometry();
}

QSize DoubleRowHeader::sizeHint() const {
  QSize sz = QHeaderView::sizeHint();
  if (!isFilterEnabled()) {
    return sz;
  }
  return {sz.width(), sz.height() * 2};
}

void DoubleRowHeader::InitFilterEditors() {
  if (!isFilterEnabled() || !m_filterEditors.isEmpty()) {
    return;
  }
  const QAbstractItemModel *pModel = model();
  CHECK_NULLPTR_RETURN_VOID(pModel);
  QStringList titlesTempList;
  const int columnCount = pModel->columnCount();
  for (int col = 0; col < columnCount; ++col) {
    const QString &headTitle = pModel->headerData(col, Qt::Orientation::Horizontal, Qt::DisplayRole).toString();
    titlesTempList.push_back(headTitle);
    QLineEdit *filterEdit = new NoEnterLineEdit{this};
    if (isColumnHidden(col)) { // hide it right now if column in hidden
      filterEdit->hide();
    }
    filterEdit->setPlaceholderText(GetColumn2SearchTemplate(headTitle));
    filterEdit->setClearButtonEnabled(true);
    connect(filterEdit, &QLineEdit::returnPressed, this, &DoubleRowHeader::UpdateSearchStatement);
    m_filterEditors.append(filterEdit);
  }
  InitHorHeaderTitles(titlesTempList);
}

void DoubleRowHeader::hideAllLineEdit() {
  for (QLineEdit *pLe : m_filterEditors) {
    pLe->setVisible(false);
  }
}
