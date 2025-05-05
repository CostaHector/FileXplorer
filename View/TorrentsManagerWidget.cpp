#include "TorrentsManagerWidget.h"
#include "Actions/TorrDBAction.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Tools/FileDescriptor/TableFields.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QDateTime>

#include <QDirIterator>
#include <QInputDialog>

TorrentsManagerWidget::TorrentsManagerWidget(QWidget* parent)
    : QMainWindow{parent},
      mDb{SystemPath::TORRENTS_DATABASE, "torrent_connection"},
      m_searchLE{new QLineEdit(QString("Name like \"%\""))},
      m_torrentsListView(new CustomTableView("TORRENT_TABLE", parent)),
      m_torrentsCentralWidget(new QWidget),
      m_torrentsDBModel(nullptr) {
  auto* mainLo = new QVBoxLayout;
  mainLo->addWidget(m_searchLE);
  mainLo->addWidget(m_torrentsListView);
  m_torrentsCentralWidget->setLayout(mainLo);

  setMenuBar(g_torrentsManagerActions().GetMenuBar());
  setCentralWidget(m_torrentsCentralWidget);

  QSqlDatabase con = mDb.GetDb();
  m_torrentsDBModel = new QSqlTableModel(this, con);
  if (con.tables().contains(DB_TABLE::TORRENTS)) {
    m_torrentsDBModel->setTable(DB_TABLE::TORRENTS);
  }
  m_torrentsDBModel->setEditStrategy(QSqlTableModel::EditStrategy::OnManualSubmit);
  m_torrentsDBModel->submitAll();

  m_torrentsListView->setModel(m_torrentsDBModel);

  m_torrentsListView->InitTableView();

  subscribe();

  setWindowTitle("Torrents Manager Widget");
  setWindowIcon(QIcon(":img/TORRENTS_MANAGER"));
  updateWindowsSize();
}

void TorrentsManagerWidget::subscribe() {
  connect(m_searchLE, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_searchLE->text();
    m_torrentsDBModel->setFilter(searchPattern);
  });

  connect(g_torrentsManagerActions().OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &mDb, &DbManager::ShowInFileSystemView);

  connect(g_torrentsManagerActions().INIT_DATABASE, &QAction::triggered, this, &TorrentsManagerWidget::onInitDataBase);
  connect(g_torrentsManagerActions().INIT_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onInitATable);
  connect(g_torrentsManagerActions().INSERT_INTO_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onInsertIntoTable);
  connect(g_torrentsManagerActions().DROP_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onDropATable);
  connect(g_torrentsManagerActions().DELETE_FROM_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onDeleteFromTable);
  connect(g_torrentsManagerActions().SUBMIT, &QAction::triggered, this, &TorrentsManagerWidget::onSubmit);
}

bool TorrentsManagerWidget::onInitDataBase() {
  return mDb.CreateDatabase();
}

void TorrentsManagerWidget::onInitATable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return;
  }

  if (con.tables().contains(DB_TABLE::TORRENTS)) {
    qDebug("Table[%s] already exists in database[%s]", qPrintable(DB_TABLE::TORRENTS), qPrintable(con.databaseName()));
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes

  QSqlQuery createTableQuery(con);
  if (!createTableQuery.exec(TorrDb::CREATE_TABLE_TEMPLATE.arg(DB_TABLE::TORRENTS))) {
    qDebug("Exec[%s] failed:%s", qPrintable(createTableQuery.executedQuery()),  //
           qPrintable(createTableQuery.lastError().text()));
    return;
  }
  m_torrentsDBModel->setTable(DB_TABLE::TORRENTS);
  m_torrentsDBModel->submitAll();
  qDebug("Table[%s] create succeed", qPrintable(DB_TABLE::TORRENTS));
}

bool TorrentsManagerWidget::onInsertIntoTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }

  if (!con.tables().contains(DB_TABLE::TORRENTS)) {
    const QString& tablesNotExistsMsg = QString("Cannot insert, table[%1] not exist.").arg(DB_TABLE::TORRENTS);
    qDebug("Table [%s] not exists", qPrintable(tablesNotExistsMsg));
    QMessageBox::warning(this, "Abort", tablesNotExistsMsg);
    return false;
  }

  const QString& defaultOpenDir = PreferenceSettings().value(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.v).toString();
  const QString& loadFromPath = QFileDialog::getExistingDirectory(this, "Load torrents from", defaultOpenDir);
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    QMessageBox::warning(this, "Failed when Load torrents from a folder", QString("Not a folder:\n%1").arg(loadFromPath));
    qDebug("Failed when Load json from a folder. Not a folder:\n%s", qPrintable(loadFromPath));
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, loadFromFi.absoluteFilePath());

  QSqlQuery query{con};
  if (!query.prepare(TorrDb::REPLACE_INTO_TABLE_TEMPLATE.arg(DB_TABLE::TORRENTS))) {
    qWarning("Prepare failed: %s", qPrintable(query.lastError().text()));
    return false;
  }

  if (!con.transaction()) {
    qDebug("Failed to start transaction[%s]", qPrintable(con.lastError().text()));
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
      qDebug("Error[%s] fail: %s", qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
      return 0;
    }
    ++succeedItemCnt;
    ++totalItemCnt;
  }

  if (!con.commit()) {
    qDebug("Failed to commit, all will be rollback");
    con.rollback();
    succeedItemCnt = 0;
  }
  query.finish();
  const QString& msg = QString("%1/%2 item(s) add succeed. %3").arg(succeedItemCnt).arg(totalItemCnt).arg(loadFromPath);
  QMessageBox::information(this, QString("Finish insert into table[%1]").arg(DB_TABLE::TORRENTS), qPrintable(msg));
  m_torrentsDBModel->submitAll();
  return true;
}

bool TorrentsManagerWidget::onDropATable() {
  const QString& sqlCmd = QString("DROP TABLE `%1`;").arg(DB_TABLE::TORRENTS);
  auto retBtn = QMessageBox::warning(this, "CONFIRM DROP?", "(NOT recoverable)\n" + sqlCmd, QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("User Cancel");
    return true;
  }

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }

  if (!con.tables().contains(DB_TABLE::TORRENTS)) {
    qDebug("Table[%s] already not exists", qPrintable(DB_TABLE::TORRENTS));
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(sqlCmd);
  if (not dropTableRet) {
    qDebug("Drop Table[%s] failed. %s", qPrintable(DB_TABLE::TORRENTS), qPrintable(con.lastError().databaseText()));
  }
  dropQry.finish();
  m_torrentsDBModel->submitAll();
  QMessageBox::information(this, "Table been Dropped", DB_TABLE::TORRENTS);
  return dropTableRet;
}

bool TorrentsManagerWidget::onDeleteFromTable() {
  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("Open db failed:%s", qPrintable(con.lastError().text()));
    return false;
  }
  const QString& whereClause = QInputDialog::getItem(this, "Where Clause", "Input clause below",  //
                                                     {"Name", "Size"}, 0, true);
  if (whereClause.isEmpty()) {
    return false;
  }
  const QString& deleteCmd = QString("DELETE FROM \"%1\" WHERE %2").arg(DB_TABLE::TORRENTS, whereClause);
  QSqlQuery seleteQry(con);
  const auto deleteRes = seleteQry.exec(deleteCmd);
  qDebug("delete from result %d", deleteRes);
  m_torrentsDBModel->submitAll();
  return deleteRes;
}

auto TorrentsManagerWidget::closeEvent(QCloseEvent* event) -> void {
  g_torrentsManagerActions().SHOW_TORRENTS_MANAGER->setChecked(false);
  PreferenceSettings().setValue("TorrentsManagerWidgetGeometry", saveGeometry());
  QMainWindow::closeEvent(event);
}

void TorrentsManagerWidget::updateWindowsSize() {
  if (PreferenceSettings().contains("TorrentsManagerWidgetGeometry")) {
    restoreGeometry(PreferenceSettings().value("TorrentsManagerWidgetGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  TorrentsManagerWidget torrManaWid;
  torrManaWid.show();
  a.exec();
  return 0;
}
#endif
