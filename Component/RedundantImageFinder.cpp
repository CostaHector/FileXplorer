﻿#include "RedundantImageFinder.h"

#include <QAbstractTableModel>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QListView>
#include <QTableView>
#include <QToolBar>
#include <QDesktopServices>

#include "Actions/FileBasicOperationsActions.h"
#include "Actions/RedundantImageFinderActions.h"
#include "Component/Notificator.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/StyleSheet.h"
#include "public/UndoRedo.h"
#include "public/PublicMacro.h"
#include "Model/QAbstractTableModelPub.h"

RedunImgLibs RedundantImageFinder::mRedunLibs{"redunSizeHashlib"};

RedundantImageFinder::RedundantImageFinder(QWidget* parent)  //
    : QMainWindow{parent} {
  m_imgModel = new (std::nothrow) RedundantImageModel{this};
  CHECK_NULLPTR_RETURN_VOID(m_imgModel);
  m_table = new (std::nothrow) CustomTableView{"RedundantImageTable", this};
  CHECK_NULLPTR_RETURN_VOID(m_table);

  auto* toolBar = g_redunImgFinderAg().GetRedunImgTB(this);
  CHECK_NULLPTR_RETURN_VOID(toolBar);
  toolBar->addSeparator();
  toolBar->addActions(g_fileBasicOperationsActions().UNDO_REDO_RIBBONS->actions());
  addToolBar(toolBar);

  m_table->setModel(m_imgModel);
  m_table->verticalHeader()->setStretchLastSection(false);
  m_table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);

  m_table->horizontalHeader()->setStretchLastSection(true);
  m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);

  m_table->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
  m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_table->setTextElideMode(Qt::TextElideMode::ElideLeft);
  m_table->setShowGrid(false);

  setCentralWidget(m_table);

  subscribe();

  m_imgModel->setRootPath(&m_imgsBunch);

  setWindowIcon(QIcon(":img/DUPLICATE_IMAGES_FINDER"));
  setWindowTitle("Redundant Images Finder");

  mRedunLibs.LearnSizeAndHashFromRedunImgPath(mRedunLibs.GetRedunPath());
  ReadSetting();
}

RedundantImageFinder::~RedundantImageFinder() {}

void RedundantImageFinder::ReadSetting() {
  if (PreferenceSettings().contains(RedunImgFinderKey::GEOMETRY.name)) {
    restoreGeometry(PreferenceSettings().value(RedunImgFinderKey::GEOMETRY.name).toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
}

void RedundantImageFinder::showEvent(QShowEvent *event) {
  QMainWindow::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void RedundantImageFinder::closeEvent(QCloseEvent* event) {
  g_fileBasicOperationsActions()._DUPLICATE_IMAGES_FINDER->setChecked(false);
  PreferenceSettings().setValue(RedunImgFinderKey::GEOMETRY.name, saveGeometry());
  QMainWindow::closeEvent(event);
}

void RedundantImageFinder::ChangeWindowTitle(const QString& rootPath) {
  setWindowTitle(QString("Redundant Images Finder | %1 | %2 item(s)").arg(rootPath).arg(m_imgsBunch.size()));
}

void RedundantImageFinder::subscribe() {
  connect(g_redunImgFinderAg().RECYLE_NOW, &QAction::triggered,  //
          this, &RedundantImageFinder::RecycleSelection);
  connect(g_redunImgFinderAg().RECYCLE_EMPTY_IMAGE, &QAction::triggered,  //
          this, [](bool recycleEmptyImage) -> void {                      //
            PreferenceSettings().setValue(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, recycleEmptyImage);
          });
  connect(g_redunImgFinderAg().OPEN_REDUNDANT_IMAGES_FOLDER, &QAction::triggered,  //
          this, [this]() {                                                         //
            QDesktopServices::openUrl(QUrl::fromLocalFile(mRedunLibs.GetRedunPath()));
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
  auto isRenameAllSucceed = g_undoRedo.Do(recycleCmds);
  qDebug("Recycle %d item(s) %d.", SELECTED_CNT, isRenameAllSucceed);
  if (isRenameAllSucceed) {
    Notificator::goodNews("Recyle redundant images succeed", QString::number(SELECTED_CNT));
  } else {
    Notificator::badNews("Recyle redundant images failed", QString::number(SELECTED_CNT));
  }
  UpdateDisplayWhenRecycled();
}

void RedundantImageFinder::UpdateDisplayWhenRecycled() {
  decltype(m_imgsBunch) redundantImgs;
  foreach (const REDUNDANT_IMG_INFO& info, m_imgsBunch) {
    if (!QFile::exists(info.filePath))
      continue;
    redundantImgs.append(info);
  }

  int beforeRowCnt = m_imgsBunch.size();
  int afterRowCnt = redundantImgs.size();
  m_imgModel->RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  m_imgsBunch.swap(redundantImgs);
  m_imgModel->RowsCountEndChange();
}

void RedundantImageFinder::operator()(const QString& folderPath) {
  const bool recycleEmptyImage = PreferenceSettings().value(RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.name, RedunImgFinderKey::ALSO_RECYCLE_EMPTY_IMAGE.v).toBool();
  REDUNDANT_IMG_BUNCH newImgs = mRedunLibs.FindRedunImgs(folderPath, recycleEmptyImage);

  int beforeRowCnt = m_imgsBunch.size();
  int afterRowCnt = newImgs.size();
  m_imgModel->RowsCountBeginChange(beforeRowCnt, afterRowCnt);
  m_imgsBunch.swap(newImgs);
  m_imgModel->RowsCountEndChange();
  ChangeWindowTitle(folderPath);
}
