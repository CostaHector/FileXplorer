#include "CastDBView.h"

#include "CastDBActions.h"
#include "FdBasedDb.h"
#include "FileFolderPreviewer.h"
#include "JsonHelper.h"
#include "MemoryKey.h"
#include "Notificator.h"
#include "PathTool.h"
#include "PerformerJsonFileHelper.h"
#include "PerformersAkaManager.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "RatingSqlTableModel.h"
#include "StringTool.h"
#include "TableFields.h"

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

QString CastDBView::GetImageHostPath() {
  return Configuration().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,  //
                               MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v)
      .toString();
}

CastDBView::CastDBView(QLineEdit* perfSearchLE, FileFolderPreviewer* floatingPreview, QWidget* parent)
  : CustomTableView{"PERFORMERS_TABLE", parent},  //
  m_perfSearch{perfSearchLE},
  _floatingPreview{floatingPreview},
  mDb{SystemPath::PEFORMERS_DATABASE, "perfs_connection"}

{
  CHECK_NULLPTR_RETURN_VOID(m_perfSearch);
  BindMenu(g_castAct().GetRightClickMenu());

  QSqlDatabase con = mDb.GetDb();
  if (!mDb.CheckValidAndOpen(con)) {
    qWarning("open failed");
    return;
  }
  m_castModel = new (std::nothrow) RatingSqlTableModel{this, con};
  CHECK_NULLPTR_RETURN_VOID(m_castModel);
  if (con.tables().contains(DB_TABLE::PERFORMERS)) {
    m_castModel->setTable(DB_TABLE::PERFORMERS);
    m_castModel->submitAll();
  }

  setModel(m_castModel);
  InitTableView();

  subscribe();
  setWindowTitle("Cast Manager Widget");
  setWindowIcon(QIcon(":img/CAST_VIEW"));
}

void CastDBView::subscribe() {
  connect(m_perfSearch, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_perfSearch->text();
    m_castModel->setFilter(searchPattern);
  });

  connect(g_castAct().SUBMIT, &QAction::triggered, this, &CastDBView::onSubmit);
  connect(g_castAct().INSERT_INTO_TABLE, &QAction::triggered, this, &CastDBView::onInsertIntoTable);
  connect(g_castAct().DELETE_RECORDS, &QAction::triggered, this, &CastDBView::onDeleteRecords);
  connect(g_castAct().INIT_DATABASE, &QAction::triggered, &mDb, &DbManager::CreateDatabase);
  connect(g_castAct().INIT_TABLE, &QAction::triggered, this, &CastDBView::onInitATable);
  connect(g_castAct().DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DROP); });
  connect(g_castAct().DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DELETE); });

  connect(g_castAct().SYNC_SELECTED_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onForceRefreshRecordsVids);
  connect(g_castAct().SYNC_ALL_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onForceRefreshAllRecordsVids);

  connect(g_castAct().OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &mDb, &DbManager::ShowInFileSystemView);
  connect(g_castAct().OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &CastDBView::onOpenRecordInFileSystem);

  connect(g_castAct().APPEND_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &CastDBView::onLoadFromFileSystemStructure);
  connect(g_castAct().APPEND_FROM_MULTILINES_INPUT, &QAction::triggered, this, &CastDBView::onLoadFromPerformersList);
  connect(g_castAct().APPEND_FROM_PJSON_FILES, &QAction::triggered, this, &CastDBView::onLoadFromPJsonDirectory);

  connect(g_castAct().DUMP_ALL_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &CastDBView::onDumpAllIntoPJsonFile);
  connect(g_castAct().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &CastDBView::onDumpIntoPJsonFile);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &CastDBView::on_selectionChanged);
}

void CastDBView::onInitATable() {
  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  if (!mDb.CreateTable(DB_TABLE::PERFORMERS, CastBaseDb::CREATE_PERF_TABLE_TEMPLATE)) {
    qWarning("Table[%s] create failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  m_castModel->setTable(DB_TABLE::PERFORMERS);
  m_castModel->submitAll();
  qDebug("Table[%s] create succeed", qPrintable(DB_TABLE::PERFORMERS));
}

bool CastDBView::onInsertIntoTable() {
  const QString& names = QInputDialog::getText(this,                   //
                                               "Insert performer(s)",  //
                                               "Cast can be split by '|'");
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
  m_castModel->submitAll();
  Notificator::goodNews(QString("%1 performer(s) inserted ok").arg(perfsLst.size()), names);
  return true;
}

bool CastDBView::onDropDeleteTable(const DbManager::DROP_OR_DELETE dropOrDelete) {
  auto retBtn = QMessageBox::warning(this,                                                                          //
                                     QString("Confirm %1?").arg((int)dropOrDelete),                                 //
                                     "Drop(0)/Delete(1) [" + DB_TABLE::PERFORMERS + "] operation not recoverable",  //
                                     QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
  if (retBtn != QMessageBox::StandardButton::Yes) {
    qDebug("User cancel drop/delete table[%s]", qPrintable(DB_TABLE::PERFORMERS));
    return true;
  }
  int rmvedTableCnt = mDb.RmvTable(DB_TABLE::PERFORMERS, dropOrDelete);
  if (rmvedTableCnt < 0) {
    Notificator::badNews("Drop/Delete failed", "see details in log");
    return false;
  }
  m_castModel->submitAll();
  Notificator::goodNews(QString("Operation: %1 on [%2]").arg((int)dropOrDelete).arg(DB_TABLE::PERFORMERS),  //
                        QString("Drop(0)/Delete(1). %1 table removed").arg(rmvedTableCnt));
  return rmvedTableCnt >= 0;
}

int CastDBView::onLoadFromFileSystemStructure() {
  if (m_castModel->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load from file-system structure");
    return false;
  }
  const QString imageHostPath{GetImageHostPath()};
  int succeedCnt = mDb.ReadFromImageHost(imageHostPath);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from path[%1] failed").arg(imageHostPath),  //
                         QString("see detail in description"));
    return 0;
  }
  m_castModel->submitAll();
  Notificator::goodNews(QString("load %1 performer(s) succeed").arg(succeedCnt), imageHostPath);
  return succeedCnt;
}

int CastDBView::onLoadFromPerformersList() {
  if (m_castModel->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load from file-system structure");
    return false;
  }
  bool ok = false;
  const QString& perfsText =                             //
      QInputDialog::getMultiLineText(this,               //
                                     "Cast List",  //
                                     "Example:\n Guardiola, Pep\nHuge Jackman, Wolverine", "", &ok);
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
  m_castModel->submitAll();
  Notificator::goodNews(QString("load %1 performer(s) succeed").arg(succeedCnt), perfsText);
  return succeedCnt;
}

bool CastDBView::onSubmit() {
  CHECK_NULLPTR_RETURN_FALSE(m_castModel)

  if (!m_castModel->isDirty()) {
    Notificator::goodNews("Table not dirty, Skip", DB_TABLE::PERFORMERS);
    return true;
  }
  if (!m_castModel->submitAll()) {
    Notificator::badNews("Submit failed. see details in logs", DB_TABLE::PERFORMERS);
    return false;
  }

  Notificator::goodNews("Submit succeed. Following .db has been saved", DB_TABLE::PERFORMERS);
  return true;
}

bool CastDBView::on_selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
  if (!currentIndex().isValid()) {
    return true;
  }
  CHECK_NULLPTR_RETURN_FALSE(_floatingPreview);
  const auto& record = m_castModel->record(currentIndex().row());
  const QString imageHostPath{GetImageHostPath()};
  _floatingPreview->operator()(record, imageHostPath);
  return true;
}

int CastDBView::onLoadFromPJsonDirectory() {
  CHECK_NULLPTR_RETURN_FALSE(m_castModel)

  if (m_castModel->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load pjson");
    return false;
  }
  const QString imageHostPath{GetImageHostPath()};
  int succeedCnt = mDb.LoadFromPJsonFile(imageHostPath);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from pJson[%1/*.pjson] failed").arg(imageHostPath),  //
                         QString("see detail in description"));
    return 0;
  }
  m_castModel->submitAll();

  Notificator::goodNews(QString("%1 pjson file load succeed").arg(succeedCnt), imageHostPath);
  return succeedCnt;
}

int CastDBView::onDumpAllIntoPJsonFile() {
  const QString imageHostPath{GetImageHostPath()};
  if (!QDir(imageHostPath).exists()) {
    Notificator::badNews("Path[pjson dump to] not exist", imageHostPath);
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (int r = 0; r < m_castModel->rowCount(); ++r) {
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_castModel->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(imageHostPath, pJson);
    succeedCnt += JsonHelper::DumpJsonDict(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("All %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  Notificator::goodNews(QString("All %1 record(s) dumped result").arg(dumpCnt), QString("%1/%2 succeed").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int CastDBView::onDumpIntoPJsonFile() {
  const QString imageHostPath{GetImageHostPath()};
  if (!QDir{imageHostPath}.exists()) {
    Notificator::badNews("Path[pjson dump to] not exist", imageHostPath);
    return 0;
  }

  if (!selectionModel()->hasSelection()) {
    Notificator::information("Nothing was selected", "Select some row to dump");
    return 0;
  }

  QDir imageHostDir{imageHostPath};
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    const QVariantHash pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_castModel->record(r));
    const QString ori {pJson[PERFORMER_DB_HEADER_KEY::Orientation].toString()};
    const QString castName {pJson[PERFORMER_DB_HEADER_KEY::Name].toString()};
    const QString prepath {ori + '/' + castName};
    if (!imageHostDir.exists(prepath) && !imageHostDir.mkpath(prepath)) {
      qWarning("Create folder [%s] under [%s] failed", qPrintable(prepath), qPrintable(imageHostPath));
      continue;
    }
    const QString pJsonPath {PerformerJsonFileHelper::PJsonPath(imageHostPath, ori, castName)};
    succeedCnt += JsonHelper::DumpJsonDict(pJson, pJsonPath);
    ++dumpCnt;
  }

  Notificator::goodNews(QString("Selected %1 record(s) dumped result").arg(dumpCnt), QString("%1/%2 succeed").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int CastDBView::onForceRefreshAllRecordsVids() {
  selectAll();
  LOG_INFO("Refresh all records may cause lag", "Click Refresh Selected action if you are sure");
  return 0;
}

QStringList GetVidsListFromVidsTable(const QSqlRecord& record, QSqlQuery& query) {
  using namespace MOVIE_TABLE;
  static auto& dbTM = PerformersAkaManager::getIns();
  const QString& searchCommand = dbTM.GetMovieTablePerformerSelectCommand(record);
  if (!query.exec(searchCommand)) {
    qWarning("Query[%s] failed: %s", qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return {};
  }
  QStringList vidPath;
  vidPath.reserve(query.size());
  while (query.next()) {
    vidPath.push_back(PathTool::Path3Join(query.value(ENUM_2_STR(PrePathLeft)).toString(),   //
                                          query.value(ENUM_2_STR(PrePathRight)).toString(),  //
                                          query.value(ENUM_2_STR(Name)).toString()));
  }
  qDebug("%d records finded", vidPath.size());
  return vidPath;
}

int CastDBView::onForceRefreshRecordsVids() {
  if (!selectionModel()->hasSelection()) {
    Notificator::badNews("Nothing was selected", "Select some row to refresh");
    return 0;
  }

  FdBasedDb movieDb{SystemPath::VIDS_DATABASE, "SEARCH_MOVIE_BY_PERFORMER"};
  QSqlDatabase con = movieDb.GetDb();  // videos table
  if (!movieDb.CheckValidAndOpen(con)) {
    qWarning("Open failed:%s", qPrintable(con.lastError().text()));
    return 0;
  }
  const QString imageHostPath{GetImageHostPath()};
  QSqlQuery qur{con};
  int recordsCnt = 0;
  int vidsCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    auto record = m_castModel->record(r);
    const QStringList& vidsList = GetVidsListFromVidsTable(record, qur);
    record.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidsList.join(StringTool::PERFS_VIDS_IMGS_SPLIT_CHAR));
    m_castModel->setRecord(r, record);  // update back

    vidsCnt += vidsList.size();
    if (recordsCnt == 0 && _floatingPreview != nullptr) {  // update cast browser for first selected record
      const auto& record = m_castModel->record(currentIndex().row());
      _floatingPreview->operator()(record, imageHostPath);
    }
    ++recordsCnt;
  }
  QString msgTitle{QString{"%1 records selection"}.arg(recordsCnt)};
  QString msgDetail{QString{"%1 videos(s) updated succeed"}.arg(vidsCnt)};
  Notificator::goodNews(msgTitle, msgDetail);
  return recordsCnt;
}

bool CastDBView::onOpenRecordInFileSystem() const {
  const QString imageHostPath{GetImageHostPath()};
  if (!QDir(imageHostPath).exists()) {
    qWarning("imgHost [%s] not exists", qPrintable(imageHostPath));
    return false;
  }
  if (!selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select a row to open pjson folder");
    return false;
  }

  const auto& record = m_castModel->record(currentIndex().row());
  QString folderPath = QString{"%1/%2/%3"}                                                              //
                           .arg(imageHostPath)                                                        //
                           .arg(record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString())  //
                           .arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  if (currentIndex().column() == PERFORMER_DB_HEADER_KEY::Detail_INDEX) {
    folderPath += QString("/%1.pjson").arg(record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString());
  }
  if (!QFile::exists(folderPath)) {
    qDebug("Path[%s] not exists", qPrintable(folderPath));
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

int CastDBView::onDeleteRecords() {
  if (!selectionModel()->hasSelection()) {
    Notificator::badNews("Nothing was selected", "Select some row(s) to delete");
    return 0;
  }
  int deleteCnt = 0;
  int succeedCnt = 0;
  const auto& itemSelection = selectionModel()->selection();
  for (auto it = itemSelection.crbegin(); it != itemSelection.crend(); ++it) {
    int startRow = it->top();  // [top, bottom]
    int size = it->bottom() - startRow + 1;
    bool ret = m_castModel->removeRows(startRow, size);
    qDebug("drop[%d] records [%d, %d]", ret, startRow, it->bottom());
    deleteCnt += size;
    succeedCnt += ((int)ret * size);
  }
  m_castModel->submitAll();
  Notificator::goodNews("delete records result", QString("%1/%2 succeed").arg(succeedCnt).arg(deleteCnt));
  return succeedCnt;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  CastDBView perfManaWid;
  perfManaWid.show();
  a.exec();
  return 0;
}
#endif
