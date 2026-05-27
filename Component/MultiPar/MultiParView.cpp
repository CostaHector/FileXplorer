#include "MultiParView.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include "FileTool.h"
#include <QMessageBox>

MultiParView::MultiParView(ParVerifyInfomationList &&resultList, const QString &instName, QWidget *parent)
  : CustomTableView{instName, parent} {
  setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

  mMultiParModel = new (std::nothrow) MultiParModel{std::move(resultList), this};
  CHECK_NULLPTR_RETURN_VOID(mMultiParModel);

  mSortFilterProxy = new (std::nothrow) QSortFilterProxyModel{this};
  CHECK_NULLPTR_RETURN_VOID(mSortFilterProxy);
  mSortFilterProxy->setFilterCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  mSortFilterProxy->setSortCaseSensitivity(Qt::CaseSensitivity::CaseInsensitive);
  mSortFilterProxy->setSourceModel(mMultiParModel);

  setModel(mSortFilterProxy);
  registerProxyModel(mSortFilterProxy);
  InitTableView();

  {
    m_repairBrokenFile = new QAction{QIcon{":img/REPAIR"}, tr("Repair Broken File"), this};
    m_syncBuiltInSrcFileListInPar2 = new QAction{QIcon{":img/RENAME"}, tr("Sync PAR2 Built-in Source File List"), this};
    m_processOldNewPar2NameThenReverify = new QAction{QIcon{":img/REFRESH_THIS_PATH"}, tr("Process Old/New PAR2 Names and Re-verify"), this};
    m_repairMissnamedFile = new QAction{QIcon{":img/REVERT"}, tr("Recover Source File Name"), this};
    m_openCurrentFile = new QAction{QIcon{":img/SYSTEM_APPLICATION"}, tr("Open in System Application"), this};

    QList<QAction *> exclusiveActs{
        m_repairBrokenFile,                  //
        m_syncBuiltInSrcFileListInPar2,      //
        m_processOldNewPar2NameThenReverify, //
        m_repairMissnamedFile,               //
        m_openCurrentFile,                   //
    };
    PushFrontExclusiveActions(exclusiveActs);
  }

  subscribe();
}

void MultiParView::setFilter(const QString &filter) {
  mSortFilterProxy->setFilterFixedString(filter);
}

void MultiParView::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &MultiParView::onOpenFileInSystemApplication);
  connect(m_openCurrentFile, &QAction::triggered, this, &MultiParView::onOpenFileInSystemApplication);
  connect(selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MultiParView::onSelectionChange);

  connect(m_repairBrokenFile, &QAction::triggered, this, &MultiParView::onRepairBrokenFile);
  connect(m_syncBuiltInSrcFileListInPar2, &QAction::triggered, this, &MultiParView::onSyncPar2FileName);
  connect(m_repairMissnamedFile, &QAction::triggered, this, &MultiParView::onFixMisnamedFile);
  connect(m_processOldNewPar2NameThenReverify, &QAction::triggered, this, &MultiParView::onProcessOldNewPar2NamesThenReverify);
}

int MultiParView::onRepairBrokenFile() {
  QModelIndexList srcIndexes = selectedRowsSource();
  if (srcIndexes.isEmpty()) {
    return 0;
  }
  const int cnt = mMultiParModel->repairBrokenFile(srcIndexes, ParVerifyInfomation::Par2StatusE::READY_TO_REPAIR);
  return cnt;
}

int MultiParView::onSyncPar2FileName() {
  QModelIndexList srcIndexes = selectedRowsSource();
  if (srcIndexes.isEmpty()) {
    return 0;
  }
  const int cnt = mMultiParModel->syncBuiltInSrcFileListInPar2(srcIndexes);
  return cnt;
}

int MultiParView::onFixMisnamedFile() {
  QModelIndexList srcIndexes = selectedRowsSource();
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip", "No selection");
    return 0;
  }

  if (QMessageBox::StandardButton::Yes
      != QMessageBox::warning(this,
                              tr("Confirm Recovery Source File Name?"),
                              tr("Recover source file names using the built-in source name list from PAR2.\n"
                                 "You may need 'Sync PAR2 Built-in Source File List' instead."))) {
    LOG_INFO_NP("Skip", "User cancelled name recovery");
    return 0;
  }
  const int cnt = mMultiParModel->repairBrokenFile(srcIndexes, ParVerifyInfomation::Par2StatusE::READY_TO_RENAME);
  return cnt;
}

int MultiParView::onProcessOldNewPar2NamesThenReverify() {
  QModelIndexList srcIndexes = selectedRowsSource();
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip", "No selection");
    return 0;
  }
  QModelIndexList needReverifyIndexes = mMultiParModel->ProcessOldNewPar2Names(srcIndexes);
  const int cnt = mMultiParModel->ReverifyPar2File(needReverifyIndexes);
  return cnt;
}

void MultiParView::onSelectionChange(const QModelIndex &proIndex) {
  if (!proIndex.isValid()) {
    LOG_INFO_NP("Skip", "No selection");
    return;
  }
  QModelIndex srcIndex = mSortFilterProxy->mapToSource(proIndex);
  const QByteArray *pCliOutput = mMultiParModel->GetCliOutput(srcIndex);
  if (pCliOutput == nullptr) {
    return;
  }
  emit showCliOutputReq(*pCliOutput);
}

bool MultiParView::onOpenFileInSystemApplication() const {
  const QModelIndex &proIndex{currentIndex()};
  if (!proIndex.isValid()) {
    return false;
  }
  QModelIndex srcIndex = mSortFilterProxy->mapToSource(proIndex);
  QString filePath;
  if (MultiParKey::isPar2Column(srcIndex.column())) {
    filePath = mMultiParModel->GetPar2FileAbsPath(srcIndex);
  } else {
    filePath = mMultiParModel->GetFrontSourceFile(srcIndex);
  }
  return FileTool::OpenLocalFileUsingDesktopService(filePath);
}
