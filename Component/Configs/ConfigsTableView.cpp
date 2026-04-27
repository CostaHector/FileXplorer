#include "ConfigsTableView.h"
#include "ConfigsModel.h"

#include "FileTool.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"

#include <QFile>

ConfigsTableView::ConfigsTableView(const QString &instName, QWidget *parent)
  : CustomTableView{instName, parent} {
  setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);

  m_cfgModel = new (std::nothrow) ConfigsModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_cfgModel);

  mSortFilterProxy = new (std::nothrow) QSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(mSortFilterProxy);
  mSortFilterProxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  mSortFilterProxy->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  mSortFilterProxy->setSourceModel(m_cfgModel);

  setModel(mSortFilterProxy);
  InitTableView();

  mStyleSheetEditDelegate = new StyleSheetEditDelegate{ConfigsModel::ColumnE::DATA_TYPE_ROLE, ConfigsModel::EDITABLE_COLUMN, this};
  setItemDelegateForColumn(ConfigsModel::EDITABLE_COLUMN, mStyleSheetEditDelegate);

  subscribe();
}

int ConfigsTableView::GetFailedCnt() const {
  return m_cfgModel->failCount();
}

void ConfigsTableView::setFilter(const QString& filter) {
  mSortFilterProxy->setFilterFixedString(filter);
}

void ConfigsTableView::initExclusivePreferenceSetting() { //
  CustomTableView::m_defaultShowBackgroundImage = true;
}

void ConfigsTableView::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &ConfigsTableView::on_cellDoubleClicked);
  connect(m_cfgModel, &ConfigsModel::failedCountChanged, this, &ConfigsTableView::modelCfgFailedCountChanged);
}

bool ConfigsTableView::on_cellDoubleClicked(const QModelIndex &proxyIndex) const {
  if (!proxyIndex.isValid()) {
    return false;
  }

  const QModelIndex srcIndex{mSortFilterProxy->mapToSource(proxyIndex)};
  if (!m_cfgModel->isPath(srcIndex)) {
    LOG_INFO_P("Skip open", "row[%d] is not path related", srcIndex.row());
    return false;
  }
  const QString &path = m_cfgModel->filePath(srcIndex);
  if (!QFile::exists(path)) {
    LOG_INFO_P("Cannot open", "row:%d, path:%s is not a existed path", srcIndex.row(), qPrintable(path));
    return false;
  }
  return FileTool::OpenLocalFile(path);
}
