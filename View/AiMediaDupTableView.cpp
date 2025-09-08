#include "AiMediaDupTableView.h"

#include "DuplicateVideosFinderActions.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include <QItemSelectionModel>

#include <QFileDialog>
#include <QMessageBox>

#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QDesktopServices>

AiMediaDupTableView::AiMediaDupTableView(QWidget* parent) : CustomTableView{"LeftDuplicateList", parent} {
  m_aiMediaDupMenu = g_dupVidFinderAg().GetMenu(this);
  BindMenu(m_aiMediaDupMenu);

  m_aiMediaTblModel = new AiMediaTablesModel{this};

  m_sortProxy = new QSortFilterProxyModel;
  m_sortProxy->setSourceModel(m_aiMediaTblModel);
  setModel(m_sortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

void AiMediaDupTableView::LoadAiMediaTableNames() {
  if (m_aiMediaTblModel == nullptr) {
    LOG_W("m_aiMediaTblModel is nullptr");
    return;
  }
  auto& aimd = AIMediaDuplicate::GetInst();
  const auto& tbl2Cnt = aimd.TableName2Cnt();
  m_aiMediaTblModel->UpdateData(tbl2Cnt);
}

QStringList AiMediaDupTableView::GetSelectedAiTables() const {
  const QModelIndexList& indx = Proxy2Source(selectionModel()->selectedRows());
  return m_aiMediaTblModel->fileNames(indx);
}

void AiMediaDupTableView::onScanAPath() {
  const QString& defaultOpenDir = Configuration().value("DUPLICATE_VIDEOS_SELECT_FROM", ".").toString();
  const QString& loadFromPath = QFileDialog::getExistingDirectory(this, "Learn From", defaultOpenDir);

  QFileInfo loadFromFi(loadFromPath);
  const QString& absPath = loadFromFi.absoluteFilePath();
  if (!loadFromFi.isDir()) {
    LOG_WARN_P("[Abort] ScanAPath", "Not a folder:%s", qPrintable(absPath));
    return;
  }
  Configuration().setValue("DUPLICATE_VIDEOS_SELECT_FROM", absPath);
  auto& aimd = AIMediaDuplicate::GetInst();
  const bool scanRet = aimd.ScanALocation(absPath, false, true);
  aimd.FillHashFieldIfSizeConflict(absPath);
  LoadAiMediaTableNames();
  LOG_GOOD_P("Scan path result", "scanRet:%d, Path: %s", scanRet, qPrintable(absPath));
}

QModelIndexList AiMediaDupTableView::Proxy2Source(const QModelIndexList& proInds) const {
  QModelIndexList ans;
  ans.reserve(proInds.size());
  for (const QModelIndex& ind : proInds) {
    ans.append(m_sortProxy->mapToSource(ind));
  }
  return ans;
}

void AiMediaDupTableView::subscribe() {
  if (g_dupVidFinderAg().tblKWFilter != nullptr) {
    connect(g_dupVidFinderAg().tblKWFilter, &QLineEdit::textChanged, this, [this](const QString& kw) -> void { m_sortProxy->setFilterWildcard(kw); });
  } else {
    LOG_W("tblKWFilter is nullptr");
  }

  connect(g_dupVidFinderAg().SCAN_A_PATH, &QAction::triggered, this, &AiMediaDupTableView::onScanAPath);

  connect(g_dupVidFinderAg().DROP_TABLE, &QAction::triggered, this, [this]() {
    auto& aimd = AIMediaDuplicate::GetInst();
    const auto ret = QMessageBox::warning(this, "Drop selected tables?", "Cannot recover",
                                          QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      LOG_GOOD_NP("[Skip]User has cancel drop table", "return");
      return;
    }
    const int tblCnt = aimd.DropTables(GetSelectedAiTables(), false);
    LoadAiMediaTableNames();
    LOG_GOOD_P("Drop Tables succeed", "%d table(s)", tblCnt);
  });

  connect(g_dupVidFinderAg().AUDIT_AI_MEDIA_TABLE, &QAction::triggered, this, [this]() {
    auto& aimd = AIMediaDuplicate::GetInst();
    const int tblCnt = aimd.AuditTables(GetSelectedAiTables(), false);
    LoadAiMediaTableNames();
    LOG_GOOD_P("Audit Tables succeed", "%d records(s)", tblCnt);
  });

  connect(g_dupVidFinderAg().DROP_THEN_REBUILD_THIS_TABLE, &QAction::triggered, this, [this]() {
    auto& aimd = AIMediaDuplicate::GetInst();
    const auto ret =
        QMessageBox::warning(this, "Drop & Rebuild selected tables?", "If disk is offline, only table fields can rebuild, records cannot",
                             QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      LOG_D("User has cancel drop&rebuild table");
      return;
    }
    const int tblCnt = aimd.RebuildTables(GetSelectedAiTables(), false);
    LoadAiMediaTableNames();
    LOG_GOOD_P("Drop & Rebuild succeed", "%d table(s)", tblCnt);
  });

  connect(this, &QTableView::doubleClicked, this, [this](const QModelIndex& ind) {
    const QModelIndex& srcInd = m_sortProxy->mapToSource(ind);
    const QString& tableName = m_aiMediaTblModel->fileName(srcInd);
    const QString& pth = TableName2Path(tableName);
    if (!QFileInfo(pth).isDir()) {
      LOG_BAD_P("[Skip open]", "Path[%s] or table[%s] not exist", qPrintable(pth), qPrintable(tableName));
      return;
    }
    const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    if (!openRet) {
      LOG_W("Open path[%s] failed", qPrintable(pth));
    }
  });
}
