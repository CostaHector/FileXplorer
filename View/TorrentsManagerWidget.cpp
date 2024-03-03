#include "TorrentsManagerWidget.h"
#include "Actions/TorrentsManagerActions.h"
#include "PublicVariable.h"
#include "Tools/TorrentsDatabaseHelper.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QHeaderView>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include <QDirIterator>
#include <QInputDialog>

TorrentsManagerWidget::TorrentsManagerWidget(QWidget* parent)
    : QMainWindow{parent},
      m_searchLE{new QLineEdit(QString("%1 like \"%\"").arg(TORRENTS_DB_HEADER_KEY::Name))},
      m_torrentsListView(new CustomTableView("TORRENT_TABLE", parent)),
      m_torrentsCentralWidget(new QWidget),
      m_torrentsDBModel(nullptr) {
  auto* mainLo = new QVBoxLayout;
  mainLo->addWidget(m_searchLE);
  mainLo->addWidget(m_torrentsListView);
  m_torrentsCentralWidget->setLayout(mainLo);

  setMenuBar(g_torrentsManagerActions().GetMenuBar());
  setCentralWidget(m_torrentsCentralWidget);

  QSqlDatabase con = GetSqlDB();
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
  setWindowIcon(QIcon(":/themes/TORRENTS_MANAGER"));
  updateWindowsSize();
}

void TorrentsManagerWidget::subscribe() {
  connect(m_searchLE, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_searchLE->text();
    m_torrentsDBModel->setFilter(searchPattern);
  });

  connect(g_torrentsManagerActions().OPEN_WITH_LOCAL_APP, &QAction::triggered, this, [this]() {
    if (not QFile::exists(SystemPath::TORRENTS_DATABASE)) {
      QMessageBox::information(this, "open failed", QString("[%1] not exists. \nCreate it first").arg(SystemPath::TORRENTS_DATABASE));
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(SystemPath::TORRENTS_DATABASE));
  });

  connect(g_torrentsManagerActions().INIT_DATABASE, &QAction::triggered, this, &TorrentsManagerWidget::onInitDataBase);
  connect(g_torrentsManagerActions().INIT_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onInitATable);
  connect(g_torrentsManagerActions().INSERT_INTO_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onInsertIntoTable);
  connect(g_torrentsManagerActions().DROP_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onDropATable);
  connect(g_torrentsManagerActions().DELETE_FROM_TABLE, &QAction::triggered, this, &TorrentsManagerWidget::onDeleteFromTable);
  connect(g_torrentsManagerActions().SUBMIT, &QAction::triggered, this, &TorrentsManagerWidget::onSubmit);
}

QSqlDatabase TorrentsManagerWidget::GetSqlDB() const {
  QSqlDatabase con;
  if (QSqlDatabase::connectionNames().contains("torrents_connection")) {
    con = QSqlDatabase::database("torrents_connection", false);
  } else {
    con = QSqlDatabase::addDatabase("QSQLITE", "torrents_connection");
  }
  con.setDatabaseName(SystemPath::TORRENTS_DATABASE);
  if (not con.open()) {
    qDebug("%s", con.lastError().text().toStdString().c_str());
  }
  return con;
}

bool TorrentsManagerWidget::onInitDataBase() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

void TorrentsManagerWidget::onInitATable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return;
  }
  if (con.tables().contains(DB_TABLE::TORRENTS)) {
    qDebug("Table[%s] already exists in database[%s]", qPrintable(DB_TABLE::TORRENTS), con.databaseName().toStdString().c_str());
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = TorrentsDatabaseHelper::CreatePerformerTableSQL(DB_TABLE::TORRENTS);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.\n%s", qPrintable(DB_TABLE::TORRENTS), createTableQuery.lastError().text().toStdString().c_str());
    return;
  }
  m_torrentsDBModel->setTable(DB_TABLE::TORRENTS);
  m_torrentsDBModel->submitAll();
  qDebug("Table[%s] create succeed", qPrintable(DB_TABLE::TORRENTS));
}

bool TorrentsManagerWidget::onInsertIntoTable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(DB_TABLE::TORRENTS)) {
    const QString& tablesNotExistsMsg = QString("Cannot insert, table[%1] not exist.").arg(DB_TABLE::TORRENTS);
    qDebug("Table [%s] not exists", qPrintable(tablesNotExistsMsg));
    QMessageBox::warning(this, "Abort", tablesNotExistsMsg);
    return false;
  }

  const QString& defaultOpenDir =
      PreferenceSettings().value(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.v).toString();
  const QString& loadFromPath = QFileDialog::getExistingDirectory(this, "Load torrents from", defaultOpenDir);
  QFileInfo loadFromFi(loadFromPath);
  if (not loadFromFi.isDir()) {
    QMessageBox::warning(this, "Failed when Load torrents from a folder", QString("Not a folder:\n%1").arg(loadFromPath));
    qDebug("Failed when Load json from a folder. Not a folder:\n%s", qPrintable(loadFromPath));
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_DB_INSERT_TORRENTS_FROM.name, loadFromFi.absoluteFilePath());
  static const QString& insertTemplate =
      QString("REPLACE INTO `%1` (%2) VALUES").arg(DB_TABLE::TORRENTS).arg(TORRENTS_DB_HEADER_KEY::HEADERS.join(',')) +
      QString("(\"%1\", %2, \"%3\", \"%4\", \"%5\", \"%6\");");

  if (not con.transaction()) {
    qDebug() << "Failed to start transaction mode";
    return 0;
  }
  QSqlQuery insertTableQuery(con);

  int totalItemCnt = 0;
  int succeedItemCnt = 0;
  QDirIterator it(loadFromPath, {"*.torrent"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    it.next();
    const QFileInfo& fi = it.fileInfo();
    const QString& currentInsert =
        insertTemplate.arg(fi.fileName()).arg(fi.size()).arg(fi.suffix()).arg(fi.lastModified().toMSecsSinceEpoch()).arg("").arg(fi.absolutePath());
    const bool insertResult = insertTableQuery.exec(currentInsert);
    succeedItemCnt += int(insertResult);
    if (not insertResult) {
      qDebug("Error [%s]: %s", qPrintable(currentInsert), insertTableQuery.lastError().text().toStdString().c_str());
    }
    ++totalItemCnt;
  }

  if (!con.commit()) {
    qDebug() << "Failed to commit, all will be rollback";
    con.rollback();
    succeedItemCnt = 0;
  }
  insertTableQuery.finish();
  const QString& msg = QString("%1/%2 item(s) add succeed. %3").arg(succeedItemCnt).arg(totalItemCnt).arg(loadFromPath);
  qDebug("%s", qPrintable(msg));
  QMessageBox::information(this, QString("Finish insert into table[%1]").arg(DB_TABLE::TORRENTS), qPrintable(msg));
  m_torrentsDBModel->submitAll();
  return true;
}

bool TorrentsManagerWidget::onDropATable() {
  const QString& sqlCmd = QString("DROP TABLE `%1`;").arg(DB_TABLE::TORRENTS);
  auto retBtn = QMessageBox::warning(this, "CONFIRM DROP?", "(NOT recoverable)\n" + sqlCmd,
                                     QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("User Cancel");
    return true;
  }

  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(DB_TABLE::TORRENTS)) {
    qDebug("Table[%s] already not exists", qPrintable(DB_TABLE::TORRENTS));
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(sqlCmd);
  if (not dropTableRet) {
    qDebug("Drop Table[%s] failed. %s", qPrintable(DB_TABLE::TORRENTS), con.lastError().databaseText().toStdString().c_str());
  }
  dropQry.finish();
  m_torrentsDBModel->submitAll();
  QMessageBox::information(this, "Table been Dropped", DB_TABLE::TORRENTS);
  return dropTableRet;
}

bool TorrentsManagerWidget::onDeleteFromTable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  const QString& whereClause = QInputDialog::getItem(this, "Where Clause", "Input clause below", TORRENTS_DB_HEADER_KEY::HEADERS, 0, true);
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
  PreferenceSettings().setValue("TorrentsManagerWidgetGeometry", saveGeometry());
  QMainWindow::closeEvent(event);
}

void TorrentsManagerWidget::updateWindowsSize() {
  if (PreferenceSettings().contains("TorrentsManagerWidgetGeometry")) {
    restoreGeometry(PreferenceSettings().value("TorrentsManagerWidgetGeometry").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
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
