#include "PerformersWidget.h"

#include "Actions/PerformersManagerActions.h"
#include "Component/RatingSqlTableModel.h"
#include "Component/NotificatorFrame.h"
#include "public/PublicMacro.h"
#include "public/PublicTool.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Tools/JsonFileHelper.h"
#include "Tools/PerformerJsonFileHelper.h"
#include "Tools/PerformersAkaManager.h"
#include "Tools/FileDescriptor/MovieBaseDb.h"

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
#include <QFileDialog>

PerformersWidget::PerformersWidget(QWidget* parent)
    : QMainWindow{parent},  //
      m_imageHostPath{PreferenceSettings()
                          .value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,  //
                                 MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v)
                          .toString()},  //
      m_performerImageHeight{PreferenceSettings()
                                 .value(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name,  //
                                        MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.v)
                                 .toInt()},  //
      mDb{SystemPath::PEFORMERS_DATABASE, "perfs_connection"}

{
  m_perfSearch = new (std::nothrow) QLineEdit{QString("%1 like \"%\"").arg(PERFORMER_DB_HEADER_KEY::Name), this};
  CHECK_NULLPTR_RETURN_VOID(m_perfSearch);
  m_perfToolbar = new (std::nothrow) QToolBar{"Performer tool", this};
  CHECK_NULLPTR_RETURN_VOID(m_perfToolbar);
  m_perfToolbar->addWidget(m_perfSearch);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_perfToolbar);

  setMenuBar(g_performersManagerActions().GetMenuBar());

  m_introTE = new (std::nothrow) PerformersPreviewTextBrowser{this};
  CHECK_NULLPTR_RETURN_VOID(m_introTE);
  m_perfPrevDock = new (std::nothrow) QDockWidget{tr("Overview"), this};
  CHECK_NULLPTR_RETURN_VOID(m_perfPrevDock);
  m_perfPrevDock->setWidget(m_introTE);
  addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_perfPrevDock);

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("open failed");
    return;
  }
  m_perfDbMdl = new (std::nothrow) RatingSqlTableModel{this, con};
  CHECK_NULLPTR_RETURN_VOID(m_perfDbMdl);
  if (con.tables().contains(DB_TABLE::PERFORMERS)) {
    m_perfDbMdl->setTable(DB_TABLE::PERFORMERS);
    m_perfDbMdl->submitAll();
  }
  m_perfTv = new (std::nothrow) PerformersTableView{this};
  CHECK_NULLPTR_RETURN_VOID(m_perfTv);
  m_perfTv->setModel(m_perfDbMdl);
  m_perfTv->InitTableView();
  setCentralWidget(m_perfTv);

  subscribe();
  readSettings();
  setWindowTitle("Performers Manager Widget");
  setWindowIcon(QIcon(":img/PERFORMERS_APP"));
}

void PerformersWidget::subscribe() {
  connect(m_perfSearch, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_perfSearch->text();
    m_perfDbMdl->setFilter(searchPattern);
  });

  connect(g_performersManagerActions().OPEN_WITH_LOCAL_APP, &QAction::triggered, this, [this]() {
    if (not QFile::exists(SystemPath::PEFORMERS_DATABASE)) {
      QMessageBox::information(this, "open failed", QString("[%1] not exists. \nCreate it first").arg(SystemPath::PEFORMERS_DATABASE));
      return;
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(SystemPath::PEFORMERS_DATABASE));
  });

  connect(g_performersManagerActions().INIT_DATABASE, &QAction::triggered,  //
          &mDb, &DbManager::CreateDatabase);
  connect(g_performersManagerActions().INIT_TABLE, &QAction::triggered, this, &PerformersWidget::onInitATable);
  connect(g_performersManagerActions().INSERT_INTO_TABLE, &QAction::triggered, this, &PerformersWidget::onInsertIntoTable);
  connect(g_performersManagerActions().DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DROP); });
  connect(g_performersManagerActions().DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DELETE); });

  connect(g_performersManagerActions().SUBMIT, &QAction::triggered, this, &PerformersWidget::onSubmit);

  connect(g_performersManagerActions().LOCATE_IMAGEHOST, &QAction::triggered, this, &PerformersWidget::onLocateImageHost);

  connect(g_performersManagerActions().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT, &QAction::triggered, this, &PerformersWidget::onChangePerformerImageHeight);

  connect(g_performersManagerActions().LOAD_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &PerformersWidget::onLoadFromFileSystemStructure);
  connect(g_performersManagerActions().LOAD_FROM_PERFORMERS_LIST, &QAction::triggered, this, &PerformersWidget::onLoadFromPerformersList);
  connect(g_performersManagerActions().LOAD_FROM_PJSON_PATH, &QAction::triggered, this, &PerformersWidget::onLoadFromPJsonDirectory);

  connect(g_performersManagerActions().DUMP_ALL_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &PerformersWidget::onDumpAllIntoPJsonFile);
  connect(g_performersManagerActions().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &PerformersWidget::onDumpIntoPJsonFile);

  connect(g_performersManagerActions().OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &PerformersWidget::onOpenRecordInFileSystem);

  connect(m_perfTv->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PerformersWidget::on_selectionChanged);

  connect(g_performersManagerActions().DELETE_RECORDS, &QAction::triggered, this, &PerformersWidget::onDeleteRecords);

  connect(g_performersManagerActions().REFRESH_SELECTED_RECORDS_VIDS, &QAction::triggered, this, &PerformersWidget::onForceRefreshRecordsVids);
  connect(g_performersManagerActions().REFRESH_ALL_RECORDS_VIDS, &QAction::triggered, this, &PerformersWidget::onForceRefreshAllRecordsVids);
}

void PerformersWidget::closeEvent(QCloseEvent* event) {
  g_performersManagerActions().PERFORMERS_BOOK->setChecked(false);
  PreferenceSettings().setValue("PerformersWidgetGeometry", saveGeometry());
  PreferenceSettings().setValue("PerformersWidgetDockerWidth", m_perfPrevDock->width());
  PreferenceSettings().setValue("PerformersWidgetDockerHeight", m_perfPrevDock->height());
  QMainWindow::closeEvent(event);
}

void PerformersWidget::readSettings() {
  if (PreferenceSettings().contains("PerformersWidgetGeometry")) {
    restoreGeometry(PreferenceSettings().value("PerformersWidgetGeometry").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 1024, 768));
  }
}

void PerformersWidget::onInitATable() {
  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  if (!mDb.CreateTable(DB_TABLE::PERFORMERS, PerfBaseDb::CREATE_PERF_TABLE_TEMPLATE)) {
    qWarning("Table[%s] create failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  m_perfDbMdl->setTable(DB_TABLE::PERFORMERS);
  m_perfDbMdl->submitAll();
  qDebug("Table[%s] create succeed", qPrintable(DB_TABLE::PERFORMERS));
}

bool PerformersWidget::onInsertIntoTable() {
  const QString& names = QInputDialog::getText(this,                   //
                                               "Insert performer(s)",  //
                                               "Performers can be split by '|'");
  if (names.isEmpty()) {
    Notificator::warning("Name is Empty", "Name cannot be empty");
    return false;
  }
  const QStringList& perfsLst = names.split('|');
  const int insertedSucceedCnt = mDb.InsertPerformers(perfsLst);
  if (insertedSucceedCnt < 0) {
    Notificator::badNews(QString("Insert failed code:%1").arg(insertedSucceedCnt), "See detail in logs");
    return false;
  }
  if (insertedSucceedCnt < perfsLst.size()) {
    Notificator::warning(QString("Insert [%1] into table partially failed").arg(names),
                         QString("Only %1/%2 succeed. see detail in description")  //
                             .arg(insertedSucceedCnt)
                             .arg(perfsLst.size()));
  }
  m_perfDbMdl->submitAll();
  Notificator::goodNews(QString("%1 performer(s) inserted ok").arg(perfsLst.size()), names);
  return true;
}

bool PerformersWidget::onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete) {
  const QString specifiedTablePattern{'^' + DB_TABLE::PERFORMERS + '$'};
  auto retBtn = QMessageBox::warning(this,                                                                           //
                                     QString("Confirm %1?").arg((int)dropOrDelete),                                  //
                                     "Drop(0)/Delete(1) [" + specifiedTablePattern + "] operation not recoverable",  //
                                     QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("User cancel drop/delete table[%s]", qPrintable(specifiedTablePattern));
    return true;
  }
  int rmvedTableCnt = mDb.RmvTable(specifiedTablePattern, dropOrDelete);
  if (rmvedTableCnt < 0) {
    QMessageBox::warning(this, "Drop/Delete failed", "see details in log");
    return false;
  }
  m_perfDbMdl->submitAll();
  Notificator::goodNews(QString("Operation: %1 on [%2]").arg((int)dropOrDelete).arg(specifiedTablePattern),  //
                        QString("Drop(0)/Delete(1). %1 table removed").arg(rmvedTableCnt));
  return rmvedTableCnt >= 0;
}

int PerformersWidget::onLoadFromFileSystemStructure() {
  if (m_perfDbMdl->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load from file-system structure");
    return false;
  }
  int succeedCnt = mDb.ReadFromImageHost(m_imageHostPath);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from path[%1] failed").arg(m_imageHostPath),  //
                         QString("see detail in description"));
    return 0;
  }
  m_perfDbMdl->submitAll();
  Notificator::goodNews(QString("load %1 performer(s) succeed").arg(succeedCnt), m_imageHostPath);
  return succeedCnt;
}

int PerformersWidget::onLoadFromPerformersList() {
  if (m_perfDbMdl->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load from file-system structure");
    return false;
  }
  bool ok = false;
  const QString& perfsText =                             //
      QInputDialog::getMultiLineText(this,               //
                                     "Performers List",  //
                                     "Example:\n Guardiola, Pep\nHuge Jackman, Slu", "", &ok);
  if (!ok) {
    Notificator::information("User cancel", "skip");
    return 0;
  }
  int succeedCnt = mDb.ReadFromUserInputSentence(perfsText);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from text[%1] failed").arg(perfsText),  //
                         QString("see detail in description"));
    return 0;
  }
  m_perfDbMdl->submitAll();
  Notificator::goodNews(QString("load %1 performer(s) succeed").arg(succeedCnt), perfsText);
  return succeedCnt;
}

bool PerformersWidget::onLocateImageHost() {
  const QString& locatePath = QFileDialog::getExistingDirectory(this, "Locate imagehost folder", m_imageHostPath);
  if (!QFile::exists(locatePath)) {
    qWarning("locate path not exist");
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, m_imageHostPath = locatePath);
  g_performersManagerActions().LOCATE_IMAGEHOST->setToolTip(m_imageHostPath);
  return true;
}

bool PerformersWidget::onChangePerformerImageHeight() {
  bool ok = false;
  int height = QInputDialog::getInt(this, "Performer image height(px)", QString("default: %1").arg(m_performerImageHeight), m_performerImageHeight, 0, INT_MAX, 1, &ok);
  if (not ok) {
    return false;
  }
  m_performerImageHeight = height;
  PreferenceSettings().setValue(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, m_performerImageHeight);
  g_performersManagerActions().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(QString::number(height));
  return true;
}

inline bool PerformersWidget::onSubmit() {
  if (!m_perfDbMdl->isDirty()) {
    qDebug("No need to submit");
    return true;
  }
  return m_perfDbMdl->submitAll();
}

bool PerformersWidget::on_selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
  if (not m_perfTv->currentIndex().isValid()) {
    m_introTE->setText("");
    return true;
  }
  const auto& record = m_perfDbMdl->record(m_perfTv->currentIndex().row());
  m_introTE->operator()(record, m_imageHostPath, m_performerImageHeight);
  return true;
}

int PerformersWidget::onLoadFromPJsonDirectory() {
  if (m_perfDbMdl->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load pjson");
    return false;
  }

  auto succeedCnt = mDb.LoadFromPJsonFile(m_imageHostPath);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from pJson[%1/*.pjson] failed").arg(m_imageHostPath),  //
                         QString("see detail in description"));
    return 0;
  }
  m_perfDbMdl->submitAll();

  Notificator::goodNews(QString("%1 pjson file load succeed").arg(succeedCnt), m_imageHostPath);
  return succeedCnt;
}

int PerformersWidget::onDumpAllIntoPJsonFile() {
  if (!QDir(m_imageHostPath).exists()) {
    Notificator::badNews("Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (int r = 0; r < m_perfDbMdl->rowCount(); ++r) {
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfDbMdl->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonFileHelper::MovieJsonDumper(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("All %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  Notificator::goodNews(QString("All %1 record(s) dumped result").arg(dumpCnt), QString("%1/%2 succeed").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int PerformersWidget::onDumpIntoPJsonFile() {
  if (!QDir(m_imageHostPath).exists()) {
    Notificator::badNews("Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }

  if (!m_perfTv->selectionModel()->hasSelection()) {
    Notificator::badNews("Nothing was selected", "Select some row to dump");
    return 0;
  }

  int dumpCnt = 0;
  int succeedCnt = 0;
  for (const auto& indr : m_perfTv->selectionModel()->selectedRows()) {
    const int r = indr.row();
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfDbMdl->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonFileHelper::MovieJsonDumper(pJson, pJsonPath);
    ++dumpCnt;
  }

  Notificator::goodNews(QString("Selected %1 record(s) dumped result").arg(dumpCnt), QString("%1/%2 succeed").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int PerformersWidget::onForceRefreshAllRecordsVids() {
  QMessageBox::information(this, QString("Oops function not support now"), QString("But you could selected all record(s) and then force refresh instead."));
  return 0;
}

int PerformersWidget::onForceRefreshRecordsVids() {
  if (!m_perfTv->selectionModel()->hasSelection()) {
    Notificator::badNews("Nothing was selected", "Select some row to refresh");
    return 0;
  }

  MovieBaseDb movieDb{SystemPath::VIDS_DATABASE, "SEARCH_MOVIE_BY_PERFORMER"};
  QSqlDatabase con = movieDb.GetDb();  // videos table
  if (!movieDb.CheckValidAndOpen(con)) {
    qWarning("Open failed:%s", qPrintable(con.lastError().text()));
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
  for (auto indr : m_perfTv->selectionModel()->selectedRows()) {
    const int r = indr.row();
    auto record = m_perfDbMdl->record(r);
    const QStringList& vidsList = GetVidsListFromVidsTable(record, qur);
    record.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidsList.join(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR));
    m_perfDbMdl->setRecord(r, record);  // update back
    vidsCnt += vidsList.size();
    if (recordsCnt == 0) {
      m_introTE->operator()(record, m_imageHostPath, m_performerImageHeight);
    }
    ++recordsCnt;
  }
  qDebug("Selected %d record(s) updated %d vid(s).", recordsCnt, vidsCnt);
  QMessageBox::information(this, QString("Selected %1 record(s) updated.").arg(recordsCnt), QString("%1 vid(s)").arg(vidsCnt));
  return recordsCnt;
}

bool PerformersWidget::onOpenRecordInFileSystem() const {
  if (!QDir(m_imageHostPath).exists()) {
    qWarning("m_imageHostPath [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (!m_perfTv->selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select a row to open pjson folder");
    return false;
  }

  const auto& record = m_perfDbMdl->record(m_perfTv->currentIndex().row());
  QString folderPath =
      QString("%1/%2/%3").arg(m_imageHostPath).arg(record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString()).arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  if (m_perfTv->currentIndex().column() == PERFORMER_DB_HEADER_KEY::Detail_INDEX) {
    folderPath += QString("/%1.pjson").arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  }
  if (!QFile::exists(folderPath)) {
    qDebug("Path[%s] not exists", qPrintable(folderPath));
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

int PerformersWidget::onDeleteRecords() {
  if (!m_perfTv->selectionModel()->hasSelection()) {
    Notificator::badNews("Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  int deleteCnt = 0;
  int succeedCnt = 0;
  const auto& itemSelection = m_perfTv->selectionModel()->selection();
  for (auto it = itemSelection.crbegin(); it != itemSelection.crend(); ++it) {
    int startRow = it->top();  // [top, bottom]
    int size = it->bottom() - startRow + 1;
    bool ret = m_perfDbMdl->removeRows(startRow, size);
    qDebug("drop[%d] records [%d, %d]", ret, startRow, it->bottom());
    deleteCnt += size;
    succeedCnt += ((int)ret * size);
  }
  m_perfDbMdl->submitAll();
  Notificator::goodNews("delete records result", QString("%1/%2 succeed").arg(succeedCnt).arg(deleteCnt));
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
