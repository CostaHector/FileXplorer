#include "ConfigsTableView.h"
#include "FileTool.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include <QFile>

ConfigsTableView::ConfigsTableView(const QString &instName, QWidget *parent)
  : CustomTableView{instName, parent} {
  setEditTriggers(QAbstractItemView::EditTrigger::EditKeyPressed);

  m_alertModel = new (std::nothrow) ConfigsModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_alertModel);

  setModel(m_alertModel);
  InitTableView();

  mStyleSheetEditDelegate = new StyleSheetEditDelegate{ConfigsModel::ColumnE::DATA_TYPE_ROLE, ConfigsModel::EDITABLE_COLUMN, this};
  setItemDelegateForColumn(ConfigsModel::EDITABLE_COLUMN, mStyleSheetEditDelegate);

  subscribe();
}

void ConfigsTableView::initExclusivePreferenceSetting() { //
  CustomTableView::m_defaultShowBackgroundImage = true;
}

void ConfigsTableView::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &ConfigsTableView::on_cellDoubleClicked);
}

bool ConfigsTableView::on_cellDoubleClicked(const QModelIndex &clickedIndex) const {
  if (!clickedIndex.isValid()) {
    return false;
  }
  if (!m_alertModel->isPath(clickedIndex)) {
    LOG_INFO_P("Skip open", "row[%d] is not path related", clickedIndex.row());
    return false;
  }
  const QString &path = m_alertModel->filePath(clickedIndex);
  if (!QFile::exists(path)) {
    LOG_INFO_P("Cannot open", "row:%d, path:%s is not a existed path", clickedIndex.row(), qPrintable(path));
    return false;
  }
  return FileTool::OpenLocalFile(path);
}
