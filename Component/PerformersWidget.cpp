#include "PerformersWidget.h"

#include "Actions/PerformersManagerActions.h"
#include "Component/RatingSqlTableModel.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/PerformerJsonFileHelper.h"
#include "Tools/PerformersAkaManager.h"

#include <QDesktopServices>
#include <QDirIterator>
#include <QDockWidget>
#include <QHeaderView>
#include <QInputDialog>
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlField>
#include <QSqlQuery>
#include <QSqlRecord>

PerformersWidget::PerformersWidget(QWidget* parent)
    : QMainWindow{parent},
      m_performersListView{new PerformersTableView(nullptr)},
      m_introductionTextEdit(new PerformersPreviewTextBrowser),
      performerPreviewDock{new QDockWidget(tr("Overview"), this)},
      m_perfsDBModel(nullptr),
      m_imageHostPath(
          PreferenceSettings().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v).toString()),
      m_performerImageHeight(
          PreferenceSettings().value(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.v).toInt()) {
  QSqlDatabase con = GetSqlDB();
  m_perfsDBModel = new RatingSqlTableModel(this, con);
  if (con.tables().contains(DB_TABLE::PERFORMERS)) {
    m_perfsDBModel->setTable(DB_TABLE::PERFORMERS);
    m_perfsDBModel->submitAll();
  }
  m_performersListView->setModel(m_perfsDBModel);
  setCentralWidget(m_performersListView);

  performerPreviewDock->setWidget(m_introductionTextEdit);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, performerPreviewDock);

  setMenuBar(g_performersManagerActions().GetMenuBar());

  m_performersListView->InitTableView();

  subscribe();

  updateWindowsSize();
  setWindowTitle("Performers Manager Widget");
  setWindowIcon(QIcon(":/themes/PERFORMERS_MANAGER"));
}

void PerformersWidget::subscribe() {
  connect(g_performersManagerActions().OPEN_WITH_LOCAL_APP, &QAction::triggered, this, [this]() {
    if (not QFile::exists(SystemPath::PEFORMERS_DATABASE)) {
      QMessageBox::information(this, "open failed", QString("[%1] not exists. \nCreate it first").arg(SystemPath::PEFORMERS_DATABASE));
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(SystemPath::PEFORMERS_DATABASE));
  });

  connect(g_performersManagerActions().INIT_DATABASE, &QAction::triggered, this, &PerformersWidget::onInitDataBase);
  connect(g_performersManagerActions().INIT_TABLE, &QAction::triggered, this, &PerformersWidget::onInitATable);
  connect(g_performersManagerActions().INSERT_INTO_TABLE, &QAction::triggered, this, &PerformersWidget::onInsertIntoTable);
  connect(g_performersManagerActions().DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DROP_OR_DELETE::DROP); });
  connect(g_performersManagerActions().DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DROP_OR_DELETE::DELETE); });

  connect(g_performersManagerActions().SUBMIT, &QAction::triggered, this, &PerformersWidget::onSubmit);

  connect(g_performersManagerActions().LOCATE_IMAGEHOST, &QAction::triggered, this, &PerformersWidget::onLocateImageHost);

  connect(g_performersManagerActions().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT, &QAction::triggered, this,
          &PerformersWidget::onChangePerformerImageHeight);

  connect(g_performersManagerActions().LOAD_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &PerformersWidget::onLoadFromFileSystemStructure);
  connect(g_performersManagerActions().LOAD_FROM_PERFORMERS_LIST, &QAction::triggered, this, &PerformersWidget::onLoadFromPerformersList);
  connect(g_performersManagerActions().LOAD_FROM_PJSON_PATH, &QAction::triggered, this, &PerformersWidget::onLoadFromPJsonDirectory);

  connect(g_performersManagerActions().DUMP_ALL_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &PerformersWidget::onDumpAllIntoPJsonFile);
  connect(g_performersManagerActions().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &PerformersWidget::onDumpIntoPJsonFile);

  connect(g_performersManagerActions().OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &PerformersWidget::onOpenRecordInFileSystem);

  connect(m_performersListView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PerformersWidget::on_selectionChanged);

  connect(g_performersManagerActions().DELETE_RECORDS, &QAction::triggered, this, &PerformersWidget::onDeleteRecords);

  connect(g_performersManagerActions().REFRESH_SELECTED_RECORDS_VIDS, &QAction::triggered, this, &PerformersWidget::onForceRefreshRecordsVids);
  connect(g_performersManagerActions().REFRESH_ALL_RECORDS_VIDS, &QAction::triggered, this, &PerformersWidget::onForceRefreshAllRecordsVids);
}

auto PerformersWidget::closeEvent(QCloseEvent* event) -> void {
  PreferenceSettings().setValue("PerformersWidgetGeometry", saveGeometry());
  PreferenceSettings().setValue("PerformersWidgetDockerWidth", performerPreviewDock->width());
  PreferenceSettings().setValue("PerformersWidgetDockerHeight", performerPreviewDock->height());
  QMainWindow::closeEvent(event);
}

void PerformersWidget::updateWindowsSize() {
  if (PreferenceSettings().contains("PerformersWidgetGeometry")) {
    restoreGeometry(PreferenceSettings().value("PerformersWidgetGeometry").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
  }
}

QSqlDatabase PerformersWidget::GetSqlDB() const {
  QSqlDatabase con;
  if (QSqlDatabase::connectionNames().contains("perfs_connection")) {
    con = QSqlDatabase::database("perfs_connection", false);
  } else {
    con = QSqlDatabase::addDatabase("QSQLITE", "perfs_connection");
  }
  con.setDatabaseName(SystemPath::PEFORMERS_DATABASE);
  if (not con.open()) {
    qDebug("%s", con.lastError().text().toStdString().c_str());
  }
  return con;
}

bool PerformersWidget::onInitDataBase() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  qDebug("Database create succeed");
  return true;
}

void PerformersWidget::onInitATable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return;
  }

  if (con.tables().contains(DB_TABLE::PERFORMERS)) {
    qDebug("Table[%s] already exists in database[%s]", qPrintable(DB_TABLE::PERFORMERS), con.databaseName().toStdString().c_str());
    return;
  }

  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  const QString& createTableSQL = PerformerJsonFileHelper::CreatePerformerTableSQL(DB_TABLE::PERFORMERS);
  QSqlQuery createTableQuery(con);
  const auto ret = createTableQuery.exec(createTableSQL);
  if (not ret) {
    qDebug("Create table[%s] failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  m_perfsDBModel->setTable(DB_TABLE::PERFORMERS);
  m_perfsDBModel->submitAll();
  qDebug("Table create succeed");
}

bool PerformersWidget::onInsertIntoTable() {
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(DB_TABLE::PERFORMERS)) {
    const QString& tablesNotExistsMsg = QString("Cannot insert, table[%1] not exist.").arg(DB_TABLE::PERFORMERS);
    qDebug("Table [%s] not exists", qPrintable(tablesNotExistsMsg));
    QMessageBox::warning(this, "Abort", tablesNotExistsMsg);
    return false;
  }
  const QString& name = QInputDialog::getText(this, "Insert a performer record", "Input name here (CANNOT be empty)");
  if (name.isEmpty()) {
    QMessageBox::warning(this, "Name is Empty", "Name cannot be empty");
    return false;
  }
  const QString& insertCmd = QString("INSERT INTO `%1` (%2) VALUES(\"%3\");").arg(DB_TABLE::PERFORMERS, PERFORMER_DB_HEADER_KEY::Name, name);
  QSqlQuery insertTableQuery(con);
  const bool insertResult = insertTableQuery.exec(insertCmd);
  if (not insertResult) {
    const QString& errorMsg = insertTableQuery.lastError().text();
    qDebug("[Command Failed] [%s]. %s", qPrintable(insertCmd), qPrintable(errorMsg));
    QMessageBox::warning(this, "Command Failed", insertCmd + '\n' + errorMsg);
    con.rollback();
    return false;
  }
  insertTableQuery.finish();
  m_perfsDBModel->submitAll();
  return true;
}

bool PerformersWidget::onDropDeleteTable(const DROP_OR_DELETE dropOrDelete) {
  QString sqlCmd;
  switch (dropOrDelete) {
    case DROP_OR_DELETE::DROP:
      sqlCmd = QString("DROP TABLE `%1`;").arg(DB_TABLE::PERFORMERS);
      break;
    case DROP_OR_DELETE::DELETE:;
      sqlCmd = QString("DELETE FROM `%1`;").arg(DB_TABLE::PERFORMERS);
      break;
    default:
      qDebug("invalid choice");
      return false;
  }
  auto retBtn = QMessageBox::question(this, "Confirm drop/delete? (not recoverable)", sqlCmd);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("cancel");
    return true;
  }
  QSqlDatabase con = GetSqlDB();
  if (not con.isOpen()) {
    qDebug("con cannot open");
    return false;
  }
  if (not con.tables().contains(DB_TABLE::PERFORMERS)) {
    qDebug("Table[%s] not exists", qPrintable(DB_TABLE::PERFORMERS));
    return true;
  }

  QSqlQuery dropQry(con);
  const auto dropTableRet = dropQry.exec(sqlCmd);
  if (not dropTableRet) {
    qDebug("[Drop Table Failed] [%s]. Reason: %s", qPrintable(sqlCmd), dropQry.lastError().text().toStdString().c_str());
  }
  dropQry.finish();
  m_perfsDBModel->submitAll();
  QMessageBox::information(this, QString("Drop/Delete result: %1").arg(dropTableRet), sqlCmd);
  return dropTableRet;
}

int PerformersWidget::onLoadFromFileSystemStructure() {
  if (not QDir(m_imageHostPath).exists()) {
    QMessageBox::warning(this, "Path[file-system structure] not exist", m_imageHostPath);
    qDebug("file-system structure [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (m_perfsDBModel->isDirty()) {
    QMessageBox::warning(this, "Cannot load from file-system structure now", "submit before load from file-system structure");
    qDebug("submit before load from file-system structure");
    return false;
  }
  const QString& insertTemplate = QString("INSERT INTO `%1` (%2,%3,%4) VALUES")
                                      .arg(DB_TABLE::PERFORMERS)
                                      .arg(PERFORMER_DB_HEADER_KEY::Name)
                                      .arg(PERFORMER_DB_HEADER_KEY::Orientation)
                                      .arg(PERFORMER_DB_HEADER_KEY::Imgs) +
                                  QString("(\"%1\", \"%2\", \"%3\") ON CONFLICT(") + PERFORMER_DB_HEADER_KEY::Name + (") DO UPDATE SET ") +
                                  PERFORMER_DB_HEADER_KEY::Orientation + "=\"%2\"," + PERFORMER_DB_HEADER_KEY::Imgs + "=\"%3\"";

  int loadCnt = 0;
  int succeedCnt = 0;
  QSqlDatabase con = GetSqlDB();
  if (con.transaction()) {
    QSqlQuery insertTableQuery(con);

    QMap<QString, QString> name2Ori;
    QMap<QString, QStringList> name2Imgs;
    QDirIterator it(m_imageHostPath, TYPE_FILTER::IMAGE_TYPE_SET, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
    while (it.hasNext()) {
      it.next();
      const QStringList& imgPath = it.filePath().split('/');
      const QString& imgName = imgPath.back();
      const QString& perfName = imgPath[imgPath.size() - 2];
      const QString& ori = imgPath[imgPath.size() - 3];
      if (name2Imgs.contains(perfName)) {
        name2Imgs[perfName] << imgName;
      } else {
        name2Imgs.insert(perfName, {imgName});
        name2Ori[perfName] = ori;
      }
    }
    for (auto mpIt = name2Ori.cbegin(); mpIt != name2Ori.cend(); ++mpIt) {
      const QString& perf = mpIt.key();
      const QString& ori = mpIt.value();
      const QString& imgs = name2Imgs[perf].join(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR);  // img seperated by \n
      const QString& currentInsert = insertTemplate.arg(perf, ori, imgs);
      bool ret = insertTableQuery.exec(currentInsert);
      succeedCnt += ret;
      ++loadCnt;
      if (not ret) {
        const QString& errorMsg = insertTableQuery.lastError().text();
        qDebug("[Command Failed] [%s]. Reason: %s", qPrintable(currentInsert), qPrintable(errorMsg));
      }
    }

    if (!con.commit()) {
      qDebug() << "Failed to commit, all will be rollback";
      con.rollback();
    }
    insertTableQuery.finish();
    m_perfsDBModel->submitAll();
  } else {
    qDebug() << "Failed to start transaction mode";
    return 0;
  }
  qDebug("pjson file load succeed %d/%d", succeedCnt, loadCnt);
  return 0;
}

int PerformersWidget::onLoadFromPerformersList() {
  bool ok = false;
  const QString& perfsText =
      QInputDialog::getMultiLineText(this, "Performers List", "Example:\nA(a1, a2)\r\nB(b1, b2))\n...\nseperated by \\r\\n.", "", &ok);
  if (not ok) {
    return 0;
  }
  QMap<QString, QString> perfs;
  for (const QString& line : perfsText.split(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR)) {
    if (line.isEmpty()) {
      continue;
    }
    QStringList aka = line.split(JSON_RENAME_REGEX::SEPERATOR_COMP);
    QString stdName = aka.front();
    aka.pop_front();
    perfs.insert(stdName, aka.join(","));
  }
  if (perfs.isEmpty()) {
    return 0;
  }

  const QString& insertTemplate =
      QString("INSERT INTO `%1` (%2,%3) VALUES").arg(DB_TABLE::PERFORMERS).arg(PERFORMER_DB_HEADER_KEY::Name).arg(PERFORMER_DB_HEADER_KEY::AKA) +
      QString("(\"%1\", \"%2\") ON CONFLICT(%3) DO UPDATE SET %4 = \"%2\"");

  QSqlDatabase con = GetSqlDB();
  if (con.transaction()) {
    QSqlQuery insertTableQuery(con);
    // update aka by new value if name conflict
    for (auto it = perfs.cbegin(); it != perfs.cend(); ++it) {
      const QString& currentInsert =
          insertTemplate.arg(it.key()).arg(it.value()).arg(PERFORMER_DB_HEADER_KEY::Name).arg(PERFORMER_DB_HEADER_KEY::AKA);
      bool ret = insertTableQuery.exec(currentInsert);
      if (not ret) {
        qDebug("[Failed] %s", qPrintable(currentInsert));
      }
    }
    if (!con.commit()) {
      qDebug() << "Failed to commit";
      con.rollback();
    }
    insertTableQuery.finish();
    m_perfsDBModel->submitAll();
  } else {
    qDebug() << "Failed to start transaction mode";
  }
  return perfs.size();
}

bool PerformersWidget::onLocateImageHost() {
  const QString& locatePath = QFileDialog::getExistingDirectory(this, "Locate imagehost folder", m_imageHostPath);
  if (not QFile::exists(locatePath)) {
    qDebug("locate path not exist");
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, m_imageHostPath = locatePath);
  g_performersManagerActions().LOCATE_IMAGEHOST->setToolTip(m_imageHostPath);
  return true;
}

bool PerformersWidget::onChangePerformerImageHeight() {
  bool ok = false;
  int height = QInputDialog::getInt(this, "Performer image height(px)", QString("default: %1").arg(m_performerImageHeight), m_performerImageHeight, 0,
                                    INT_MAX, 1, &ok);
  if (not ok) {
    return false;
  }
  m_performerImageHeight = height;
  PreferenceSettings().setValue(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, m_performerImageHeight);
  g_performersManagerActions().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(QString::number(height));
  return true;
}

inline bool PerformersWidget::onSubmit() {
  if (not m_perfsDBModel->isDirty()) {
    qDebug("No need to submit");
    return true;
  }
  return m_perfsDBModel->submitAll();
}

bool PerformersWidget::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
  if (not m_performersListView->currentIndex().isValid()) {
    m_introductionTextEdit->setText("");
    return true;
  }
  const auto& record = m_perfsDBModel->record(m_performersListView->currentIndex().row());
  m_introductionTextEdit->operator()(record, m_imageHostPath, m_performerImageHeight);
  return true;
}

int PerformersWidget::onLoadFromPJsonDirectory() {
  if (not QDir(m_imageHostPath).exists()) {
    QMessageBox::warning(this, "Path[pjson load from] not exist", m_imageHostPath);
    qDebug("*.pjson path load from [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (m_perfsDBModel->isDirty()) {
    QMessageBox::warning(this, "Cannot load pjson now", "submit before load pjson");
    qDebug("submit before load pjson");
    return false;
  }

  int loadCnt = 0;
  int succeedCnt = 0;
  QSqlDatabase con = GetSqlDB();
  if (con.transaction()) {
    QSqlQuery insertTableQuery(con);
    QDirIterator it(m_imageHostPath, {"*.pjson"}, QDir::Filter::Files, QDirIterator::IteratorFlag::Subdirectories);
    while (it.hasNext()) {
      it.next();
      const QVariantHash& pJson = JsonFileHelper::MovieJsonLoader(it.filePath());
      const QString& currentInsert = PerformerJsonFileHelper::PerformerInsertSQL(DB_TABLE::PERFORMERS, pJson);
      bool ret = insertTableQuery.exec(currentInsert);
      succeedCnt += ret;
      ++loadCnt;
      if (not ret) {
        qDebug("[Command Failed] [%s]. Reason: %s", qPrintable(currentInsert), insertTableQuery.lastError().text().toStdString().c_str());
      }
    }
    if (!con.commit()) {
      qDebug() << "Failed to commit, all will be rollback";
      con.rollback();
    }
    insertTableQuery.finish();
    m_perfsDBModel->submitAll();
  } else {
    qDebug() << "Failed to start transaction mode";
    return 0;
  }
  qDebug("pjson file load succeed %d/%d", succeedCnt, loadCnt);
  return 0;
}

int PerformersWidget::onDumpAllIntoPJsonFile() {
  if (not QDir(m_imageHostPath).exists()) {
    qDebug("*.pjson path dump to [%s] not exists", qPrintable(m_imageHostPath));
    QMessageBox::warning(this, "Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (int r = 0; r < m_perfsDBModel->rowCount(); ++r) {
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfsDBModel->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonFileHelper::MovieJsonDumper(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("All %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  QMessageBox::information(this, QString("All %1 record(s) dumped result").arg(dumpCnt), QString("succeed:%1/%2").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int PerformersWidget::onDumpIntoPJsonFile() {
  if (not QDir(m_imageHostPath).exists()) {
    qDebug("*.pjson path dump to [%s] not exists", qPrintable(m_imageHostPath));
    QMessageBox::warning(this, "Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }

  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select some row to dump");
    QMessageBox::warning(this, "Nothing was selected", "Select some row to dump");
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (auto indr : m_performersListView->selectionModel()->selectedRows()) {
    const int r = indr.row();
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfsDBModel->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonFileHelper::MovieJsonDumper(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("Selected %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  QMessageBox::information(this, QString("Selected %1 record(s) dumped result").arg(dumpCnt), QString("succeed:%1/%2").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int PerformersWidget::onForceRefreshAllRecordsVids() {
  QMessageBox::information(this, QString("Oops function not support now"),
                           QString("But you could selected all record(s) and then force refresh instead."));
  return 0;
}

int PerformersWidget::onForceRefreshRecordsVids() {
  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select some records to refresh");
    QMessageBox::warning(this, "Nothing was selected", "Select some row to refresh");
    return 0;
  }
  QSqlDatabase con = ::GetSqlVidsDB();  // videos table
  if (not con.isOpen()) {
    qDebug("con cannot open [%s]", qPrintable(SystemPath::VIDS_DATABASE));
    return 0;
  }

  static auto& dbTM = PerformersAkaManager::getIns();
  static auto GetVidsListFromVidsTable = [](const QSqlRecord& record, QSqlQuery& qur) -> QStringList {
    const QString& searchCommand = dbTM.GetMovieTablePerformerSelectCommand(record);
    bool ret = qur.exec(searchCommand);
    if (not ret) {
      qDebug("Failed when[%s]", qPrintable(searchCommand));
      return {};
    }
    QStringList vidPath;
    while (qur.next()) {
      vidPath << qur.value(DB_HEADER_KEY::ForSearch).toString();
    }
    return vidPath;
  };

  QSqlQuery qur(con);
  int recordsCnt = 0;
  int vidsCnt = 0;
  for (auto indr : m_performersListView->selectionModel()->selectedRows()) {
    const int r = indr.row();
    auto record = m_perfsDBModel->record(r);
    const QStringList& vidsList = GetVidsListFromVidsTable(record, qur);
    record.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidsList.join(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR));
    m_perfsDBModel->setRecord(r, record);  // update back
    vidsCnt += vidsList.size();
    if (recordsCnt == 0){
      m_introductionTextEdit->operator()(record, m_imageHostPath, m_performerImageHeight);
    }
    ++recordsCnt;
  }
  qDebug("Selected %d record(s) updated %d vid(s).", recordsCnt, vidsCnt);
  QMessageBox::information(this, QString("Selected %1 record(s) updated.").arg(recordsCnt), QString("%1 vid(s)").arg(vidsCnt));
  return recordsCnt;
}

bool PerformersWidget::onOpenRecordInFileSystem() const {
  if (not QDir(m_imageHostPath).exists()) {
    qDebug("m_imageHostPath [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select a row to open pjson folder");
    return false;
  }

  const auto& record = m_perfsDBModel->record(m_performersListView->currentIndex().row());
  QString folderPath = QString("%1/%2/%3")
                           .arg(m_imageHostPath)
                           .arg(record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString())
                           .arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  if (m_performersListView->currentIndex().column() == PERFORMER_DB_HEADER_KEY::Detail_INDEX) {
    folderPath += QString("/%1.pjson").arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  }
  if (not QFile::exists(folderPath)) {
    qDebug("Path[%s] not exists", qPrintable(folderPath));
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

int PerformersWidget::onDeleteRecords() {
  if (not m_performersListView->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select some row(s) to delete");
    QMessageBox::warning(this, "Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  int deleteCnt = 0;
  int succeedCnt = 0;
  const auto& itemSelection = m_performersListView->selectionModel()->selection();
  for (auto it = itemSelection.crbegin(); it != itemSelection.crend(); ++it) {
    int startRow = it->top();  // [top, bottom]
    int size = it->bottom() - startRow + 1;
    bool ret = m_perfsDBModel->removeRows(startRow, size);
    qDebug("drop[%d] records [%d, %d]", ret, startRow, it->bottom());
    deleteCnt += size;
    succeedCnt += ((int)ret * size);
  }
  m_perfsDBModel->submitAll();
  qDebug("delete records succeed: %d/%d.", succeedCnt, deleteCnt);
  QMessageBox::information(this, "delete records result", QString("%1/%2 succeed").arg(succeedCnt).arg(deleteCnt));
  return succeedCnt;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  PerformersManagerWidget perfManaWid;
  perfManaWid.show();
  a.exec();
  return 0;
}
#endif
