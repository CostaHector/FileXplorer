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

CastDBView::CastDBView(CastDatabaseSearchToolBar* castDbSearchBar_, FileFolderPreviewer* floatingPreview_, QWidget* parent)
  : CustomTableView{"PERFORMERS_TABLE", parent},  //
  _castDbSearchBar{castDbSearchBar_},
  _floatingPreview{floatingPreview_},
  mDb{SystemPath::PEFORMERS_DATABASE, "CAST_CONNECTION"},
  mImageHost{Configuration().value(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name,  //
                                   MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.v)
                 .toString()}
{
  if (!QFileInfo{mImageHost}.isDir()) {
    LOG_CRITICAL("Image host path not exist", mImageHost);
    return;
  }

  CHECK_NULLPTR_RETURN_VOID(_castDbSearchBar);
  BindMenu(g_castAct().GetRightClickMenu(this));

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

bool CastDBView::onOpenRecordInFileSystem() const {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO("Nothing was selected.", "Select a row to open pson folder");
    return false;
  }
  const auto& record = m_castModel->record(currentIndex().row());
  QString revealPath{currentIndex().column() == PERFORMER_DB_HEADER_KEY::Detail ?
                     CastBaseDb::GetCastFilePath(record, mImageHost):
                     CastBaseDb::GetCastPath(record, mImageHost)};
  if (!QFile::exists(revealPath)) {
    LOG_WARN("Path not exists", revealPath);
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(revealPath));
}


void CastDBView::subscribe() {
  connect(_castDbSearchBar, &CastDatabaseSearchToolBar::whereClauseChanged, m_castModel, &QSqlTableModel::setFilter);

  auto& castInst = g_castAct();
  connect(castInst.SUBMIT, &QAction::triggered, this, &CastDBView::onSubmit);
  connect(castInst.APPEND_FROM_MULTILINES_INPUT, &QAction::triggered, this, &CastDBView::onAppendCasts);
  connect(castInst.DELETE_RECORDS, &QAction::triggered, this, &CastDBView::onDeleteRecords);
  connect(castInst.INIT_DATABASE, &QAction::triggered, &mDb, &DbManager::CreateDatabase);
  connect(castInst.INIT_TABLE, &QAction::triggered, this, &CastDBView::onInitATable);
  connect(castInst.DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DROP); });
  connect(castInst.DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DELETE); });

  connect(castInst.SYNC_SELECTED_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBView::onSyncImgsFieldFromImageHost);
  connect(castInst.SYNC_ALL_RECORDS_IMGS_FROM_DISK, &QAction::triggered, this, &CastDBView::onSyncAllImgsFieldFromImageHost);
  connect(castInst.SYNC_SELECTED_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onForceRefreshRecordsVids);
  connect(castInst.SYNC_ALL_RECORDS_VIDS_FROM_DB, &QAction::triggered, this, &CastDBView::onForceRefreshAllRecordsVids);

  connect(castInst.OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &mDb, &DbManager::ShowInFileSystemView);
  connect(castInst.OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &CastDBView::onOpenRecordInFileSystem);

  connect(castInst.APPEND_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &CastDBView::onLoadFromFileSystemStructure);
  connect(castInst.APPEND_FROM_PSON_FILES, &QAction::triggered, this, &CastDBView::onLoadFromPsonDirectory);

  connect(castInst.DUMP_ALL_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBView::onDumpAllIntoPsonFile);
  connect(castInst.DUMP_SELECTED_RECORDS_INTO_PSON_FILE, &QAction::triggered, this, &CastDBView::onDumpIntoPsonFile);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &CastDBView::on_selectionChanged);
}

QString CastDBView::filePath(const QModelIndex& index) const {
  if (m_castModel == nullptr || !index.isValid()){
    return "";
  }
  const auto& record = m_castModel->record(currentIndex().row());
  return CastBaseDb::GetCastFilePath(record, mImageHost);
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

int CastDBView::onAppendCasts() {
  if (m_castModel->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load from file-system structure");
    return false;
  }
  bool ok = false;
  const QString& perfsText =                             //
      QInputDialog::getMultiLineText(this,               //
                                     "Input 'Casts, aka'",  //
                                     "Example:\n Guardiola, Pep\nHuge Jackman, Wolverine", "", &ok);
  if (!ok) {
    Notificator::information("User cancel", "skip");
    return 0;
  }
  int succeedCnt = mDb.AppendCastFromMultiLineInput(perfsText);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from text[%1] failed").arg(perfsText),  //
                         "see detail in description");
    return 0;
  }
  m_castModel->submitAll();
  Notificator::goodNews(QString("load %1 performer(s) succeed").arg(succeedCnt), perfsText);
  return succeedCnt;
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
  int succeedCnt = mDb.ReadFromImageHost(mImageHost);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from path[%1] failed").arg(mImageHost),  //
                         "see detail in description");
    return 0;
  }
  m_castModel->submitAll();
  Notificator::goodNews(QString("load %1 performer(s) succeed").arg(succeedCnt), mImageHost);
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
  _floatingPreview->operator()(record, mImageHost);
  return true;
}

int CastDBView::onLoadFromPsonDirectory() {
  if (m_castModel->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load pson");
    return 0;
  }
  int succeedCnt = mDb.LoadFromPsonFile(mImageHost);
  if (succeedCnt < 0) {
    Notificator::warning(QString("Load perfs from pJson[%1/*.pson] failed").arg(mImageHost),  //
                         "see detail in description");
    return succeedCnt;
  }
  m_castModel->submitAll();
  Notificator::goodNews(QString("%1 pson file load succeed").arg(succeedCnt), mImageHost);
  return succeedCnt;
}

int CastDBView::onSyncAllImgsFieldFromImageHost() {
  const int totalCnt{m_castModel->rowCount()};
  if (totalCnt == 0) {
    LOG_INFO("No records at all skip", "No need sync");
    return 0;
  }
  int succeedCnt = 0;
  for (int r = 0; r < m_castModel->rowCount(); ++r) {
    QSqlRecord sqlRecord = m_castModel->record(r);
    succeedCnt += CastBaseDb::UpdateRecordImgsField(sqlRecord, mImageHost);
    m_castModel->setRecord(r, sqlRecord);
  }
  RefreshHtmlContents();
  QString msgTitle{QString("All %1 record(s) imgs field been sync").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN(msgTitle, msgDetail);
  } else {
    Notificator::goodNews(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onSyncImgsFieldFromImageHost() {
  if (!selectionModel()->hasSelection()) {
    Notificator::information("Nothing was selected", "Select some row to sync imgs fields");
    return 0;
  }
  currentIndex();

  const int totalCnt{selectionModel()->selectedRows().size()};
  int succeedCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    QSqlRecord sqlRecord = m_castModel->record(r);
    succeedCnt += CastBaseDb::UpdateRecordImgsField(sqlRecord, mImageHost);
    m_castModel->setRecord(r, sqlRecord);
  }
  RefreshHtmlContents();
  QString msgTitle{QString("%1 record(s) selected imgs field been sync").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN(msgTitle, msgDetail);
  } else {
    Notificator::goodNews(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onDumpAllIntoPsonFile() {
  const int totalCnt{m_castModel->rowCount()};
  if (totalCnt == 0) {
    LOG_INFO("No records at all skip", "No need dump");
    return 0;
  }
  int succeedCnt = 0;
  for (int r = 0; r < m_castModel->rowCount(); ++r) {
    const auto& pson = PerformerJsonFileHelper::PerformerJsonJoiner(m_castModel->record(r));
    const QString& psonPath = PerformerJsonFileHelper::PsonPath(mImageHost, pson);
    succeedCnt += JsonHelper::DumpJsonDict(pson, psonPath);
  }
  QString msgTitle{QString("All %1 record(s) dumped result").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN(msgTitle, msgDetail);
  } else {
    Notificator::goodNews(msgTitle, msgDetail);
  }
  return succeedCnt;
}

int CastDBView::onDumpIntoPsonFile() {
  if (!selectionModel()->hasSelection()) {
    Notificator::information("Nothing was selected", "Select some row to dump");
    return 0;
  }

  QDir imageHostDir{mImageHost};
  int totalCnt {selectionModel()->selectedRows().size()};
  int succeedCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    const auto& record = m_castModel->record(r);
    const QString ori {record.value(PERFORMER_DB_HEADER_KEY::Ori).toString()};
    const QString castName {record.value(PERFORMER_DB_HEADER_KEY::Name).toString()};
    const QString prepath {ori + '/' + castName};
    if (!imageHostDir.exists(prepath) && !imageHostDir.mkpath(prepath)) {
      qWarning("Create folder [%s] under [%s] failed", qPrintable(prepath), qPrintable(mImageHost));
      continue;
    }
    const QString psonPath {PerformerJsonFileHelper::PsonPath(mImageHost, ori, castName)};
    const QVariantHash pson = PerformerJsonFileHelper::PerformerJsonJoiner(record);
    succeedCnt += JsonHelper::DumpJsonDict(pson, psonPath);
  }

  QString msgTitle{QString("Selected %1 record(s) dumped result").arg(totalCnt)};
  QString msgDetail{QString("%1/%2 succeed").arg(succeedCnt).arg(totalCnt)};
  if (totalCnt != succeedCnt) {
    LOG_WARN(msgTitle, msgDetail);
  } else {
    Notificator::goodNews(msgTitle, msgDetail);
  }
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
    ++recordsCnt;
  }
  RefreshHtmlContents();
  QString msgTitle{QString{"%1 records selection"}.arg(recordsCnt)};
  QString msgDetail{QString{"%1 videos(s) updated succeed"}.arg(vidsCnt)};
  Notificator::goodNews(msgTitle, msgDetail);
  return recordsCnt;
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

void CastDBView::RefreshHtmlContents() {
  if (_floatingPreview == nullptr || !selectionModel()->hasSelection()) {
    return;
  }
  const auto& record = m_castModel->record(currentIndex().row());
  _floatingPreview->operator()(record, mImageHost);
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
