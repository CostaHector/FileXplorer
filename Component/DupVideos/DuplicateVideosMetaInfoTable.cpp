#include "DuplicateVideosMetaInfoTable.h"

#include "NotificatorMacro.h"
#include "PublicCheckMacro.h"
#include "FileTool.h"
#include "Configuration.h"

#include <QItemSelectionModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QDragEnterEvent>
#include <QLineEdit>
#include <QMimeData>
#include <QUrl>

DuplicateVideosMetaInfoTable::DuplicateVideosMetaInfoTable(QWidget* parent) : CustomTableView{"AiMediaDupTableView", parent} {
  m_ClearAnalysisList = new (std::nothrow) QAction{"Clear Analyse", this};
  CHECK_NULLPTR_RETURN_VOID(m_ClearAnalysisList);
  m_ClearAnalysisList->setToolTip("Clear current analyse list");

  m_AnalyseTheseTables = new (std::nothrow) QAction{QIcon(":img/ANALYSE_AI_MEDIA_TABLES"), "Analyse", this};
  CHECK_NULLPTR_RETURN_VOID(m_AnalyseTheseTables);
  m_AnalyseTheseTables->setToolTip("Append records from selected tables to the current analysis list");

  m_ScanAPath = new (std::nothrow) QAction{QIcon(":img/LOAD_A_PATH"), "Scan a path", this};
  CHECK_NULLPTR_RETURN_VOID(m_ScanAPath);
  m_ScanAPath->setToolTip("Scan videos from a directory and populate the table with their information");

  m_AuditTheseTables = new (std::nothrow) QAction{QIcon{":img/AUDIT_AI_MEDIA_DUP"}, "Audit", this};
  CHECK_NULLPTR_RETURN_VOID(m_AuditTheseTables);
  m_AuditTheseTables->setToolTip("Validate table records and update if corresponding files are missing");

  m_DropTheseTables = new (std::nothrow) QAction{QIcon{":img/DROP_TABLE"}, "Drop Tables", this};
  CHECK_NULLPTR_RETURN_VOID(m_DropTheseTables);
  m_DropTheseTables->setToolTip("Delete selected table(s)");

  m_ForceReloadTables = new (std::nothrow) QAction{QIcon{":img/REFRESH_THIS_PATH"}, "Force reload", this};
  CHECK_NULLPTR_RETURN_VOID(m_ForceReloadTables);
  m_ForceReloadTables->setToolTip("Delete selected table(s) and reload them from source");

  m_OpenDatabase = new (std::nothrow) QAction{QIcon(":img/SQLITE_APP"), "&Open Database", this};
  CHECK_NULLPTR_RETURN_VOID(m_OpenDatabase);
  m_OpenDatabase->setToolTip(QString("<b>%1 (%2)</b><br/>Open SQLite database files (*.db)<br/>Requires DB Browser for SQLite to be installed and set as the default application")
                                .arg(m_OpenDatabase->text(), m_OpenDatabase->shortcut().toString()));

  QList<QAction*> exclusiveActs{
      m_AnalyseTheseTables, m_ScanAPath,       m_ForceReloadTables,
      m_AuditTheseTables,   m_DropTheseTables, m_ClearAnalysisList,
  };
  PushFrontExclusiveActions(exclusiveActs);

  m_aiMediaTblModel = new (std::nothrow) DuplicateVideosMetaInfoModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_aiMediaTblModel);
  m_sortProxy = new (std::nothrow) QSortFilterProxyModel;
  CHECK_NULLPTR_RETURN_VOID(m_sortProxy);
  m_sortProxy->setSourceModel(m_aiMediaTblModel);
  setModel(m_sortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
  setDragDropMode(QAbstractItemView::DropOnly);
}

void DuplicateVideosMetaInfoTable::LoadAiMediaTableNames() {
  CHECK_NULLPTR_RETURN_VOID(m_aiMediaTblModel);
  DupVidTableName2RecordCountList tbl2Cnt = mDupVidMngr.TableName2Cnt();
  m_aiMediaTblModel->UpdateDupVideoTableMetaInfoList(tbl2Cnt);
}

QStringList DuplicateVideosMetaInfoTable::GetSelectedAiTables() const {
  const QModelIndexList& indx = Proxy2Source(selectionModel()->selectedRows());
  return m_aiMediaTblModel->fileNames(indx);
}

QString DuplicateVideosMetaInfoTable::GetCurrentDupVideoMetaInfo() const {
  const int actualTablesCount = m_aiMediaTblModel->rowCount();
  return QString("Analysed %1 videos in %2 tables")  //
      .arg(mVideosListNeedAnalyse.size())            //
      .arg(actualTablesCount)                        //
      ;                                              //
}

int DuplicateVideosMetaInfoTable::onAnalyzeTheseSelectedTables() {
  const QStringList& tablesNeedAnalyze = GetSelectedAiTables();
  if (tablesNeedAnalyze.isEmpty()) {
    LOG_INFO_NP("Skip Analyze", "nothing selected");
    return 0;
  }
  return startAnalyzeNewTables(tablesNeedAnalyze);
}

int DuplicateVideosMetaInfoTable::onClearAnalyzeList() {
  return startAnalyzeNewTables({});
}

int DuplicateVideosMetaInfoTable::startAnalyzeNewTables(const QStringList& tablesNeedAnalyze) {
  mDupVidMngr.ReadSpecifiedTables2List(tablesNeedAnalyze, mVideosListNeedAnalyse);
  emit analyzeTablesFinished(mVideosListNeedAnalyse);

  const QString dupVideosMetaInfoTile = GetCurrentDupVideoMetaInfo();
  emit windowTitleChanged(dupVideosMetaInfoTile);

  return mVideosListNeedAnalyse.size();
}

bool DuplicateVideosMetaInfoTable::onScanAPath(const QString& specifiedPath) {
  QString loadFromPath;
  if (specifiedPath.isEmpty()) {  // ask user to select
    const QString& defaultOpenDir = Configuration().value("DUPLICATE_VIDEOS_SELECT_FROM", ".").toString();
    loadFromPath = QFileDialog::getExistingDirectory(this, "Learn From", defaultOpenDir);
  } else {  // from input directly
    loadFromPath = specifiedPath;
  }
  QFileInfo loadFromFi(loadFromPath);
  const QString& absPath = loadFromFi.absoluteFilePath();
  if (!loadFromFi.isDir()) {
    LOG_WARN_P("[Abort] ScanAPath", "Not a folder: %s", qPrintable(absPath));
    return false;
  }
  Configuration().setValue("DUPLICATE_VIDEOS_SELECT_FROM", absPath);
  const bool scanRet = mDupVidMngr.ScanALocation(absPath);
  mDupVidMngr.FillHashFieldIfSizeConflict(absPath);
  LoadAiMediaTableNames();
  LOG_OE_P(scanRet, "Scan path", "path: %s", qPrintable(absPath));
  return scanRet;
}

void DuplicateVideosMetaInfoTable::dragEnterEvent(QDragEnterEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    event->ignore();
    return;
  }
  event->accept();
}
void DuplicateVideosMetaInfoTable::dragMoveEvent(QDragMoveEvent* event) {
  if (!event->mimeData()->hasUrls()) {
    event->ignore();
    return;
  }
  event->accept();
}
void DuplicateVideosMetaInfoTable::dropEvent(QDropEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  auto* pMimedata = event->mimeData();
  if (pMimedata == nullptr) {
    event->ignore();
    return;
  }
  if (!pMimedata->hasUrls()) {
    event->ignore();
    return;
  }
  const auto& urlsLst = pMimedata->urls();
  foreach (const QUrl& url, urlsLst) {
    const QString specifiedPath = url.toLocalFile();
    onScanAPath(specifiedPath);
  }
  event->accept();
}

QModelIndexList DuplicateVideosMetaInfoTable::Proxy2Source(const QModelIndexList& proInds) const {
  QModelIndexList ans;
  ans.reserve(proInds.size());
  for (const QModelIndex& ind : proInds) {
    ans.append(m_sortProxy->mapToSource(ind));
  }
  return ans;
}

void DuplicateVideosMetaInfoTable::subscribe() {  
  connect(m_AnalyseTheseTables, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onAnalyzeTheseSelectedTables);
  connect(m_ClearAnalysisList, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onClearAnalyzeList);
  connect(m_ScanAPath, &QAction::triggered, this, [this]() { onScanAPath(); });
  connect(m_DropTheseTables, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onDropSelectedTables);
  connect(m_AuditTheseTables, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onAuditSelectedTables);
  connect(m_ForceReloadTables, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onForceReloadTables);
  connect(m_OpenDatabase, &QAction::triggered, &mDupVidMngr, &DupVidsManager::onShowInFileSystemView);
  connect(this, &QTableView::doubleClicked, this, &DuplicateVideosMetaInfoTable::onOpenTableAssociatedPath);
}

bool DuplicateVideosMetaInfoTable::onDropSelectedTables() {
  const QStringList& tables = GetSelectedAiTables();
  if (tables.isEmpty()) {
    LOG_INFO_NP("Skip Drop", "nothing selected");
    return true;
  }
  QMessageBox::StandardButton ret = QMessageBox::StandardButton::Yes;
#ifndef RUNNING_UNIT_TESTS
  ret = QMessageBox::warning(this, "Drop selected tables?", "Cannot recover", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No,
                             QMessageBox::StandardButton::No);
#endif
  if (ret != QMessageBox::StandardButton::Yes) {
    LOG_OK_NP("[Skip]User has cancel drop table", "return");
    return true;
  }
  const int tblCnt = mDupVidMngr.DropTables(tables);
  LoadAiMediaTableNames();
  LOG_OK_P("Drop Tables succeed", "%d table(s)", tblCnt);
  return true;
}

bool DuplicateVideosMetaInfoTable::onAuditSelectedTables() {
  const QStringList& tables = GetSelectedAiTables();
  if (tables.isEmpty()) {
    LOG_INFO_NP("Skip Audit", "nothing selected");
    return true;
  }
  const int tblCnt = mDupVidMngr.AuditTables(tables);
  LoadAiMediaTableNames();
  LOG_OK_P("Audit Tables succeed", "%d records(s)", tblCnt);
  return true;
}

bool DuplicateVideosMetaInfoTable::onForceReloadTables() {
  const QStringList& tables = GetSelectedAiTables();
  if (tables.isEmpty()) {
    LOG_INFO_NP("Skip Drop and rebuild", "nothing selected");
    return true;
  }
  QMessageBox::StandardButton ret = QMessageBox::StandardButton::Yes;
#ifndef RUNNING_UNIT_TESTS
  ret = QMessageBox::warning(this, "Drop & Rebuild selected tables?", "If disk is offline, skip it",
                             QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
#endif
  if (ret != QMessageBox::StandardButton::Yes) {
    LOG_OK_NP("Skip", "User has cancel drop&rebuild table");
    return true;
  }
  mDupVidMngr.RebuildTables(tables);
  LoadAiMediaTableNames();
  return true;
}

bool DuplicateVideosMetaInfoTable::onOpenTableAssociatedPath(const QModelIndex& ind) {
  if (!ind.isValid()) {
    return false;
  }
  const QModelIndex& srcInd = m_sortProxy->mapToSource(ind);
  if (!srcInd.isValid()) {
    return false;
  }
  const QString& tableName = m_aiMediaTblModel->fileName(srcInd);
  const QString& pth = TableName2Path(tableName);
  if (!QFileInfo(pth).isDir()) {
    LOG_ERR_P("[Path not exist]", "[%s]\nderived from:[%s]", qPrintable(pth), qPrintable(tableName));
    return false;
  }
  const bool openRet = FileTool::OpenLocalFileUsingDesktopService(pth);
  LOG_OE_NP(openRet, "Open Table Associated path", pth);
  return openRet;
}
