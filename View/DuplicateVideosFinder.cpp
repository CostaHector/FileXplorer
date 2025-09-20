#include "DuplicateVideosFinder.h"
#include "DuplicateVideosFinderActions.h"
#include "FileBasicOperationsActions.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "UndoRedo.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"

#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>

#include <QSortFilterProxyModel>

#include <QApplication>
#include <QClipboard>

LeftDuplicateList::LeftDuplicateList(QWidget* parent) : CustomTableView{"LeftDuplicateList", parent} {
  m_dupListModel = new VidInfoModel{this};

  m_sortProxy = new QSortFilterProxyModel;
  m_sortProxy->setSourceModel(m_dupListModel);
  setModel(m_sortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

void LeftDuplicateList::subscribe() {}

// -------------------------------------------------------------------------------------------------

RightDuplicateDetails::RightDuplicateDetails(QWidget* parent) : CustomTableView{"RightDuplicateDetails", parent} {
  m_detailsModel = new DuplicateDetailsModel{this};

  m_sortProxy = new QSortFilterProxyModel;
  m_sortProxy->setSourceModel(m_detailsModel);
  setModel(m_sortProxy);

  subscribe();

  InitTableView();
  setSortingEnabled(true);
}

void RightDuplicateDetails::on_effectiveNameCopiedForEverything(const QModelIndex& ind) const {
  const auto& srcIndex = m_sortProxy->mapToSource(ind);
  const QString& name = m_detailsModel->fileNameEverything(srcIndex);
  auto* cb = QApplication::clipboard();
  if (cb == nullptr) {
    LOG_W("cb is nullptr, Copy[%s] failed", qPrintable(name));
    return;
  }
  cb->setText(name, QClipboard::Mode::Clipboard);
  return;
}

void RightDuplicateDetails::on_cellDoubleClicked(const QModelIndex& ind) const {
  const auto& srcIndex = m_sortProxy->mapToSource(ind);
  const QString filepath = m_detailsModel->filePath(srcIndex);
  QDesktopServices::openUrl(QUrl::fromLocalFile(filepath));
}

void RightDuplicateDetails::setSharedMember(CLASSIFIED_SORT_LIST_2D* pClassifiedSort, DIFFER_BY_TYPE* pCurDifferType) {
  m_detailsModel->SyncFrom(pClassifiedSort, pCurDifferType);
}

void RightDuplicateDetails::onRecycleSelection() {
  const int SELECTED_CNT = selectionModel()->selectedRows().size();
  if (SELECTED_CNT < 1) {
    LOG_D("nothing selected to recycle");
    return;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(SELECTED_CNT);
  for (const auto& proInd : selectionModel()->selectedRows()) {
    const auto& srcInd = m_sortProxy->mapToSource(proInd);
    recycleCmds.append(ACMD::GetInstMOVETOTRASH("", m_detailsModel->filePath(srcInd)));
  }
  auto isRenameAllSucceed = UndoRedo::GetInst().Do(recycleCmds);
  LOG_D("Recycle %d item(s) %d.", SELECTED_CNT, isRenameAllSucceed);
}

void RightDuplicateDetails::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &RightDuplicateDetails::on_cellDoubleClicked);
  connect(g_dupVidFinderAg().OPEN_DATABASE, &QAction::triggered, []() {
    const QString vidDupDbPath = DupVidsManager::GetAiDupVidDbPath();
    bool openResult = FileTool::OpenLocalFileUsingDesktopService(vidDupDbPath);
    if (!openResult) {
      LOG_ERR_NP("Open database failed", vidDupDbPath);
    } else {
      LOG_OK_NP("Open database succeed", vidDupDbPath);
    }
  });
  connect(g_dupVidFinderAg().RECYCLE_ONE_FILE, &QAction::triggered, this, &RightDuplicateDetails::onRecycleSelection);
}

// -------------------------------------------------------------------------------------------------
const QString DuplicateVideosFinder::DUPLICATE_FINDER_TITLE_TEMPLATE  //
    {"Duplicate Videos Finder: Analyzing %1 table(s) | Differ by %2 | %3 batch(es) | total %4 video(s)"};

DuplicateVideosFinder::DuplicateVideosFinder(QWidget* parent) : QMainWindow{parent} {
  m_tb = g_dupVidFinderAg().GetAiMediaToolBar(this);
  CHECK_NULLPTR_RETURN_VOID(m_tb);
  m_tb->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_tb);

  m_aiTables = new (std::nothrow) AiMediaDupTableView{this};
  CHECK_NULLPTR_RETURN_VOID(m_aiTables);

  m_dupList = new (std::nothrow) LeftDuplicateList{this};
  CHECK_NULLPTR_RETURN_VOID(m_dupList);
  m_details = new (std::nothrow) RightDuplicateDetails{this};
  CHECK_NULLPTR_RETURN_VOID(m_details);
  m_details->setSharedMember(&m_dupList->m_dupListModel->m_classifiedSort,  //
                             &m_dupList->m_dupListModel->m_currentDiffer);  //

  m_detail_left_right = new (std::nothrow) QSplitter{Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_detail_left_right);
  m_detail_left_right->addWidget(m_dupList);
  m_detail_left_right->addWidget(m_details);

  m_tbl_detail_ver = new (std::nothrow) QSplitter{Qt::Orientation::Vertical, this};
  m_tbl_detail_ver->addWidget(m_aiTables);
  m_tbl_detail_ver->addWidget(m_detail_left_right);

  setCentralWidget(m_tbl_detail_ver);

  subscribe();

  updateWindowsSize();
  UpdateWindowsTitle(0);
  setWindowIcon(QIcon(":img/DUPLICATE_VIDEOS_FINDER"));

  UpdateAiMediaTableNames();
}

void DuplicateVideosFinder::UpdateWindowsTitle(int tablesInAnalyseCnt) {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    return;
  }
  static int lastTimeTablesInAnalyseCnt = 0;
  if (tablesInAnalyseCnt >= 0) { // -1 input will keep last time tables count value
    lastTimeTablesInAnalyseCnt = tablesInAnalyseCnt;
  }
  auto* pDupListModel = m_dupList->m_dupListModel;
  setWindowTitle(DUPLICATE_FINDER_TITLE_TEMPLATE
                     .arg(lastTimeTablesInAnalyseCnt)           //
                     .arg(pDupListModel->getDifferTypeStr())    //
                     .arg(pDupListModel->rowCount())            //
                     .arg(pDupListModel->getReadVidsCount()));  //
}

void DuplicateVideosFinder::updateWindowsSize() {
  if (Configuration().contains("DuplicateVideosFinderGeometry")) {
    restoreGeometry(Configuration().value("DuplicateVideosFinderGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_tbl_detail_ver->restoreState(Configuration().value("DuplicateVideosFinderSplitterState", QByteArray()).toByteArray());
}

void DuplicateVideosFinder::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void DuplicateVideosFinder::closeEvent(QCloseEvent* event) {
  Configuration().setValue("DuplicateVideosFinderGeometry", saveGeometry());
  Configuration().setValue("DuplicateVideosFinderSplitterState", m_tbl_detail_ver->saveState());
  QMainWindow::closeEvent(event);
}

void DuplicateVideosFinder::keyPressEvent(QKeyEvent* e) {
  if (e->modifiers() == Qt::KeyboardModifier::ControlModifier && e->key() == Qt::Key_Insert) {
    const QModelIndex& playInd = m_details->currentIndex();
    if (!playInd.isValid()) {
      LOG_W("invalid index ignore copy everything name");
      return;
    }
    m_details->on_effectiveNameCopiedForEverything(playInd);
    return;
  }
  if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
    switch (e->modifiers()) {
      case Qt::KeyboardModifier::NoModifier: {
        const QModelIndex& playInd = m_details->currentIndex();
        if (!playInd.isValid()) {
          LOG_W("playInd is invalid");
          return;
        }
        emit m_details->doubleClicked(m_details->currentIndex());
        return;
      }
      default:
        break;
    }
  }
  return QMainWindow::keyPressEvent(e);
}

bool DuplicateVideosFinder::TablesGroupChangedTo(const QStringList& tbls) {
  if (m_dupList == nullptr || m_dupList->m_dupListModel == nullptr) {
    LOG_W("Duplicate list or its model is nullptr");
    return false;
  }
  m_dupList->m_dupListModel->ChangeTableGroups(tbls);
  UpdateWindowsTitle(tbls.size());
  return true;
}

void DuplicateVideosFinder::subscribe() {
  connect(g_dupVidFinderAg().ANALYSE_THESE_TABLES, &QAction::triggered, this, &DuplicateVideosFinder::onAnalyseAiMediaTableChanged);
  connect(g_dupVidFinderAg().CANCEL_ANALYSE, &QAction::triggered, this, &DuplicateVideosFinder::onCancelAnalyse);
  connect(g_dupVidFinderAg().DIFFER_BY, &QActionGroup::triggered, this, &DuplicateVideosFinder::onDifferTypeChanged);
  connect(g_dupVidFinderAg().durationDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeDurationDeviation);
  connect(g_dupVidFinderAg().sizeDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeSizeDeviation);
  connect(m_dupList->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &DuplicateVideosFinder::on_selectionChanged);
}

void DuplicateVideosFinder::on_selectionChanged(const QModelIndex& current, const QModelIndex& /*previous*/) {
  const auto& proxyIndex = current;
  const auto& srcIndex = m_dupList->m_sortProxy->mapToSource(proxyIndex);

  if (!srcIndex.isValid()) {
    return;
  }
  m_details->clearSelection();
  m_details->m_detailsModel->onChangeDetailIndex(srcIndex.row());
}

void DuplicateVideosFinder::UpdateAiMediaTableNames() {
  if (m_aiTables == nullptr) {
    LOG_W("m_aiTables is nullptr");
    return;
  }
  m_aiTables->LoadAiMediaTableNames();
}

void DuplicateVideosFinder::onAnalyseAiMediaTableChanged() {
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  const QStringList& tbls = m_aiTables->GetSelectedAiTables();
  TablesGroupChangedTo(tbls);
  UpdateWindowsTitle(tbls.size());
}

void DuplicateVideosFinder::onCancelAnalyse() {
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  TablesGroupChangedTo({});
  UpdateWindowsTitle(0);
}

void DuplicateVideosFinder::onDifferTypeChanged(QAction* newDifferAct) {
  if (m_dupList == nullptr || m_dupList->m_dupListModel == nullptr) {
    LOG_W("Duplicate list or its model is nullptr when set differ by duration");
    return;
  }
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  if (newDifferAct == g_dupVidFinderAg().DIFFER_BY_SIZE) {
    m_dupList->m_dupListModel->setDifferType(DIFFER_BY_TYPE::SIZE);
  } else if (newDifferAct == g_dupVidFinderAg().DIFFER_BY_DURATION) {
    m_dupList->m_dupListModel->setDifferType(DIFFER_BY_TYPE::DURATION);
  }
  UpdateWindowsTitle(-1);  // unchange
}

void DuplicateVideosFinder::onChangeSizeDeviation() {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    LOG_W("Duplicate list or its model is nullptr when onChangeSizeDeviation");
    return;
  }
  const QString& szDevStr = g_dupVidFinderAg().sizeDevLE->text();
  bool isIntValid = false;
  int dev = szDevStr.toInt(&isIntValid);
  if (not isIntValid or dev <= 0) {
    LOG_W("size str[%s] is not valid", qPrintable(szDevStr));
    return;
  }
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  m_dupList->m_dupListModel->setDeviationSize(dev);
  UpdateWindowsTitle(-1);  // unchange
}
void DuplicateVideosFinder::onChangeDurationDeviation() {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    LOG_W("Duplicate list or its model is nullptr when onChangeDurationDeviation");
    return;
  }
  const QString& durDevStr = g_dupVidFinderAg().durationDevLE->text();
  bool isIntValid = false;
  int dev = durDevStr.toInt(&isIntValid);
  if (not isIntValid or dev <= 0) {
    LOG_W("duration str[%s] is not valid", qPrintable(durDevStr));
    return;
  }
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  m_dupList->m_dupListModel->setDeviationDuration(dev);
  UpdateWindowsTitle(-1);  // unchange
}
