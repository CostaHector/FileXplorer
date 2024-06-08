#include "ConflictsRecycle.h"
#include "Actions/ConflictsSolveActions.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"
#include "UndoRedo.h"

#include <QDesktopServices>
#include <QPushButton>

using namespace STATUS_COLOR;

ConflictsRecycle::ConflictsRecycle(const ConflictsItemHelper& itemIF_, QWidget* parent)
    : QDialog(parent),
      itemIF(itemIF_),
      m_conflictModel{new ConflictsFileSystemModel{itemIF_, this}},
      m_conflictTV{new CustomTableView{"ConflictFileSystemTable", this}},
      m_conflictTB{g_conflictSolveAct().GetBatchChangeSelectionToolBar(this)},
      m_conflictMenu{g_conflictSolveAct().GetMenu(this)} {
  m_conflictTV->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
  m_conflictTV->setModel(m_conflictModel);
  m_conflictTV->BindMenu(m_conflictMenu);

  m_mainLayout->addWidget(m_conflictTB);
  m_mainLayout->addWidget(m_conflictTV, 7);
  m_mainLayout->addWidget(buttonBox);
  setLayout(m_mainLayout);

  Subscribe();

  ReadSettings();

  setWindowTitle(QString("%1/%2 item(s) Conflict | Operation[%3]")
                     .arg(itemIF.commonList.size())
                     .arg(itemIF.m_fromPathItems.size())
                     .arg(CCMMode2QString.value(itemIF_.m_mode, "unknown")));
  setWindowIcon(QIcon(":/themes/CONFLICT"));
}

void ConflictsRecycle::ReadSettings() {
  if (PreferenceSettings().contains("CONFLICT_RECYLE_TABLE_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("CONFLICT_RECYLE_TABLE_GEOMETRY").toByteArray());
  } else {
    setGeometry(DEFAULT_GEOMETRY);
  }
  m_conflictTV->InitTableView();
  if (g_conflictSolveAct().HIDE_NO_CONFLICT_ITEM->isChecked()) {
    onHideNoConflictLine(true);
  }
}

void ConflictsRecycle::hideEvent(QHideEvent* event) {
  //  g_fileLeafActions()._LOGGING->setChecked(false);
  QDialog::hideEvent(event);
}

void ConflictsRecycle::closeEvent(QCloseEvent* event) {
  PreferenceSettings().setValue("CONFLICT_RECYLE_TABLE_GEOMETRY", saveGeometry());
  QDialog::closeEvent(event);
}

auto ConflictsRecycle::on_completeMerge() -> bool {
  if (not m_conflictModel->isCommandsAvail()) {
    m_conflictModel->updateCommands();
  }
  const auto& cmds = m_conflictModel->getCommands();
  auto isRenameAllSucceed = g_undoRedo.Do(cmds);
  Notificator::information("Rename conflicts submit", QString("%1 command(s) result:%2").arg(cmds.size()).arg(isRenameAllSucceed));
  close();
  return true;
}

bool ConflictsRecycle::on_ShowCommand() {
  if (not m_conflictModel->isCommandsAvail()) {
    m_conflictModel->updateCommands();
  }
  m_commandsPreview->appendPlainText(m_conflictModel->displayCommands());
  m_commandsPreview->setWindowTitle(QString("Conflict solve names unique | Total %1 items(s)").arg(m_conflictModel->rowCount()));
  m_commandsPreview->setMinimumWidth(1024);
  m_commandsPreview->raise();
  m_commandsPreview->show();
}

auto ConflictsRecycle::Subscribe() -> void {
  connect(buttonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &ConflictsRecycle::on_completeMerge);
  connect(buttonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &ConflictsRecycle::close);
  connect(buttonBox->button(QDialogButtonBox::StandardButton::Help), &QPushButton::clicked, this, &ConflictsRecycle::on_ShowCommand);

  connect(g_conflictSolveAct().SIZE_ACT, &QAction::triggered, this,
          [this]() { m_conflictModel->setKeepItemPriority(ConflictsFileSystemModel::KEEP_PRIORITY::SIZE_LARGE_THEN_NEWER_TIME); });
  connect(g_conflictSolveAct().DATE_ACT, &QAction::triggered, this,
          [this]() { m_conflictModel->setKeepItemPriority(ConflictsFileSystemModel::KEEP_PRIORITY::NEWER_TIME_THEN_SIZE_LARGE); });
  connect(g_conflictSolveAct().ALWAYS_KEEP_LEFT_SIDE, &QAction::triggered, this,
          [this]() { m_conflictModel->setKeepItemPriority(ConflictsFileSystemModel::KEEP_PRIORITY::ALWAYS_SIDE_LEFT); });
  connect(g_conflictSolveAct().ALWAYS_KEEP_RIGHT_SIDE, &QAction::triggered, this,
          [this]() { m_conflictModel->setKeepItemPriority(ConflictsFileSystemModel::KEEP_PRIORITY::ALWAYS_SIDE_RIGHT); });
  connect(g_conflictSolveAct().REVERT_ACT, &QAction::triggered, this,
          [this](const bool checked) { m_conflictModel->setRevertKeepItemPriority(checked); });

  connect(g_conflictSolveAct().HIDE_NO_CONFLICT_ITEM, &QAction::triggered, this, &ConflictsRecycle::onHideNoConflictLine);

  connect(g_conflictSolveAct().RECYCLE_SELECTION, &QAction::triggered, m_conflictModel, [this]() { onSetRecycleOrKeepChoiceByUser(true); });
  connect(g_conflictSolveAct().KEEP_SELECTION, &QAction::triggered, m_conflictModel, [this]() { onSetRecycleOrKeepChoiceByUser(false); });

  g_conflictSolveAct().LEFT_FOLDER->setText(itemIF.l);
  g_conflictSolveAct().RIGHT_FOLDER->setText(itemIF.r);
  connect(g_conflictSolveAct().LEFT_FOLDER, &QAction::triggered, g_conflictSolveAct().LEFT_FOLDER,
          []() { QDesktopServices::openUrl(QUrl::fromLocalFile(g_conflictSolveAct().LEFT_FOLDER->text())); });
  connect(g_conflictSolveAct().RIGHT_FOLDER, &QAction::triggered, g_conflictSolveAct().RIGHT_FOLDER,
          []() { QDesktopServices::openUrl(QUrl::fromLocalFile(g_conflictSolveAct().RIGHT_FOLDER->text())); });

  connect(m_conflictTV, &QTableView::doubleClicked, this, &ConflictsRecycle::on_Open);
}

bool ConflictsRecycle::onSetRecycleOrKeepChoiceByUser(const bool isSetDelete) {
  if (not m_conflictTV) {
    return false;
  }
  const auto& indexes = m_conflictTV->selectionModel()->selectedIndexes();
  if (indexes.isEmpty()) {
    return true;
  }
  m_conflictModel->setDeleteOrRecycle(indexes, isSetDelete);
  return true;
}

bool ConflictsRecycle::onHideNoConflictLine(const bool isSetHide) {
  if (m_conflictTV == nullptr || m_conflictModel == nullptr) {
    return false;
  }
  const auto& mp = m_conflictModel->getConflictMap();
  for (int r = 0; r < m_conflictModel->rowCount(); ++r) {
    if (mp.contains(r)) {
      continue;
    }
    // not conflict one
    if (isSetHide) {
      m_conflictTV->hideRow(r);
    } else {
      m_conflictTV->showRow(r);
    }
  }
  return true;
}

bool ConflictsRecycle::on_Open(const QModelIndex& index) {
  if (m_conflictModel == nullptr) {
    return false;
  }
  const QString& filePath = m_conflictModel->filePath(index);
  if (filePath.isEmpty()) {
    return false;
  }
  return QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
}

void ConflictsRecycle::keyPressEvent(QKeyEvent* e) {
  if (e->key() == Qt::Key_Delete) {
    emit g_conflictSolveAct().RECYCLE_SELECTION->triggered();
    return;
  }
  QDialog::keyPressEvent(e);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  //  RenameConflictsIllu wid;

  const QString& ENV_PATH =
      QFileInfo(QFileInfo(__FILE__).absolutePath()).absoluteDir().absoluteFilePath("TestCase/test/TestEnv_ConflictSolve/CHANGABLE");
  const QString& DONT_CHANGE_SRC =
      QFileInfo(QFileInfo(__FILE__).absolutePath()).absoluteDir().absoluteFilePath("TestCase/test/TestEnv_ConflictSolve/DONT_CHANGE");
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
    return -1;
  }

  auto leftFolderPath = QDir(ENV_PATH).absoluteFilePath("Page12");
  auto rightFolderPath = QDir(ENV_PATH).absoluteFilePath("");
  ConflictsItemHelper conflictItem{leftFolderPath, rightFolderPath, QStringList{"Movie"}, CCMMode::CUT};
  ConflictsRecycle win(conflictItem);  // here the ",self" close parent alse close this sub window;

  win.show();
  return a.exec();
}
#endif
