#include "AiMediaDupTableView.h"

#include "Actions/DuplicateVideosFinderActions.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"
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
    qWarning("m_aiMediaTblModel is nullptr");
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
  const QString& defaultOpenDir = PreferenceSettings().value("DUPLICATE_VIDEOS_SELECT_FROM", ".").toString();
  const QString& loadFromPath = QFileDialog::getExistingDirectory(this, "Learn From", defaultOpenDir);

  QFileInfo loadFromFi(loadFromPath);
  const QString& absPath = loadFromFi.absoluteFilePath();
  if (not loadFromFi.isDir()) {
    QMessageBox::warning(this, "Failed when select a folder", QString("Not a folder:\n%1").arg(absPath));
    qWarning("Failed when select a folder. Not a folder:\n%s", qPrintable(absPath));
    return;
  }
  PreferenceSettings().setValue("DUPLICATE_VIDEOS_SELECT_FROM", absPath);
  auto& aimd = AIMediaDuplicate::GetInst();
  const bool scanRet = aimd.ScanALocation(absPath, false, true);
  aimd.FillHashFieldIfSizeConflict(absPath);
  LoadAiMediaTableNames();
  Notificator::goodNews("Scan path result", QString("bool:%1, Path: %2").arg(scanRet).arg(absPath));
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
    qWarning("tblKWFilter is nullptr");
  }

  connect(g_dupVidFinderAg().SCAN_A_PATH, &QAction::triggered, this, &AiMediaDupTableView::onScanAPath);

  connect(g_dupVidFinderAg().DROP_TABLE, &QAction::triggered, this, [this]() {
    auto& aimd = AIMediaDuplicate::GetInst();
    const auto ret = QMessageBox::warning(this, "Drop selected tables?", "Cannot recover",
                                          QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      Notificator::goodNews("User has cancel drop table", "ok");
      return;
    }
    const int tblCnt = aimd.DropTables(GetSelectedAiTables(), false);
    LoadAiMediaTableNames();
    Notificator::goodNews("Drop Tables succeed", QString("%1 table(s)").arg(tblCnt));
  });

  connect(g_dupVidFinderAg().AUDIT_AI_MEDIA_TABLE, &QAction::triggered, this, [this]() {
    auto& aimd = AIMediaDuplicate::GetInst();
    const int tblCnt = aimd.AuditTables(GetSelectedAiTables(), false);
    LoadAiMediaTableNames();
    Notificator::goodNews("Audit Tables succeed", QString("%1 records(s)").arg(tblCnt));
  });

  connect(g_dupVidFinderAg().DROP_THEN_REBUILD_THIS_TABLE, &QAction::triggered, this, [this]() {
    auto& aimd = AIMediaDuplicate::GetInst();
    const auto ret =
        QMessageBox::warning(this, "Drop & Rebuild selected tables?", "If disk is offline, only table fields can rebuild, records cannot",
                             QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      qDebug("User has cancel drop&rebuild table");
      return;
    }
    const int tblCnt = aimd.RebuildTables(GetSelectedAiTables(), false);
    LoadAiMediaTableNames();
    Notificator::goodNews("Drop & Rebuild succeed", QString("%1 table(s)").arg(tblCnt));
  });

  connect(this, &QTableView::doubleClicked, this, [this](const QModelIndex& ind) {
    const QModelIndex& srcInd = m_sortProxy->mapToSource(ind);
    const QString& tableName = m_aiMediaTblModel->fileName(srcInd);
    const QString& pth = TableName2Path(tableName);
    if (!QFileInfo(pth).isDir()) {
      qDebug("Path[%s] or table[%s] not exist", qPrintable(pth), qPrintable(tableName));
      Notificator::badNews("Open failed succeed", QString("Path:%1\ntable:%2").arg(pth).arg(tableName));
      return;
    }
    const bool openRet = QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
    if (!openRet) {
      qWarning("Open path[%s] failed", qPrintable(pth));
    }
  });
}
