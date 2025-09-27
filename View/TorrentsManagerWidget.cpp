#include "TorrentsManagerWidget.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

#include "TorrDBAction.h"
#include "TableFields.h"
#include "NotificatorMacro.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>
#include <QDirIterator>
#include <QInputDialog>

TorrentsManagerWidget::TorrentsManagerWidget(QWidget* parent)
  : QMainWindow{parent},
  mDb{SystemPath::TORRENTS_DATABASE(), "torrent_connection"} {
  CHECK_NULLPTR_RETURN_VOID(parent);

  setMenuBar(g_torrActions().GetMenuBar());

  m_searchToobar = new (std::nothrow) QToolBar{"search toolbar", this};
  m_searchLE = new (std::nothrow) QLineEdit{"Name like \"%\"", m_searchToobar};
  m_searchLE->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  m_searchToobar->addWidget(m_searchLE);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_searchToobar);

  QSqlDatabase con = mDb.GetDb();
  m_torrentsDBModel = new (std::nothrow) QSqlTableModel{this, con};
  CHECK_NULLPTR_RETURN_VOID(m_torrentsDBModel);
  if (con.tables().contains(DB_TABLE::TORRENTS)) {
    m_torrentsDBModel->setTable(DB_TABLE::TORRENTS);
  }
  m_torrentsDBModel->setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  m_torrentsDBModel->submitAll();

  m_torrentsListView = new (std::nothrow) CustomTableView{"TORRENT_TABLE", this};
  m_torrentsListView->setModel(m_torrentsDBModel);
  setCentralWidget(m_torrentsListView);
  m_torrentsListView->InitTableView();


  subscribe();
  updateWindowsSize();
  setWindowTitle("Torrents Manager Widget");
  setWindowIcon(QIcon(":img/TORRENTS_VIEW"));
}

void TorrentsManagerWidget::subscribe() {
  connect(m_searchLE, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_searchLE->text();
    LOG_OK_NP("[New where clause]", searchPattern);
    m_torrentsDBModel->setFilter(searchPattern);
  });

  connect(g_torrActions().OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &mDb, &DbManager::onShowInFileSystemView);
  connect(g_torrActions().INIT_DATABASE, &QAction::triggered, this, &TorrentsManagerWidget::onInitDataBase);
  connect(g_torrActions().INIT_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onInitATable);
  connect(g_torrActions().INSERT_INTO_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onInsertIntoTable);
  connect(g_torrActions().DROP_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onDropATable);
  connect(g_torrActions().DELETE_FROM_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onDeleteFromTable);
  connect(g_torrActions().SUBMIT, &QAction::triggered, this, &TorrentsManagerWidget::onSubmit);
}

bool TorrentsManagerWidget::onInitDataBase() {
  return mDb.CreateDatabase();
}

void TorrentsManagerWidget::onInitATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    LOG_ERR_NP("Open db failed", con.lastError().text());
    return;
  }

  if (con.tables().contains(DB_TABLE::TORRENTS)) {
    LOG_OK_NP("[Skip] Table already exists", DB_TABLE::TORRENTS);
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes

  QSqlQuery createTableQuery(con);
  if (!createTableQuery.exec(TorrDb::CREATE_TABLE_TEMPLATE.arg(DB_TABLE::TORRENTS))) {
    LOG_ERR_NP("[failed] Exec failed", createTableQuery.lastError().text());
    return;
  }
  m_torrentsDBModel->setTable(DB_TABLE::TORRENTS);
  m_torrentsDBModel->submitAll();
  LOG_OK_NP("Table create succeed", DB_TABLE::TORRENTS);
}

bool TorrentsManagerWidget::onInsertIntoTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    LOG_ERR_NP("Open db failed", con.lastError().text());
    return false;
  }

  if (!con.tables().contains(DB_TABLE::TORRENTS)) {
    LOG_ERR_NP("[Failed] cannot insert, table not exists", DB_TABLE::TORRENTS);
    return false;
  }

  const QString& defaultOpenDir = Configuration().value(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.v).toString();
  const QString& loadFromPath = QFileDialog::getExistingDirectory(this, "Load torrents from", defaultOpenDir);
  QFileInfo loadFromFi(loadFromPath);
  if (!loadFromFi.isDir()) {
    LOG_ERR_NP("[Failed] not a folder", loadFromPath);
    return false;
  }
  Configuration().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, loadFromFi.absoluteFilePath());

  QSqlQuery query{con};
  if (!query.prepare(TorrDb::REPLACE_INTO_TABLE_TEMPLATE.arg(DB_TABLE::TORRENTS))) {
    LOG_ERR_NP("Prepare failed", query.lastError().text());
    return false;
  }

  if (!con.transaction()) {
    LOG_ERR_NP("Failed to start transaction", con.lastError().text());
    return 0;
  }

  int totalItemCnt = 0;
  int succeedItemCnt = 0;
  QDirIterator it{loadFromPath, {"*.torrent"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories};
  while (it.hasNext()) {
    it.next();
    const QFileInfo& fi = it.fileInfo();
    query.bindValue(TORRENTS_DB_HEADER_KEY::Name, fi.fileName());
    query.bindValue(TORRENTS_DB_HEADER_KEY::Size, fi.size());
    query.bindValue(TORRENTS_DB_HEADER_KEY::DateModified, fi.lastModified().toMSecsSinceEpoch());
    query.bindValue(TORRENTS_DB_HEADER_KEY::MD5, "");
    query.bindValue(TORRENTS_DB_HEADER_KEY::PrePath, fi.absolutePath());
    if (!query.exec()) {
      con.rollback();
      LOG_W("Error when[%s] fail: %s", qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return 0;
    }
    ++succeedItemCnt;
    ++totalItemCnt;
  }
  if (!con.commit()) {
    LOG_ERR_NP("[Failed] commit failed, all will be rollback", con.lastError().text());
    con.rollback();
    succeedItemCnt = 0;
  }
  query.finish();
  m_torrentsDBModel->submitAll();
  LOG_OK_P("[Ok] Insert into succeed", "%1 / %2 item(s) -> Table[%s]", succeedItemCnt, totalItemCnt, qPrintable(DB_TABLE::TORRENTS));
  return true;
}

bool TorrentsManagerWidget::onDropATable() {
  const QString& sqlCmd = QString("DROP TABLE `%1`;").arg(DB_TABLE::TORRENTS);
  auto retBtn = QMessageBox::warning(this, "CONFIRM DROP?", "(NOT recoverable)\n" + sqlCmd, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    LOG_OK_NP("User Cancel Drop", "return");
    return true;
  }

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    LOG_ERR_NP("Open db failed", con.lastError().text());
    return false;
  }

  if (!con.tables().contains(DB_TABLE::TORRENTS)) {
    LOG_ERR_NP("[Skip] Table already not exists", DB_TABLE::TORRENTS);
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(sqlCmd);
  if (!dropTableRet) {
    LOG_ERR_NP("[failed] Drop Table", con.lastError().databaseText());
    return false;
  }
  dropQry.finish();
  m_torrentsDBModel->submitAll();
  LOG_OK_NP("Table been Dropped", DB_TABLE::TORRENTS);
  QMessageBox::information(this, "Table been Dropped", DB_TABLE::TORRENTS);
  return dropTableRet;
}

bool TorrentsManagerWidget::onDeleteFromTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    LOG_ERR_NP("Open db failed", con.lastError().text());
    return false;
  }
  const QString& whereClause = QInputDialog::getItem(this, "Where Clause", "Input clause below",  //
                                                     {"Name", "Size"}, 0, true);
  if (whereClause.isEmpty()) {
    LOG_ERR_NP("Where clause empty", "return");
    return false;
  }
  const QString& deleteCmd = QString("DELETE FROM \"%1\" WHERE %2").arg(DB_TABLE::TORRENTS, whereClause);
  QSqlQuery seleteQry(con);
  const bool deleteRes = seleteQry.exec(deleteCmd);
  if (!deleteRes) {
    LOG_ERR_NP("[Failed] delete command failed", seleteQry.lastError().text());
    return false;
  }
  LOG_OK_NP("[Ok] delete command exec ok", deleteCmd);
  m_torrentsDBModel->submitAll();
  return deleteRes;
}

bool TorrentsManagerWidget::onSubmit() {
  CHECK_NULLPTR_RETURN_FALSE(m_torrentsDBModel)

  if (!m_torrentsDBModel->isDirty()) {
    LOG_OK_NP("[Skip] Table not dirty", DB_TABLE::TORRENTS);
    return true;
  }
  if (!m_torrentsDBModel->submitAll()) {
    LOG_ERR_NP("[Submit failed]", m_torrentsDBModel->lastError().text());
    return false;
  }

  LOG_OK_NP("Submit succeed", DB_TABLE::TORRENTS);
  return true;
}

void TorrentsManagerWidget::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void TorrentsManagerWidget::closeEvent(QCloseEvent* event) {
  g_torrActions().SHOW_TORRENTS_MANAGER->setChecked(false);
  Configuration().setValue("TorrentsManagerWidgetGeometry", saveGeometry());
  QMainWindow::closeEvent(event);
}

void TorrentsManagerWidget::updateWindowsSize() {
  if (Configuration().contains("TorrentsManagerWidgetGeometry")) {
    restoreGeometry(Configuration().value("TorrentsManagerWidgetGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}
