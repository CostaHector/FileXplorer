#include "RecycleBinWidget.h"
#include <QDesktopServices>
#include <QSqlQuery>

#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RecycleBinActions.h"

#include "Component/NotificatorFrame.h"
#include "FileOperation/RecycleBinHelper.h"
#include "UndoRedo.h"
#include "public/DisplayEnhancement.h"

QVariant TrashbinModel::data(const QModelIndex& idx, int role) const {
  if (not idx.isValid()) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    if (idx.column() == RECYCLE_HEADER_KEY::SIZE_INDEX)
      return FILE_SIZE_DSP::toHumanReadFriendly(QSqlTableModel::data(idx, Qt::ItemDataRole::DisplayRole).toUInt());
  } else if (role == Qt::DecorationRole) {
    if (idx.column() == RECYCLE_HEADER_KEY::DB_NAME_INDEX) {
      return m_iconProvider.icon(fileName(idx));
    }
  } else if (role == Qt::ForegroundRole) {
    if (idx.column() == RECYCLE_HEADER_KEY::DB_NAME_INDEX) {
      if (not QFile::exists(fullPath(idx))) {
        return QBrush(Qt::GlobalColor::lightGray);
      }
    }
  }
  return QSqlTableModel::data(idx, role);
}

QString TrashbinModel::fileName(const QModelIndex& curIndex) const {
  const QModelIndex& nameIndex =
      (curIndex.column() == RECYCLE_HEADER_KEY::DB_NAME_INDEX ? curIndex : index(curIndex.row(), RECYCLE_HEADER_KEY::DB_NAME_INDEX));
  return data(nameIndex, Qt::ItemDataRole::DisplayRole).toString();
}
QString TrashbinModel::oldLocationFolder(const QModelIndex& curIndex) const {
  return data(index(curIndex.row(), RECYCLE_HEADER_KEY::OLD_LOCATION_INDEX), Qt::ItemDataRole::DisplayRole).toString();
}
QString TrashbinModel::tempLocationFolder(const QModelIndex& curIndex) const {
  return data(index(curIndex.row(), RECYCLE_HEADER_KEY::TEMP_LOCATION_INDEX), Qt::ItemDataRole::DisplayRole).toString();
}

QString TrashbinModel::fullPath(const QModelIndex& curIndex) const {
  return QDir(tempLocationFolder(curIndex)).absoluteFilePath(fileName(curIndex));
}

void TrashbinModel::refresh() {
  for (int i = rowCount() - 1; i > -1; --i) {
    if (not QFile::exists(fullPath(index(i, 0)))) {
      removeRow(i);
    }
  }
}

RecycleBinWidget::RecycleBinWidget(QWidget* parent)
    : QMainWindow(parent), m_recycleBinMenu{g_recycleBinAg().getRecycleBinMenu()}, m_recycleBinToolBar{g_recycleBinAg().getRecycleBinToolBar()} {
  m_recycleBinToolBar->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_recycleBinToolBar);

  auto con = GetSqlRecycleBinDB();
  if (not con.isOpen()) {
    qWarning("Open connection failed");
    return;
  }
  if (not con.tables().contains(RecycleBinHelper::RECYCLE_ITEMS_TABLE)) {
    const bool tableCreateResult = createRecycleTable();
    if (not tableCreateResult) {
      qWarning("Table Create failed");
      return;
    }
  }

  m_view = new CustomTableView{"Trashbin Window"};
  m_view->BindMenu(m_recycleBinMenu);
  m_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

  m_model = new TrashbinModel(nullptr, con);
  m_model->setTable(RecycleBinHelper::RECYCLE_ITEMS_TABLE);
  m_model->submitAll();

  m_view->setModel(m_model);

  setCentralWidget(m_view);

  m_view->InitTableView();
  setWindowTitle(g_recycleBinAg().RECYLE_BIN_WIDGET->text());
  ChangeWindowIcon();

  subscribe();
}

void RecycleBinWidget::ChangeWindowIcon() {
  if (m_model->rowCount() == 0) {
    g_recycleBinAg().RECYLE_BIN_WIDGET->setIcon(QIcon(":/themes/CLEAN_STATUS"));
    setWindowIcon(QIcon(":/themes/CLEAN_STATUS"));
    return;
  }
  g_recycleBinAg().RECYLE_BIN_WIDGET->setIcon(QIcon(":/themes/DIRTY_STATUS"));
  setWindowIcon(QIcon(":/themes/DIRTY_STATUS"));
}

void RecycleBinWidget::closeEvent(QCloseEvent* event) {
  g_recycleBinAg().RECYLE_BIN_WIDGET->setChecked(false);
  QMainWindow::closeEvent(event);
}

void RecycleBinWidget::onSubmitAndRefresh() {
  m_model->refresh();
  m_model->submitAll();
  ChangeWindowIcon();
}

bool RecycleBinWidget::onDeleteAll() {
  const int rowCount = m_model->rowCount();
  if (rowCount == 0) {
    qInfo("Skip. Nothing need to delete");
    Notificator::information("Skip", "Nothing need to delete");
    return true;
  }
  if (!queryForConfirm("Confirm delete all item(s)?", QString("Will Delete %1 item(s) permanently!!!").arg(rowCount))) {
    return true;
  }
  return deleteItems(GetAllRows());
}

bool RecycleBinWidget::onRestoreAll() {
  const int rowCount = m_model->rowCount();
  if (rowCount == 0) {
    qInfo("Skip. Nothing need to recycle");
    Notificator::information("Skip", "Nothing need to recycle");
    return true;
  }
  return restoreItems(GetAllRows());
}

bool RecycleBinWidget::onDeleteSelection() {
  const auto& selectedRows = m_view->selectionModel()->selectedRows();
  const int rowCount = selectedRows.size();
  if (rowCount == 0) {
    qInfo("Skip. Nothing need to delete");
    Notificator::information("Skip", "Nothing need to delete");
    return true;
  }
  if (!queryForConfirm("Confirm delete selected item(s)?", QString("Will Delete %1 item(s) permanently!!!").arg(rowCount))) {
    return true;
  }
  return deleteItems(selectedRows);
}

bool RecycleBinWidget::onRestoreSelection() {
  const auto& selectedRows = m_view->selectionModel()->selectedRows();
  const int rowCount = selectedRows.size();
  if (rowCount == 0) {
    qInfo("Skip. Nothing need to recycle");
    Notificator::information("Skip", "Nothing need to recycle");
    return true;
  }
  return restoreItems(selectedRows);
}

void RecycleBinWidget::subscribe() {
  connect(g_recycleBinAg().RESTORE_SELECTED_ITEMS, &QAction::triggered, this, &RecycleBinWidget::onRestoreSelection);
  connect(g_recycleBinAg().DELETE_SELECTED_ITEMS, &QAction::triggered, this, &RecycleBinWidget::onDeleteSelection);
  connect(g_recycleBinAg().EMPTY_RECYCLE_BIN, &QAction::triggered, this, &RecycleBinWidget::onDeleteAll);
  connect(g_recycleBinAg().RESTORE_ALL_ITEMS, &QAction::triggered, this, &RecycleBinWidget::onRestoreAll);

  connect(g_recycleBinAg().SUBMIT_AND_REFRESH, &QAction::triggered, this, &RecycleBinWidget::onSubmitAndRefresh);

  connect(g_recycleBinAg().OPEN_DB_IN_APP, &QAction::triggered, this, [this]() {
    if (not QFile::exists(SystemPath::RECYCLE_BIN_DATABASE)) {
      QMessageBox::information(this, "open failed", QString("[%1] not exists. \nCreate it first").arg(SystemPath::PEFORMERS_DATABASE));
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(SystemPath::RECYCLE_BIN_DATABASE));
  });
}

bool RecycleBinWidget::deleteItems(const QModelIndexList& indexes) {
  // delete subitems first
  const auto& ans = SubdirItemsFirstSort(m_model, indexes);

  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  cmds.reserve(indexes.size());

  for (int i = 0; i < ans.names.size(); ++i) {
    const QFileInfo fi(QDir(ans.tempPaths[i]).absoluteFilePath(ans.names[i]));
    if (not fi.exists()) {
      continue;
    }
    if (fi.isDir()) {
      cmds.append({"rmdir", ans.tempPaths[i], ans.names[i]});
    } else {
      cmds.append({"rmfile", ans.tempPaths[i], ans.names[i]});
    }
  }
  const bool isSucceed = g_undoRedo.Do(cmds);
  if (isSucceed) {
    Notificator::goodNews(QString("Delete [%1/%2] all succeed"), "");
  } else {
    Notificator::badNews(QString("Delete [%1/%2] some failed"), "");
  }
  for (const auto& ind : indexes) {
    m_model->removeRow(ind.row());
  }
  m_model->submitAll();
  ChangeWindowIcon();
  return true;
}

bool RecycleBinWidget::restoreItems(const QModelIndexList& indexes) {
  // recover subitems first
  const auto& ans = SubdirItemsFirstSort(m_model, indexes);
  FileOperatorType::BATCH_COMMAND_LIST_TYPE cmds;
  cmds.reserve(indexes.size());

  for (int i = 0; i < ans.names.size(); ++i) {
    const QFileInfo fi(QDir(ans.tempPaths[i]).absoluteFilePath(ans.names[i]));
    if (not fi.exists()) {
      continue;
    }
    if (fi.isDir()) {
      cmds.append({"rename", ans.tempPaths[i], ans.names[i], ans.oldPaths[i], ans.names[i]});
    } else {
      cmds.append({"rename", ans.tempPaths[i], ans.names[i], ans.oldPaths[i], ans.names[i]});
    }
  }
  const bool isSucceed = g_undoRedo.Do(cmds);
  if (isSucceed) {
    Notificator::goodNews(QString("Restore [%1/%2] all succeed"), "");
  } else {
    Notificator::badNews(QString("Restore [%1/%2] some failed"), "");
  }
  for (const auto& ind : indexes) {
    m_model->removeRow(ind.row());
  }
  m_model->submitAll();
  ChangeWindowIcon();
  return true;
}

RecycleBinWidget::SubdirItemsFirst RecycleBinWidget::SubdirItemsFirstSort(const QStringList& names,
                                                                          const QStringList& tmpPaths,
                                                                          const QStringList& oldPaths) {
  if (tmpPaths.size() != names.size() or oldPaths.size() != names.size()) {
    qWarning("names[%d] and paths[%d, %d] list size not equal", names.size(), tmpPaths.size(), oldPaths.size());
    return {};
  }
  QMap<QString, int> fullpath2BeforeIndex;
  QStringList fullpath;
  fullpath.reserve(names.size());
  for (int i = 0; i < names.size(); ++i) {
    fullpath.append(oldPaths[i] + '/' + names[i]);
    fullpath2BeforeIndex[fullpath.back()] = i;
  }
  fullpath.sort(Qt::CaseSensitivity::CaseInsensitive);

  SubdirItemsFirst ans;
  ans.names.reserve(names.size());
  ans.oldPaths.reserve(names.size());
  ans.tempPaths.reserve(names.size());
  for (int i = fullpath.size() - 1; i > -1; --i) {
    const int beforeIndex = fullpath2BeforeIndex[fullpath[i]];
    ans.names.append(names[beforeIndex]);
    ans.oldPaths.append(oldPaths[beforeIndex]);
    ans.tempPaths.append(tmpPaths[beforeIndex]);
  }
  return ans;
}

RecycleBinWidget::SubdirItemsFirst RecycleBinWidget::SubdirItemsFirstSort(const TrashbinModel* model, const QModelIndexList& indexes) {
  QStringList tmpPaths;
  tmpPaths.reserve(indexes.size());
  QStringList names;
  names.reserve(indexes.size());
  QStringList oldPaths;
  oldPaths.reserve(indexes.size());
  for (const auto& ind : indexes) {
    oldPaths.append(model->oldLocationFolder(ind));
    tmpPaths.append(model->tempLocationFolder(ind));
    names.append(model->fileName(ind));
  }
  return SubdirItemsFirstSort(names, tmpPaths, oldPaths);
}

bool RecycleBinWidget::createRecycleTable() {
  QSqlDatabase con = GetSqlRecycleBinDB();
  if (not con.isOpen() and not con.open()) {
    qDebug("Con cannot open");
    return false;
  }
  static const QString& createTableSQL = QString(
                                             "CREATE TABLE IF NOT EXISTS `%1`("
                                             "   `%2` NCHAR(256) NOT NULL,"  // Name
                                             "   `%3` INT DEFAULT 0,"        // Size
                                             "   `%4` CHAR(10),"             // Type
                                             "   `%5` CHAR(20),"             // DateModified
                                             "   `%6` CHAR(256) NOT NULL,"   // OldLocation
                                             "   `%7` INT,"                  // DeleteID
                                             "   `%8` CHAR(256) NOT NULL,"   // TempLocation
                                             "   `%9` CHAR(20),"             // DeleteTime
                                             "    PRIMARY KEY (%2, %8)"
                                             "    );")
                                             .arg(RecycleBinHelper::RECYCLE_ITEMS_TABLE)
                                             .arg(RECYCLE_HEADER_KEY::Name)
                                             .arg(RECYCLE_HEADER_KEY::Size)
                                             .arg(RECYCLE_HEADER_KEY::Type)
                                             .arg(RECYCLE_HEADER_KEY::DateModified)
                                             .arg(RECYCLE_HEADER_KEY::OldLocation)
                                             .arg(RECYCLE_HEADER_KEY::DeleteID)
                                             .arg(RECYCLE_HEADER_KEY::TempLocation)
                                             .arg(RECYCLE_HEADER_KEY::DeleteTime);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", qPrintable(RecycleBinHelper::RECYCLE_ITEMS_TABLE));
    QMessageBox::warning(this, RecycleBinHelper::RECYCLE_ITEMS_TABLE, "Create table[%s] failed");
    return false;
  }
  return true;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RecycleBinWidget win;
  win.show();
  RecycleBinHelper::RecycleABatch({"E:/"}, {"analytics.js-middleware-braze-deduplicate.js.gz"});
  a.exec();
  return 0;
}
#endif
