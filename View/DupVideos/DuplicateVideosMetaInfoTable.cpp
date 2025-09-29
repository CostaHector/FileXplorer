#include "DuplicateVideosMetaInfoTable.h"

#include "DuplicateVideosFinderActions.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

#include <QItemSelectionModel>

#include <QFileDialog>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QDesktopServices>
#include <QMimeData>
#include <QUrl>

DuplicateVideosMetaInfoTable::DuplicateVideosMetaInfoTable(QWidget* parent) : CustomTableView{"AiMediaDupTableView", parent}, mDupVidMngr{} {
  m_aiMediaDupMenu = g_dupVidFinderAg().GetMenu(this);
  CHECK_NULLPTR_RETURN_VOID(m_aiMediaDupMenu);
  BindMenu(m_aiMediaDupMenu);

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
  auto tbl2Cnt = mDupVidMngr.TableName2Cnt();
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
  auto& dupVidAgInst = g_dupVidFinderAg();

  connect(dupVidAgInst.ANALYSE_THESE_TABLES, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onAnalyzeTheseSelectedTables);
  connect(dupVidAgInst.CLEAR_ANALYSIS_LIST, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onClearAnalyzeList);

  connect(dupVidAgInst.OPEN_DATABASE, &QAction::triggered, &mDupVidMngr, &DupVidsManager::onShowInFileSystemView);

  connect(dupVidAgInst.SCAN_A_PATH, &QAction::triggered, this, [this]() { onScanAPath(); });

  connect(dupVidAgInst.DROP_THESE_TABLES, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onDropSelectedTables);

  connect(dupVidAgInst.AUDIT_THESE_TABLES, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onAuditSelectedTables);

  connect(dupVidAgInst.FORCE_RELOAD_TABLES, &QAction::triggered, this, &DuplicateVideosMetaInfoTable::onForceReloadTables);

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
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
  LOG_OE_NP(openRet, "Open Directory", pth);
  return openRet;
}
