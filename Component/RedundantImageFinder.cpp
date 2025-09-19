#include "RedundantImageFinder.h"

#include "FileBasicOperationsActions.h"
#include "RedundantImageFinderActions.h"
#include "NotificatorMacro.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "UndoRedo.h"
#include "PublicMacro.h"
#include "SpacerWidget.h"
#include "PathTool.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QItemSelectionModel>
#include <QDesktopServices>
#include <QMenu>

constexpr char RedundantImageFinder::GEOMETRY_KEY[];

RedundantImageFinder::RedundantImageFinder(QWidget* parent)  //
    : QMainWindow{parent}                                    //
{
  mResultAlsoContainEmptyImage =
      Configuration().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool();

  auto& redunInst = g_redunImgFinderAg();
  QMenu* findByMenu = new (std::nothrow) QMenu{"Find by Menu", this};
  CHECK_NULLPTR_RETURN_VOID(findByMenu);
  findByMenu->addActions(redunInst.mDecideByIntAction.getActionEnumAscendingList());

  mFindImgByTb = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(mFindImgByTb);
  mFindImgByTb->setIcon(QIcon{":img/DUPLICATE_IMAGES_FINDER"});
  mFindImgByTb->setText("Find By Mode");
  mFindImgByTb->setMenu(findByMenu);
  mFindImgByTb->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  mFindImgByTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  auto* pSpacer = GetSpacerWidget(this, Qt::Orientation::Horizontal);
  CHECK_NULLPTR_RETURN_VOID(pSpacer);

  m_toolBar = new (std::nothrow) QToolBar{"Redun toolbar", this};
  CHECK_NULLPTR_RETURN_VOID(m_toolBar);
  m_toolBar->addWidget(mFindImgByTb);
  m_toolBar->addAction(redunInst.INCLUDING_EMPTY_IMAGES);
  m_toolBar->addAction(redunInst.OPEN_BENCHMARK_FOLDER);
  m_toolBar->addAction(redunInst.RELOAD_BENCHMARK_LIB);
  m_toolBar->addSeparator();
  m_toolBar->addAction(redunInst.RECYLE_NOW);
  m_toolBar->addWidget(pSpacer);
  m_toolBar->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  m_toolBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  addToolBar(m_toolBar);

  m_imgModel = new (std::nothrow) RedundantImageModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_imgModel);
  m_table = new (std::nothrow) CustomTableView{"RedundantImageTable", this};
  CHECK_NULLPTR_RETURN_VOID(m_table);
  m_table->setModel(m_imgModel);
  setCentralWidget(m_table);

  subscribe();

  m_imgModel->setRootPath(&m_imgsBunch);

  setWindowIcon(mFindImgByTb->icon());
  ChangeWindowTitle(mCurrentPath);
  ReadSetting();
}

void RedundantImageFinder::ReadSetting() {
  if (Configuration().contains(GEOMETRY_KEY)) {
    restoreGeometry(Configuration().value(GEOMETRY_KEY).toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void RedundantImageFinder::showEvent(QShowEvent* event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void RedundantImageFinder::closeEvent(QCloseEvent* event) {
  g_fileBasicOperationsActions()._DUPLICATE_IMAGES_FINDER->setChecked(false);
  Configuration().setValue(GEOMETRY_KEY, saveGeometry());
  Configuration().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, mResultAlsoContainEmptyImage);
  QMainWindow::closeEvent(event);
}

void RedundantImageFinder::ChangeWindowTitle(const QString& rootPath) {
  setWindowTitle(QString("Redundant Images Finder | Path: %1 | %2 item(s)").arg(rootPath).arg(m_imgsBunch.size()));
}

void RedundantImageFinder::subscribe() {
  auto& inst = g_redunImgFinderAg();
  connect(inst.RECYLE_NOW, &QAction::triggered, this, &RedundantImageFinder::RecycleSelection);
  connect(inst.INCLUDING_EMPTY_IMAGES, &QAction::toggled, this, &RedundantImageFinder::setResultAlsoContainEmptyImage);
  connect(inst.mDecideByIntAction.getActionGroup(), &QActionGroup::triggered, this, &RedundantImageFinder::whenModeChanged);

  ImagesInfoManager& redunImgLibInst = ImagesInfoManager::getInst();
  connect(inst.OPEN_BENCHMARK_FOLDER, &QAction::triggered, this, [&redunImgLibInst]() {
    const QString benchmarkPath = redunImgLibInst.GetDynRedunPath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(benchmarkPath));
  });
  connect(inst.RELOAD_BENCHMARK_LIB, &QAction::triggered, [&redunImgLibInst]() {
    redunImgLibInst.ForceReloadImpl();
  });
  connect(m_table, &QAbstractItemView::doubleClicked, this, [this](const QModelIndex& clickedIndex) {
    const QString imgPath{m_imgModel->filePath(clickedIndex)};
    QDesktopServices::openUrl(QUrl::fromLocalFile(imgPath));
  });
}

void RedundantImageFinder::RecycleSelection() {
  const QModelIndexList& sel = m_table->selectionModel()->selectedRows();
  const int SELECTED_CNT = sel.size();
  if (SELECTED_CNT <= 0) {
    return;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE recycleCmds;
  recycleCmds.reserve(SELECTED_CNT);
  for (const auto& srcInd : sel) {
    recycleCmds.append(ACMD::GetInstMOVETOTRASH("", m_imgModel->filePath(srcInd)));
  }
  bool isRenameAllSucceed = UndoRedo::GetInst().Do(recycleCmds);
  if (isRenameAllSucceed) {
    LOG_OK_P("Recyle redundant images succeed", "selected count: %d", SELECTED_CNT);
  } else {
    LOG_ERR_P("Recyle redundant images failed", "selected count: %d", SELECTED_CNT);
  }
  UpdateDisplayWhenRecycled();
}

void RedundantImageFinder::whenModeChanged() {
  operator()(mCurrentPath);
}

void RedundantImageFinder::UpdateDisplayWhenRecycled() {
  decltype(m_imgsBunch) redundantImgs;
  redundantImgs.reserve(m_imgsBunch.size());
  foreach (const REDUNDANT_IMG_INFO& info, m_imgsBunch) {
    if (!QFile::exists(info.filePath)) {
      continue;
    }
    redundantImgs.append(info);
  }

  int beforeRowCnt = m_imgsBunch.size();
  int afterRowCnt = redundantImgs.size();
  m_imgModel->RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  m_imgsBunch.swap(redundantImgs);
  m_imgModel->RowsCountEndChange();
}

void RedundantImageFinder::operator()(const QString& folderPath) {
  if (PathTool::isRootOrEmpty(folderPath) || !QFileInfo{folderPath}.isDir()) {
    LOG_ERR_P("[Abort] Find redundant image", "Path[%s] not exists", qPrintable(folderPath));
    return;
  }
  mCurrentPath = folderPath;
  using namespace RedundantImageTool;
  const DecideByE decideBy = g_redunImgFinderAg().GetCurFindDupBy();
  ImagesInfoManager& redunLibsInst = ImagesInfoManager::getInst();

  RedundantImagesList newImgs;
  switch (decideBy) {
    case DecideByE::LIBRARY: {
      newImgs = redunLibsInst.FindRedunImgs(mCurrentPath, mResultAlsoContainEmptyImage);
      break;
    }
    case DecideByE::MD5: {
      newImgs = FindDuplicateImgs(mCurrentPath, mResultAlsoContainEmptyImage);
      break;
    }
    default: {
      LOG_W("DecideBy Enum[%s] not support", RedundantImageTool::c_str(decideBy));
      break;
    }
  }

  int beforeRowCnt = m_imgsBunch.size();
  int afterRowCnt = newImgs.size();
  m_imgModel->RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  m_imgsBunch.swap(newImgs);
  m_imgModel->RowsCountEndChange();
  ChangeWindowTitle(mCurrentPath);
  LOG_OK_P(mCurrentPath, "%d images(s) found according to[%s]", afterRowCnt, c_str(decideBy));
}
