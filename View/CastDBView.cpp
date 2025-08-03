#include "CastDBView.h"

#include "Actions/CastDBActions.h"
#include "Component/FolderPreview/FloatingPreview.h"
#include "Component/RatingSqlTableModel.h"
#include "Component/Notificator.h"
#include "Tools/FileDescriptor/FdBasedDb.h"
#include "public/PathTool.h"
#include "public/PublicMacro.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Tools/Json/JsonHelper.h"
#include "Tools/PerformerJsonFileHelper.h"
#include "Tools/PerformersAkaManager.h"
#include "Tools/FileDescriptor/TableFields.h"

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

CastDBView::CastDBView(QLineEdit* perfSearchLE, FloatingPreview* floatingPreview, QWidget* parent)
    : CustomTableView{"PERFORMERS_TABLE", parent},  //
      m_perfSearch{perfSearchLE},
      _floatingPreview{floatingPreview},
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
  CHECK_NULLPTR_RETURN_VOID(m_perfSearch);

  BindMenu(g_castAct().GetRightClickMenu());
  AppendVerticalHeaderMenuAGS(g_castAct().GetVerAGS());
  AppendHorizontalHeaderMenuAGS(g_castAct().GetHorAGS());

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

  setModel(m_perfDbMdl);
  InitTableView();

  subscribe();
  setWindowTitle("Cast Manager Widget");
  setWindowIcon(QIcon(":img/PERFORMERS_APP"));
}

void CastDBView::subscribe() {
  connect(m_perfSearch, &QLineEdit::returnPressed, this, [this]() {
    const QString& searchPattern = m_perfSearch->text();
    m_perfDbMdl->setFilter(searchPattern);
  });

  connect(g_castAct().SUBMIT, &QAction::triggered, this, &CastDBView::onSubmit);
  connect(g_castAct().INSERT_INTO_TABLE, &QAction::triggered, this, &CastDBView::onInsertIntoTable);
  connect(g_castAct().DELETE_RECORDS, &QAction::triggered, this, &CastDBView::onDeleteRecords);
  connect(g_castAct().INIT_DATABASE, &QAction::triggered, &mDb, &DbManager::CreateDatabase);
  connect(g_castAct().INIT_TABLE, &QAction::triggered, this, &CastDBView::onInitATable);
  connect(g_castAct().DROP_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DROP); });
  connect(g_castAct().DELETE_TABLE, &QAction::triggered, this, [this]() { onDropDeleteTable(DbManager::DROP_OR_DELETE::DELETE); });

  connect(g_castAct().REFRESH_SELECTED_RECORDS_VIDS, &QAction::triggered, this, &CastDBView::onForceRefreshRecordsVids);
  connect(g_castAct().REFRESH_ALL_RECORDS_VIDS, &QAction::triggered, this, &CastDBView::onForceRefreshAllRecordsVids);

  connect(g_castAct().OPEN_DB_WITH_LOCAL_APP, &QAction::triggered, &mDb, &DbManager::ShowInFileSystemView);
  connect(g_castAct().OPEN_RECORD_IN_FILE_SYSTEM, &QAction::triggered, this, &CastDBView::onOpenRecordInFileSystem);
  connect(g_castAct().LOCATE_IMAGEHOST, &QAction::triggered, this, &CastDBView::onLocateImageHost);

  connect(g_castAct().LOAD_FROM_FILE_SYSTEM_STRUCTURE, &QAction::triggered, this, &CastDBView::onLoadFromFileSystemStructure);
  connect(g_castAct().LOAD_FROM_PERFORMERS_LIST, &QAction::triggered, this, &CastDBView::onLoadFromPerformersList);
  connect(g_castAct().LOAD_FROM_PJSON_PATH, &QAction::triggered, this, &CastDBView::onLoadFromPJsonDirectory);

  connect(g_castAct().DUMP_ALL_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &CastDBView::onDumpAllIntoPJsonFile);
  connect(g_castAct().DUMP_SELECTED_RECORDS_INTO_PJSON_FILE, &QAction::triggered, this, &CastDBView::onDumpIntoPJsonFile);

  connect(g_castAct().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT, &QAction::triggered, this, &CastDBView::onChangePerformerImageHeight);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this, &CastDBView::on_selectionChanged);
}

void CastDBView::onInitATable() {
  // UTF-8 each char takes 1 to 4 byte, 256 chars means 256~1024 bytes
  if (!mDb.CreateTable(DB_TABLE::PERFORMERS, PerfBaseDb::CREATE_PERF_TABLE_TEMPLATE)) {
    qWarning("Table[%s] create failed.", qPrintable(DB_TABLE::PERFORMERS));
    return;
  }
  m_perfDbMdl->setTable(DB_TABLE::PERFORMERS);
  m_perfDbMdl->submitAll();
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
  m_perfDbMdl->submitAll();
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
    QMessageBox::warning(this, "Drop/Delete failed", "see details in log");
    return false;
  }
  m_perfDbMdl->submitAll();
  Notificator::goodNews(QString("Operation: %1 on [%2]").arg((int)dropOrDelete).arg(DB_TABLE::PERFORMERS),  //
                        QString("Drop(0)/Delete(1). %1 table removed").arg(rmvedTableCnt));
  return rmvedTableCnt >= 0;
}

int CastDBView::onLoadFromFileSystemStructure() {
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

int CastDBView::onLoadFromPerformersList() {
  if (m_perfDbMdl->isDirty()) {
    Notificator::badNews("Table dirty", "submit before load from file-system structure");
    return false;
  }
  bool ok = false;
  const QString& perfsText =                             //
      QInputDialog::getMultiLineText(this,               //
                                     "Cast List",  //
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

bool CastDBView::onLocateImageHost() {
  const QString& locatePath = QFileDialog::getExistingDirectory(this, "Locate imagehost folder", m_imageHostPath);
  if (!QFile::exists(locatePath)) {
    Notificator::badNews("cannot open", "locate path not exist");
    qWarning("locate path not exist");
    return false;
  }
  PreferenceSettings().setValue(MemoryKey::PATH_PERFORMER_IMAGEHOST_LOCATE.name, m_imageHostPath = locatePath);
  g_castAct().LOCATE_IMAGEHOST->setToolTip(m_imageHostPath);
  return true;
}

bool CastDBView::onChangePerformerImageHeight() {
  bool ok = false;
  int height = QInputDialog::getInt(this, "Cast image height(px)", QString::number(m_performerImageHeight), m_performerImageHeight, 0, INT_MAX, 1, &ok);
  if (!ok) {
    return false;
  }
  m_performerImageHeight = height;
  g_castAct().CHANGE_PERFORMER_IMAGE_FIXED_HEIGHT->setToolTip(QString::number(height));
  PreferenceSettings().setValue(MemoryKey::PERFORMER_IMAGE_FIXED_HEIGHT.name, m_performerImageHeight);
  return true;
}

inline bool CastDBView::onSubmit() {
  if (m_perfDbMdl == nullptr) {
    qCritical("_dbModel is nullptr");
    return false;
  }

  if (!m_perfDbMdl->isDirty()) {
    Notificator::goodNews("Table not dirty, Skip", DB_TABLE::PERFORMERS);
    return true;
  }
  if (!m_perfDbMdl->submitAll()) {
    Notificator::badNews("Submit failed. see details in logs", DB_TABLE::PERFORMERS);
    return false;
  }

  Notificator::goodNews("Submit succeed", DB_TABLE::PERFORMERS);
  return true;
}

bool CastDBView::on_selectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/) {
  if (!currentIndex().isValid()) {
    return true;
  }
  CHECK_NULLPTR_RETURN_FALSE(_floatingPreview);
  const auto& record = m_perfDbMdl->record(currentIndex().row());
  _floatingPreview->operator()(record, m_imageHostPath, m_performerImageHeight);
  return true;
}

int CastDBView::onLoadFromPJsonDirectory() {
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

int CastDBView::onDumpAllIntoPJsonFile() {
  if (!QDir(m_imageHostPath).exists()) {
    Notificator::badNews("Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }
  int dumpCnt = 0;
  int succeedCnt = 0;
  for (int r = 0; r < m_perfDbMdl->rowCount(); ++r) {
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfDbMdl->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonHelper::DumpJsonDict(pJson, pJsonPath);
    ++dumpCnt;
  }
  qDebug("All %d record(s) dump into pjson file. succeed: %d/%d.", dumpCnt, succeedCnt, dumpCnt);
  Notificator::goodNews(QString("All %1 record(s) dumped result").arg(dumpCnt), QString("%1/%2 succeed").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int CastDBView::onDumpIntoPJsonFile() {
  if (!QDir(m_imageHostPath).exists()) {
    Notificator::badNews("Path[pjson dump to] not exist", m_imageHostPath);
    return 0;
  }

  if (!selectionModel()->hasSelection()) {
    Notificator::badNews("Nothing was selected", "Select some row to dump");
    return 0;
  }

  int dumpCnt = 0;
  int succeedCnt = 0;
  for (const auto& indr : selectionModel()->selectedRows()) {
    const int r = indr.row();
    const auto& pJson = PerformerJsonFileHelper::PerformerJsonJoiner(m_perfDbMdl->record(r));
    const QString& pJsonPath = PerformerJsonFileHelper::PJsonPath(m_imageHostPath, pJson);
    succeedCnt += JsonHelper::DumpJsonDict(pJson, pJsonPath);
    ++dumpCnt;
  }

  Notificator::goodNews(QString("Selected %1 record(s) dumped result").arg(dumpCnt), QString("%1/%2 succeed").arg(succeedCnt).arg(dumpCnt));
  return succeedCnt;
}

int CastDBView::onForceRefreshAllRecordsVids() {
  QMessageBox::information(this, QString("Oops function not support now"), QString("But you could selected all record(s) and then force refresh instead."));
  return 0;
}

QStringList GetVidsListFromVidsTable(const QSqlRecord& record, QSqlQuery& query) {
  using namespace MOVIE_TABLE;
  static auto& dbTM = PerformersAkaManager::getIns();
  const QString& searchCommand = dbTM.GetMovieTablePerformerSelectCommand(record);
  if (!query.exec(searchCommand)) {
    qWarning("Query[%s] failed: %s",  //
             qPrintable(query.executedQuery()), qPrintable(query.lastError().text()));
    return {};
  }
  QStringList vidPath;
  while (query.next()) {
    vidPath << PathTool::Path3Join(query.value(ENUM_2_STR(PrePathLeft)).toString(),   //
                                   query.value(ENUM_2_STR(PrePathRight)).toString(),  //
                                   query.value(ENUM_2_STR(Name)).toString());
  }
  qDebug("%d records finded", vidPath.size());
  return vidPath;
};

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
    auto record = m_perfDbMdl->record(r);
    const QStringList& vidsList = GetVidsListFromVidsTable(record, qur);
    record.setValue(PERFORMER_DB_HEADER_KEY::Vids, vidsList.join(PerformerJsonFileHelper::PERFS_VIDS_IMGS_SPLIT_CHAR));
    m_perfDbMdl->setRecord(r, record);  // update back
    vidsCnt += vidsList.size();
    if (recordsCnt == 0) {  // update display html content of first selected record
      CHECK_NULLPTR_RETURN_FALSE(_floatingPreview);
      const auto& record = m_perfDbMdl->record(currentIndex().row());
      _floatingPreview->operator()(record, m_imageHostPath, m_performerImageHeight);
    }
    ++recordsCnt;
  }
  qDebug("Selected %d record(s) updated %d vid(s).", recordsCnt, vidsCnt);
  QMessageBox::information(this, QString("Selected %1 record(s) updated.").arg(recordsCnt), QString("%1 vid(s)").arg(vidsCnt));
  return recordsCnt;
}

bool CastDBView::onOpenRecordInFileSystem() const {
  if (!QDir(m_imageHostPath).exists()) {
    qWarning("imgHost [%s] not exists", qPrintable(m_imageHostPath));
    return false;
  }
  if (!selectionModel()->hasSelection()) {
    qDebug("Nothing was selected. Select a row to open pjson folder");
    return false;
  }

  const auto& record = m_perfDbMdl->record(currentIndex().row());
  QString folderPath = QString{"%1/%2/%3"}                                                              //
                           .arg(m_imageHostPath)                                                        //
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
  CastDBView perfManaWid;
  perfManaWid.show();
  a.exec();
  return 0;
}
#endif
