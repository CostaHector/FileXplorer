#include "DuplicateVideosFinder.h"
#include "RightVideoDuplicatesModel.h"

#include "DuplicateVideosFinderActions.h"
#include "FileBasicOperationsActions.h"

#include "MemoryKey.h"
#include "StyleSheet.h"
#include "UndoRedo.h"
#include "PublicTool.h"
#include "PublicVariable.h"
#include "NotificatorMacro.h"

using namespace RedundantVideoTool;

DuplicateVideosFinder::DuplicateVideosFinder(QWidget* parent) : QMainWindow{parent} {
  tableNameFilterLE = new (std::nothrow) QLineEdit{"", parent};
  CHECK_NULLPTR_RETURN_VOID(tableNameFilterLE);
  tableNameFilterLE->addAction(QIcon(":img/SEARCH"), QLineEdit::LeadingPosition);

  int szDev = Configuration().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_FILESIZE.v).toInt();
  sizeDevLE = new (std::nothrow) QLineEdit{QString::number(szDev), parent};
  CHECK_NULLPTR_RETURN_VOID(sizeDevLE);

  int durDev = Configuration().value(MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.name, MemoryKey::DUPLICATE_FINDER_DEVIATION_DURATION.v).toInt();
  durationDevLE = new (std::nothrow) QLineEdit{QString::number(durDev), parent};
  CHECK_NULLPTR_RETURN_VOID(durationDevLE);

  m_tb = g_dupVidFinderAg().GetAiMediaToolBar(tableNameFilterLE, sizeDevLE, durationDevLE, this);
  CHECK_NULLPTR_RETURN_VOID(m_tb);
  m_tb->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_tb);

  m_aiTables = new (std::nothrow) DuplicateVideosMetaInfoTable{this};
  CHECK_NULLPTR_RETURN_VOID(m_aiTables);

  m_leftGrpsTbl = new (std::nothrow) LeftVideoGroupsTable{this};
  CHECK_NULLPTR_RETURN_VOID(m_leftGrpsTbl);
  m_rightDetailsTbl = new (std::nothrow) RightVideoDuplicatesDetails{this};
  CHECK_NULLPTR_RETURN_VOID(m_rightDetailsTbl);
  m_rightDetailsTbl->setSharedMember(&m_leftGrpsTbl->m_leftGrpModel->m_groupedVidLstArr,  //
                                     &m_leftGrpsTbl->m_leftGrpModel->m_currentDiffer);    // never forget call this one

  m_detail_left_right = new (std::nothrow) QSplitter{Qt::Orientation::Horizontal, this};
  CHECK_NULLPTR_RETURN_VOID(m_detail_left_right);
  m_detail_left_right->addWidget(m_leftGrpsTbl);
  m_detail_left_right->addWidget(m_rightDetailsTbl);

  m_tbl_detail_ver = new (std::nothrow) QSplitter{Qt::Orientation::Vertical, this};
  m_tbl_detail_ver->addWidget(m_aiTables);
  m_tbl_detail_ver->addWidget(m_detail_left_right);

  setCentralWidget(m_tbl_detail_ver);

  subscribe();

  updateWindowsSize();
  setWindowIcon(QIcon(":img/DUPLICATE_VIDEOS_FINDER"));

  m_aiTables->LoadAiMediaTableNames();

  UpdateWindowsTitleGroupInfo(m_leftGrpsTbl->GetCurrentDupVideoGroupInfo());
  UpdateWindowsTitleMetaInfo(m_aiTables->GetCurrentDupVideoMetaInfo());
  UpdateWindowsTitle();
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
    const QModelIndex& playInd = m_rightDetailsTbl->currentIndex();
    if (!playInd.isValid()) {
      LOG_W("invalid index ignore copy everything name");
      e->ignore();
      return;
    }
    m_rightDetailsTbl->on_effectiveNameCopiedForEverything(playInd);
    e->accept();
    return;
  } else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
    if (m_rightDetailsTbl->hasFocus() && e->modifiers() == Qt::KeyboardModifier::NoModifier) {
      const QModelIndex& playInd = m_rightDetailsTbl->currentIndex();
      if (!playInd.isValid()) {
        LOG_W("playInd is invalid");
        e->ignore();
        return;
      }
      emit m_rightDetailsTbl->doubleClicked(m_rightDetailsTbl->currentIndex());
      e->accept();
      return;
    }
  }
  return QMainWindow::keyPressEvent(e);
}

void DuplicateVideosFinder::subscribe() {
  /* below will change duplicates group result. So clear left selection and right associated current left row needed */
  connect(m_aiTables, &DuplicateVideosMetaInfoTable::analyzeTablesFinished, this, &DuplicateVideosFinder::onAnalyzeDuplicatesInVideosList);
  connect(g_dupVidFinderAg().DIFFER_BY, &QActionGroup::triggered, this, &DuplicateVideosFinder::onDifferTypeChanged);
  connect(durationDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeDurationDeviation);
  connect(sizeDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeSizeDeviation);
  /* above invalidateLeftGrpTblData needed before */

  connect(tableNameFilterLE, &QLineEdit::textChanged,  //
          m_aiTables->m_sortProxy,                     //
          static_cast<void (QSortFilterProxyModel::*)(const QString&)>(&QSortFilterProxyModel::setFilterRegularExpression));
  connect(m_leftGrpsTbl, &LeftVideoGroupsTable::leftSelectionChanged, m_rightDetailsTbl,
          &RightVideoDuplicatesDetails::onLeftVideoGroupsTableSelectionChanged);

  connect(m_aiTables, &DuplicateVideosMetaInfoTable::windowTitleChanged, this, &DuplicateVideosFinder::UpdateWindowsTitleMetaInfo);
  connect(m_leftGrpsTbl, &LeftVideoGroupsTable::windowTitleChanged, this, &DuplicateVideosFinder::UpdateWindowsTitleGroupInfo);
}

void DuplicateVideosFinder::invalidateLeftGrpTblData() {
  m_leftGrpsTbl->clearSelection();
}

void DuplicateVideosFinder::onAnalyzeDuplicatesInVideosList(const DupVidMetaInfoList& needAnalyzeVidLst) {
  invalidateLeftGrpTblData();
  m_leftGrpsTbl->onDuplicateVideosListChanged(needAnalyzeVidLst);
}

void DuplicateVideosFinder::onDifferTypeChanged(QAction* newDifferAct) {
  invalidateLeftGrpTblData();
  if (newDifferAct == g_dupVidFinderAg().DIFFER_BY_SIZE) {
    m_leftGrpsTbl->setDifferType(DIFFER_BY_TYPE::SIZE);
  } else if (newDifferAct == g_dupVidFinderAg().DIFFER_BY_DURATION) {
    m_leftGrpsTbl->setDifferType(DIFFER_BY_TYPE::DURATION);
  }
}

void DuplicateVideosFinder::onChangeSizeDeviation() {
  const QString& szDevStr = sizeDevLE->text();
  bool isIntValid = false;
  int dev = szDevStr.toInt(&isIntValid);
  if (!isIntValid || dev <= 0) {
    LOG_W("size str[%s] is not valid", qPrintable(szDevStr));
    return;
  }
  invalidateLeftGrpTblData();
  m_leftGrpsTbl->setDeviationSize(dev);
}

void DuplicateVideosFinder::onChangeDurationDeviation() {
  const QString& durDevStr = durationDevLE->text();
  bool isIntValid = false;
  int dev = durDevStr.toInt(&isIntValid);
  if (!isIntValid || dev <= 0) {
    LOG_W("duration str[%s] is not valid", qPrintable(durDevStr));
    return;
  }
  invalidateLeftGrpTblData();
  m_leftGrpsTbl->setDeviationDuration(dev);
}

void DuplicateVideosFinder::UpdateWindowsTitleGroupInfo(const QString& dupVidGroupInfoTitle) {
  mTitleGrpInto = dupVidGroupInfoTitle;
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::UpdateWindowsTitleMetaInfo(const QString& dupVidTblMetaInfoTitle) {
  mTitleMetaInfo = dupVidTblMetaInfoTitle;
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::UpdateWindowsTitle() {
  setWindowTitle("Duplicate Videos: " + mTitleMetaInfo + "|" + mTitleGrpInto);
}
