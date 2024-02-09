#include "RenameConflicts.h"
#include "Component/NotificatorFrame.h"
#include "PublicTool.h"
#include "UndoRedo.h"

using namespace STATUS_COLOR;

const QStringList RenameConflicts::COLUMNS_NAME_LIST = {"DateModified", "Size", "Name"};
const int RenameConflicts::COLUMNS_NAME_LIST_LEN = COLUMNS_NAME_LIST.size();

auto RenameConflicts::GetQuickControlToolBar() -> QToolBar* {
  auto* SIZE_ACT(new QAction(QIcon(":/themes/FILESIZE_SCALE"), tr("Size Smaller"), this));
  auto* DATE_ACT(new QAction(QIcon(":/themes/TIME_AXIS"), tr("Date Newer"), this));
  auto* REVERT_ACT(new QAction(QIcon(":/themes/MIRROR_REVERT"), tr("Revert Selection"), this));
  auto* RESET_ACT(new QAction(QIcon(":/themes/RESET"), tr("Reset Table"), this));

  connect(SIZE_ACT, &QAction::triggered, this, &RenameConflicts::on_Size);
  connect(DATE_ACT, &QAction::triggered, this, &RenameConflicts::on_Date);
  connect(REVERT_ACT, &QAction::triggered, this, &RenameConflicts::on_Revert);
  connect(RESET_ACT, &QAction::triggered, this, &RenameConflicts::InitData);

  auto* conflictsControlBar = new QToolBar;
  conflictsControlBar->addWidget(RenameConflicts::getHorizontalSpacer());
  conflictsControlBar->addAction(SIZE_ACT);
  conflictsControlBar->addSeparator();
  conflictsControlBar->addAction(DATE_ACT);
  conflictsControlBar->addSeparator();
  conflictsControlBar->addAction(RESET_ACT);
  conflictsControlBar->addSeparator();
  conflictsControlBar->addAction(REVERT_ACT);
  conflictsControlBar->addWidget(RenameConflicts::getHorizontalSpacer());
  conflictsControlBar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return conflictsControlBar;
}

auto RenameConflicts::TableWidgetGetter() const -> QTableWidget* {
  auto* tw = new QTableWidget;
  tw->setRowCount(0);
  tw->setColumnCount(RenameConflicts::COLUMNS_NAME_LIST.size());
  for (int headIndex = 0; headIndex < RenameConflicts::COLUMNS_NAME_LIST.size(); ++headIndex) {
    tw->setHorizontalHeaderItem(headIndex, new QTableWidgetItem(RenameConflicts::COLUMNS_NAME_LIST[headIndex]));
  }
  tw->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Interactive);
  tw->horizontalHeader()->setStretchLastSection(true);
  tw->setAlternatingRowColors(true);
  tw->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
  tw->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  tw->setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView::NoEditTriggers;
  tw->setDragDropMode(QAbstractItemView::NoDragDrop);
  tw->setContextMenuPolicy(Qt::CustomContextMenu);
  return tw;
}

auto RenameConflicts::ConflictTableEvent(QAction* RECYCLE, QTableWidget* conflictTw, QTableWidget* recycleTw, QTableWidget* opsiteConflictTW)
    -> void {
  auto* menuQWidget = new QMenu(conflictTw);
  menuQWidget->addAction(RECYCLE);

  auto on_RightClick = [menuQWidget]() -> void { menuQWidget->popup(QCursor::pos()); };
  auto on_Recycle = [conflictTw, recycleTw, opsiteConflictTW]() -> void {
    const QList<QTableWidgetSelectionRange>& indexes = conflictTw->selectedRanges();
    if (indexes.isEmpty()) {
      qDebug("please select something first");
      return;
    }
    qDebug("%d range(s) will be recycle", indexes.size());
    for (const auto& rng : indexes) {
      for (auto r = rng.topRow(); r < rng.bottomRow() + 1; ++r) {
        for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
          if (conflictTw->item(r, c) == nullptr) {
            continue;
          }
          recycleTw->setItem(r, c, conflictTw->takeItem(r, c));
          recycleTw->item(r, c)->setBackground(TRANSPARENT_COLOR);
          if (opsiteConflictTW->item(r, c) != nullptr) {
            opsiteConflictTW->item(r, c)->setBackground(LIGHT_GREEN_COLOR);
          }
        }
      }
    }
  };
  connect(conflictTw, &QTableWidget::customContextMenuRequested, on_RightClick);
  connect(RECYCLE, &QAction::triggered, on_Recycle);
}
auto RenameConflicts::RecycleTableEvent(QAction* RESTORE, QTableWidget* recycleTw, QTableWidget* conflictTw, QTableWidget* opositeConflictTw)
    -> void {
  auto* menuQWidget = new QMenu(recycleTw);
  menuQWidget->addAction(RESTORE);

  auto on_RightClick = [menuQWidget]() -> void { menuQWidget->popup(QCursor::pos()); };

  auto on_Restore = [recycleTw, conflictTw, opositeConflictTw]() -> void {
    const QList<QTableWidgetSelectionRange>& indexes = recycleTw->selectedRanges();
    if (indexes.isEmpty()) {
      qDebug("please select something first");
      return;
    }
    qDebug("%d range(s) will be recycle", indexes.size());
    for (const auto& rng : indexes) {
      for (auto r = rng.topRow(); r < rng.bottomRow() + 1; ++r) {
        for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
          if (recycleTw->item(r, c) == nullptr) {
            continue;
          }
          conflictTw->setItem(r, c, recycleTw->takeItem(r, c));
          if (opositeConflictTw->item(r, c) == nullptr) {
            conflictTw->item(r, c)->setBackground(LIGHT_GREEN_COLOR);
          } else {
            conflictTw->item(r, c)->setBackground(TOMATO_COLOR);
            opositeConflictTw->item(r, c)->setBackground(TOMATO_COLOR);
          }
        }
      }
    }
  };
  connect(recycleTw, &QTableWidget::customContextMenuRequested, on_RightClick);
  connect(RESTORE, &QAction::triggered, on_Restore);
}

auto RenameConflicts::NoConflictOperation(const QStringList& selectedItems, const QSet<QString>& leftDeleteSet) const
    -> FileOperation::BATCH_COMMAND_LIST_TYPE {
  FileOperation::BATCH_COMMAND_LIST_TYPE cmds;
  if (OP == CCMMode::CUT or OP == CCMMode::MERGE) {
    for (const QString& nm : selectedItems) {
      if (leftDeleteSet.contains(nm)) {
        continue;
      }
      cmds.append({"rename", leftFolderPath, nm, rightFolderPath, nm});
    }
  } else if (OP == CCMMode::COPY) {
    for (const QString& nm : selectedItems) {
      if (leftDeleteSet.contains(nm)) {
        continue;
      }
      if (QFileInfo(leftFolderPath, nm).isDir()) {
        cmds.append({"mkpath", rightFolderPath, nm});
      } else {
        cmds.append({"cpfile", leftFolderPath, nm, rightFolderPath});
      }
    }
  } else if (OP == CCMMode::LINK) {
    for (const QString& nm : selectedItems) {
      if (leftDeleteSet.contains(nm)) {
        continue;
      }
      cmds.append({"link", leftFolderPath, nm, rightFolderPath});
    }
  }
  if (OP == CCMMode::MERGE) {
    cmds.append({"rmpath", "", leftFolderPath});  // when merge A to B, folder A need to removed
  }
  return cmds;
}

auto RenameConflicts::on_Submit() -> bool {
  const auto& invalidPair = on_Check();
  const auto& invalidLineList = invalidPair.first;
  const auto& bothRemoveLineList = invalidPair.second;
  if (not invalidLineList.isEmpty()) {
    QMessageBox::critical(this, "Submit Fail (Conflict Still Exists)",
                          QString("Please solve the following conflict lines first:\n %1").arg(QIntStr2QString(invalidLineList)));
    return false;
  }
  if (not bothRemoveLineList.isEmpty()) {
    auto ret = QMessageBox::warning(this, "Both Remove Confirm",
                                    QString("The following lines will be remove:\n %1").arg(QIntStr2QString(bothRemoveLineList)),
                                    QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Cancel);
    if (ret == QMessageBox::StandardButton::Cancel) {
      qDebug("Both Remove Confirm: Cancel");
      return false;
    }
    qDebug("Both Remove Confirm: Ok");
  }
  QSet<QString> leftDeleteSet;
  FileOperation::BATCH_COMMAND_LIST_TYPE removeCmds;
  for (int r = 0; r < ROW_COUNT; ++r) {
    auto leftItem = leftRestore->item(r, RenameConflicts::CONFLICT_NAME_COLUMN_INDEX);
    auto rightItem = rightRestore->item(r, RenameConflicts::CONFLICT_NAME_COLUMN_INDEX);
    if (leftItem != nullptr) {
      const QString& nm = leftItem->text();
      leftDeleteSet.insert(nm);
      removeCmds.append({"moveToTrash", leftFolderPath, nm});
    }
    if (rightItem != nullptr) {
      removeCmds.append({"moveToTrash", rightFolderPath, rightItem->text()});
    }
  }

  QString msg;
  if (not removeCmds.isEmpty()) {
    auto isRmvAllSucceed = g_undoRedo.Do(removeCmds);
    qDebug("Submit result: isRmvAllSucceed:%d", isRmvAllSucceed);
    msg += QString("%1 rmv cmd(s): %2.\n").arg(removeCmds.size()).arg(isRmvAllSucceed);
    if (not isRmvAllSucceed) {
      return false;
    }
  }

  const QStringList& leftItems = itemIF.GetLeftRelPathList(OP == CCMMode::CUT or OP == CCMMode::MERGE);
  decltype(removeCmds) renameCmds = NoConflictOperation(leftItems, leftDeleteSet);

  if (not renameCmds.isEmpty()) {
    auto isRenameAllSucceed = g_undoRedo.Do(renameCmds);
    qDebug("Submit is Rename and post remove operation all succeed:%d", isRenameAllSucceed);
    msg += QString("%1 cmd(s) including mv + 0|trailing rmpath cmd: %2.").arg(renameCmds.size()).arg(isRenameAllSucceed);
    if (not isRenameAllSucceed) {
      return false;
    }
  }

  Notificator::information("Rename conflicts submit", msg);
  close();
  return true;
}

auto RenameConflicts::Subscribe() -> void {
  connect(buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &RenameConflicts::on_Submit);
  connect(buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &RenameConflicts::close);
  connect(leftFolderBtn, &QPushButton::clicked, this, [this]() { on_OpenButtonTextFolder(leftFolderBtn); });
  connect(rightFolderBtn, &QPushButton::clicked, this, [this]() { on_OpenButtonTextFolder(rightFolderBtn); });

  connect(leftConflict, &QTableWidget::doubleClicked, this, &RenameConflicts::on_Open);
  connect(rightConflict, &QTableWidget::doubleClicked, this, &RenameConflicts::on_Open);

  connect(leftConflict->verticalScrollBar(), &QScrollBar::valueChanged, rightConflict->verticalScrollBar(), &QScrollBar::setValue);
  connect(rightConflict->verticalScrollBar(), &QScrollBar::valueChanged, leftConflict->verticalScrollBar(), &QScrollBar::setValue);

  connect(leftRestore->verticalScrollBar(), &QScrollBar::valueChanged, rightRestore->verticalScrollBar(), &QScrollBar::setValue);
  connect(rightRestore->verticalScrollBar(), &QScrollBar::valueChanged, leftRestore->verticalScrollBar(), &QScrollBar::setValue);

  if (OP == CCMMode::COPY) {  // disable left table delete action when copy
    disconnect(RECYCL_LEFT, &QAction::triggered, nullptr, nullptr);
    disconnect(RESTORE_LEFT, &QAction::triggered, nullptr, nullptr);
  }
}

auto RenameConflicts::InitData() -> void {
  leftConflict->setRowCount(0);
  rightConflict->setRowCount(0);
  leftRestore->setRowCount(0);
  rightRestore->setRowCount(0);

  leftConflict->setRowCount(ROW_COUNT);
  rightConflict->setRowCount(ROW_COUNT);
  leftRestore->setRowCount(ROW_COUNT);
  rightRestore->setRowCount(ROW_COUNT);

  int r = 0;
  QFileIconProvider iconProvider;
  for (int r = 0; r < ROW_COUNT; ++r) {
    leftConflict->setItem(r, 0, new QTableWidgetItem(lCommonInfo[r].dateModified));
    leftConflict->setItem(r, 1, new QTableWidgetItem(lCommonInfo[r].size));
    leftConflict->setItem(r, 2, new QTableWidgetItem(lCommonInfo[r].name));
    leftConflict->item(r, 0)->setBackground(TOMATO_COLOR);
    leftConflict->item(r, 1)->setBackground(TOMATO_COLOR);
    leftConflict->item(r, 2)->setBackground(TOMATO_COLOR);

    const QString& nm = rCommonInfo[r].name;
    rightConflict->setItem(r, 0, new QTableWidgetItem(rCommonInfo[r].dateModified));
    rightConflict->setItem(r, 1, new QTableWidgetItem(rCommonInfo[r].size));
    rightConflict->setItem(r, 2, new QTableWidgetItem(iconProvider.icon(QFileInfo(rightFolderPath + '/' + nm)), nm));
    rightConflict->item(r, 0)->setBackground(TOMATO_COLOR);
    rightConflict->item(r, 1)->setBackground(TOMATO_COLOR);
    rightConflict->item(r, 2)->setBackground(TOMATO_COLOR);
  }
}

auto RenameConflicts::on_Size() -> void {
  auto leftSelectionModel = leftConflict->selectionModel();
  auto rightSelectionModel = rightConflict->selectionModel();
  auto leftModel = leftConflict->model();
  auto rightModel = rightConflict->model();
  for (auto r = 0; r < ROW_COUNT; ++r) {
    if ((leftConflict->item(r, 0) == nullptr) or (rightConflict->item(r, 0) == nullptr)) {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Deselect);
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Deselect);
      }
      continue;
    }
    if (leftConflict->item(r, 1)->text().toInt() < rightConflict->item(r, 1)->text().toInt()) {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Select);
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Deselect);
      }
    } else {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Deselect);
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Select);
      }
    }
  }
}
auto RenameConflicts::on_Date() -> void {
  auto* leftSelectionModel = leftConflict->selectionModel();
  auto* rightSelectionModel = rightConflict->selectionModel();
  auto* leftModel = leftConflict->model();
  auto* rightModel = rightConflict->model();
  for (auto r = 0; r < ROW_COUNT; ++r) {
    if ((leftConflict->item(r, 0) == nullptr) or (rightConflict->item(r, 0) == nullptr)) {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Deselect);
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Deselect);
      }
      continue;
    }
    if (leftConflict->item(r, 0)->text() < rightConflict->item(r, 0)->text()) {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Select);
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Deselect);
      }
    } else {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Deselect);
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Select);
      }
    }
  }
}
auto RenameConflicts::on_Revert() -> void {
  auto* leftSelectionModel = leftConflict->selectionModel();
  auto* rightSelectionModel = rightConflict->selectionModel();
  auto* leftModel = leftConflict->model();
  auto* rightModel = rightConflict->model();
  for (auto r = 0; r < ROW_COUNT; ++r) {
    if (leftConflict->item(r, 0) != nullptr) {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        leftSelectionModel->select(leftModel->index(r, c), QItemSelectionModel::Toggle);
      }
    }
    if (rightConflict->item(r, 0) != nullptr) {
      for (auto c = 0; c < RenameConflicts::COLUMNS_NAME_LIST_LEN; ++c) {
        rightSelectionModel->select(rightModel->index(r, c), QItemSelectionModel::Toggle);
      }
    }
  }
}

auto RenameConflicts::on_Open(const QModelIndex index) -> bool {
  QString pth;
  if (leftConflict->hasFocus()) {
    auto item = leftConflict->item(index.row(), RenameConflicts::CONFLICT_NAME_COLUMN_INDEX);
    if (item == nullptr) {
      return true;
    }
    const QString& nm = item->text();
    pth = QDir(leftFolderPath).absoluteFilePath(nm);
  } else if (rightConflict->hasFocus()) {
    auto item = rightConflict->item(index.row(), RenameConflicts::CONFLICT_NAME_COLUMN_INDEX);
    if (item == nullptr) {
      return true;
    }
    const QString& nm = item->text();
    pth = QDir(rightFolderPath).absoluteFilePath(nm);
  } else {
    return true;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
}

auto RenameConflicts::eventFilter(QObject* obj, QEvent* event) -> bool {
  if (event->type() == QEvent::KeyPress) {
    auto* e = dynamic_cast<QKeyEvent*>(event);
    if (e->key() == Qt::Key_Delete) {
      if (leftConflict->hasFocus()) {
        emit RECYCL_LEFT->triggered(false);
        return true;
      } else if (rightConflict->hasFocus()) {
        emit RECYCL_RIGHT->triggered(false);
        return true;
      } else if (leftRestore->hasFocus()) {
        emit RESTORE_LEFT->triggered(false);
        return true;
      } else if (rightRestore->hasFocus()) {
        emit RESTORE_RIGHT->triggered(false);
        return true;
      }
    }
  }
  return QDialog::eventFilter(obj, event);
}

class RenameConflictsIllu : public QWidget {
 public:
  static const QString& DONT_CHANGE_SRC;
  static const QString& ENV_PATH;

  auto startEvent() -> bool {
    if (QFile::exists(ENV_PATH)) {
      auto ret = QDir(ENV_PATH).removeRecursively();
      if (not ret) {
        qDebug("[Error] when clear environment folder");
        return false;
      }
    }
    auto ret = PublicTool::copyDirectoryFiles(DONT_CHANGE_SRC, ENV_PATH);
    if (not ret) {
      qDebug("Copy environment folder failed");
      return false;
    }
    return true;
  }

  auto closeEvent(QCloseEvent* event) -> void override {
    if (QFile::exists(ENV_PATH)) {
      auto ret = QDir(ENV_PATH).removeRecursively();
      if (not ret) {
        qDebug("[Error] when clear environment folder");
      }
    }
    return QWidget::closeEvent(event);
  }

  explicit RenameConflictsIllu(QWidget* parent = nullptr) : QWidget(parent) {
    if (not startEvent()) {
      qDebug("Environment not met");
      return;
    }

    auto leftFolderPath = QDir(RenameConflictsIllu::ENV_PATH).absoluteFilePath("Movie 1994");
    auto rightFolderPath = QDir(RenameConflictsIllu::ENV_PATH).absoluteFilePath("Movie 1994 DVD");
    ConflictsItemHelper conflictItem{leftFolderPath, rightFolderPath};
    auto ttfm = new RenameConflicts(conflictItem, CCMMode::MERGE, this);  // here the ",self" close parent alse close this sub window;
    // ttfm.exec_() // here block parent window;
    ttfm->show();
    setWindowTitle("close me will also close RenameConflicts");
  }
};
const QString& RenameConflictsIllu::DONT_CHANGE_SRC =
    QFileInfo(QFileInfo(__FILE__).absolutePath()).absoluteDir().absoluteFilePath("TestCase/test/TestEnv_FileSystemEnv/DONT_CHANGE");
const QString& RenameConflictsIllu::ENV_PATH =
    QFileInfo(QFileInfo(__FILE__).absolutePath()).absoluteDir().absoluteFilePath("TestCase/test/TestEnv_FileSystemEnv/CHANGABLE");

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RenameConflictsIllu wid;
  wid.show();
  return a.exec();
}
#endif
