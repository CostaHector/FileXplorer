#include "DuplicateVideosFinder.h"
#include "Actions/DuplicateVideosFinderActions.h"
#include "Actions/FileBasicOperationsActions.h"
#include "public/PublicVariable.h"
#include "public/UndoRedo.h"

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
    qWarning("cb is nullptr, Copy[%s] failed", qPrintable(name));
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
    qDebug("nothing selected to recycle");
    return;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(SELECTED_CNT);
  for (const auto& proInd : selectionModel()->selectedRows()) {
    const auto& srcInd = m_sortProxy->mapToSource(proInd);
    recycleCmds.append(ACMD{MOVETOTRASH, {"", m_detailsModel->filePath(srcInd)}});
  }
  auto isRenameAllSucceed = g_undoRedo.Do(recycleCmds);
  qDebug("Recycle %d item(s) %d.", SELECTED_CNT, isRenameAllSucceed);
}

void RightDuplicateDetails::subscribe() {
  connect(this, &QTableView::doubleClicked, this, &RightDuplicateDetails::on_cellDoubleClicked);
  connect(g_dupVidFinderAg().RECYCLE_ONE_FILE, &QAction::triggered, this, &RightDuplicateDetails::onRecycleSelection);
}

// -------------------------------------------------------------------------------------------------
const QString DuplicateVideosFinder::DUPLICATE_FINDER_TEMPLATE = "Duplicate Videos Finder(Differ by %1) | %2 batch(es) | total %3 video(s)";

DuplicateVideosFinder::DuplicateVideosFinder(QWidget* parent) : QMainWindow{parent} {
  m_tb = g_dupVidFinderAg().GetAiMediaToolBar(this);
  m_tb->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  m_aiTables = new AiMediaDupTableView{this};
  m_aiTablesTB = new QToolBar{"Ai Media Tables", this};
  m_aiTablesTB->addWidget(m_aiTables);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_tb);
  addToolBarBreak(Qt::ToolBarArea::TopToolBarArea);
  addToolBar(Qt::ToolBarArea::TopToolBarArea, m_aiTablesTB);

  m_dupList = new LeftDuplicateList{this};
  m_details = new RightDuplicateDetails{this};
  m_mainWidget = new QSplitter{Qt::Orientation::Horizontal, this};

  m_mainWidget->addWidget(m_dupList);
  m_mainWidget->addWidget(m_details);

  setCentralWidget(m_mainWidget);

  m_details->setSharedMember(&m_dupList->m_dupListModel->m_classifiedSort, &m_dupList->m_dupListModel->m_currentDiffer);
  subscribe();

  updateWindowsSize();
  UpdateWindowsTitle();
  setWindowIcon(QIcon(":img/DUPLICATE_VIDEOS_FINDER"));

  UpdateAiMediaTableNames();
}

void DuplicateVideosFinder::UpdateWindowsTitle() {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    return;
  }
  auto* pDupListModel = m_dupList->m_dupListModel;
  setWindowTitle(
      DUPLICATE_FINDER_TEMPLATE.arg(pDupListModel->getDifferTypeStr()).arg(pDupListModel->rowCount()).arg(pDupListModel->getReadVidsCount()));
}

void DuplicateVideosFinder::updateWindowsSize() {
  if (PreferenceSettings().contains("DuplicateVideosFinderGeometry")) {
    restoreGeometry(PreferenceSettings().value("DuplicateVideosFinderGeometry").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_mainWidget->restoreState(PreferenceSettings().value("DuplicateVideosFinderSplitterState", QByteArray()).toByteArray());
}

void DuplicateVideosFinder::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("DuplicateVideosFinderGeometry", saveGeometry());
  PreferenceSettings().setValue("DuplicateVideosFinderSplitterState", m_mainWidget->saveState());
  QMainWindow::closeEvent(event);
}

void DuplicateVideosFinder::keyPressEvent(QKeyEvent* e) {
  if (e->modifiers() == Qt::KeyboardModifier::ControlModifier && e->key() == Qt::Key_Insert) {
    const QModelIndex& playInd = m_details->currentIndex();
    if (!playInd.isValid()) {
      qWarning("invalid index ignore copy everything name");
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
          qWarning("playInd is invalid");
          return;
        }
        emit m_details->doubleClicked(m_details->currentIndex());
        return;
      }
      case Qt::KeyboardModifier::ShiftModifier: {
        const QModelIndex& bef = m_dupList->currentIndex();
        if (not bef.isValid()) {
          qWarning("before index is invalid");
          return;
        }
        const QModelIndex& aft = m_dupList->model()->index(bef.row() + 1, bef.column());
        if (not aft.isValid()) {
          qWarning("after index is invalid");
          return;
        }
        m_dupList->setCurrentIndex(aft);
        emit m_dupList->currentChanged(aft, bef);
        return;
      }
      default:
        break;
    }
  }
  return QMainWindow::keyPressEvent(e);
}

bool DuplicateVideosFinder::TablesGroupChangedTo(const QStringList& tbls) {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    qWarning("Duplicate list or its model is nullptr");
    return false;
  }
  m_dupList->m_dupListModel->ChangeTableGroups(tbls);
  UpdateWindowsTitle();
  return true;
}

void DuplicateVideosFinder::subscribe() {
  connect(g_dupVidFinderAg().ANALYSE_THESE_TABLES, &QAction::triggered, this, &DuplicateVideosFinder::onAnalyseAiMediaTableChanged);
  connect(g_dupVidFinderAg().CANCEL_ANALYSE, &QAction::triggered, this, &DuplicateVideosFinder::onCancelAnalyse);
  connect(g_dupVidFinderAg().DIFFER_BY, &QActionGroup::triggered, this, &DuplicateVideosFinder::onDifferTypeChanged);
  connect(g_dupVidFinderAg().durationDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeDurationDeviation);
  connect(g_dupVidFinderAg().sizeDevLE, &QLineEdit::returnPressed, this, &DuplicateVideosFinder::onChangeSizeDeviation);
  connect(m_dupList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DuplicateVideosFinder::on_selectionChanged);
}

void DuplicateVideosFinder::on_selectionChanged() {
  const auto& proxyIndex = m_dupList->currentIndex();
  const auto& srcIndex = m_dupList->m_sortProxy->mapToSource(proxyIndex);

  if (not srcIndex.isValid()) {
    return;
  }
  m_details->clearSelection();
  m_details->m_detailsModel->onChangeDetailIndex(srcIndex.row());
}

void DuplicateVideosFinder::UpdateAiMediaTableNames() {
  if (m_aiTables == nullptr) {
    qWarning("m_aiTables is nullptr");
    return;
  }
  m_aiTables->LoadAiMediaTableNames();
}

void DuplicateVideosFinder::onAnalyseAiMediaTableChanged() {
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  const QStringList& tbls = m_aiTables->GetSelectedAiTables();
  TablesGroupChangedTo(tbls);
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::onCancelAnalyse() {
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  TablesGroupChangedTo({});
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::onDifferTypeChanged(QAction* newDifferAct) {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    qWarning("Duplicate list or its model is nullptr when set differ by duration");
    return;
  }
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  if (newDifferAct == g_dupVidFinderAg().DIFFER_BY_SIZE) {
    m_dupList->m_dupListModel->setDifferType(DIFFER_BY_TYPE::SIZE);
  } else if (newDifferAct == g_dupVidFinderAg().DIFFER_BY_DURATION) {
#ifdef _WIN32
    m_dupList->m_dupListModel->setDifferType(DIFFER_BY_TYPE::DURATION);
#else
    QMessageBox::warning(this, "Cannot differ by type", "MediaInfo lib is not lib");
#endif
  }
  UpdateWindowsTitle();
}

void DuplicateVideosFinder::onChangeSizeDeviation() {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    qWarning("Duplicate list or its model is nullptr when onChangeSizeDeviation");
    return;
  }
  const QString& szDevStr = g_dupVidFinderAg().sizeDevLE->text();
  bool isIntValid = false;
  int dev = szDevStr.toInt(&isIntValid);
  if (not isIntValid or dev <= 0) {
    qWarning("size str[%s] is not valid", qPrintable(szDevStr));
    return;
  }
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  m_dupList->m_dupListModel->setDeviationSize(dev);
  UpdateWindowsTitle();
}
void DuplicateVideosFinder::onChangeDurationDeviation() {
  if (m_dupList == nullptr or m_dupList->m_dupListModel == nullptr) {
    qWarning("Duplicate list or its model is nullptr when onChangeDurationDeviation");
    return;
  }
  const QString& durDevStr = g_dupVidFinderAg().durationDevLE->text();
  bool isIntValid = false;
  int dev = durDevStr.toInt(&isIntValid);
  if (not isIntValid or dev <= 0) {
    qWarning("duration str[%s] is not valid", qPrintable(durDevStr));
    return;
  }
  m_dupList->clearSelection();
  m_details->m_detailsModel->whenDifferTypeAboutToChanged();
  m_dupList->m_dupListModel->setDeviationDuration(dev);
  UpdateWindowsTitle();
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  DuplicateVideosFinder mainWindow;
  mainWindow.show();
  a.exec();
  return 0;
}
#endif
