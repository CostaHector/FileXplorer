#include "SceneTableView.h"
#include "Model/ScenesTableModel.h"
#include "Tools/PlayVideo.h"
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QClipboard>

class AlignDelegate : public QStyledItemDelegate {
 public:
  void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override {
    QStyledItemDelegate::initStyleOption(option, index);
    option->decorationPosition = QStyleOptionViewItem::Position::Bottom;
  }
};

SceneTableView::SceneTableView(ScenesTableModel* sceneModel, QWidget* parent) : CustomTableView{"SCENES_TABLE", parent}, _sceneModel{sceneModel} {
  if (_sceneModel == nullptr) {
    qWarning("sceneModel is nullptr");
  } else {
    setModel(_sceneModel);
  }
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);

  mAlignDelegate = new AlignDelegate;
  setItemDelegate(mAlignDelegate);

  m_menu = new QMenu{"scene table view menu", this};
  COPY_BASENAME_FROM_SCENE = new QAction("copy basename", m_menu);
  OPEN_CORRESPONDING_FOLDER = new QAction("play this folder", m_menu);
  m_menu->addAction(COPY_BASENAME_FROM_SCENE);
  m_menu->addAction(OPEN_CORRESPONDING_FOLDER);
  BindMenu(m_menu);
  //  BindMenu(g_performersManagerActions().GetRightClickMenu());
  //  AppendVerticalHeaderMenuAGS(g_performersManagerActions().GetVerAGS());
  //  AppendHorizontalHeaderMenuAGS(g_performersManagerActions().GetHorAGS());
  subscribe();
}

void SceneTableView::onCopyBaseName() {
  const QModelIndex& curInd = currentIndex();
  const QString& copiedStr = _sceneModel->baseName(curInd);
  auto* cb = QApplication::clipboard();
  cb->setText(copiedStr, QClipboard::Mode::Clipboard);
  qDebug("user copied str: [%s]", qPrintable(copiedStr));
}

void SceneTableView::onOpenCorrespondingFolder() {
  const QModelIndex& curInd = currentIndex();
  const QString& scenePath = _sceneModel->absolutePath(curInd);
  on_ShiftEnterPlayVideo(scenePath);
  qDebug("Play path: [%s]", qPrintable(scenePath));
}

void SceneTableView::subscribe() {
  connect(COPY_BASENAME_FROM_SCENE, &QAction::triggered, this, &SceneTableView::onCopyBaseName);
  connect(OPEN_CORRESPONDING_FOLDER, &QAction::triggered, this, &SceneTableView::onOpenCorrespondingFolder);
}

void SceneTableView::setRootPath(const QString& rootPath) {
  if (rootPath.count('/') < 2) {  // large folder
    qDebug("rootPath[%s] may contains a large item(s)", qPrintable(rootPath));
    const auto ret = QMessageBox::warning(this, "Large folder alert(May cause LAG)", rootPath,
                                          QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      qDebug("User cancel setRootPath on a large path[%s]", qPrintable(rootPath));
      return;
    }
  }
  _sceneModel->setRootPath(rootPath);
  //  resizeRowsToContents();
  qDebug("setRootPath[%s]", qPrintable(rootPath));
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QMainWindow>
#include "View/SceneActionsSubscribe.h"
#include "Actions/SceneInPageActions.h"
#include <QToolBar>

class BearingWidget : public QMainWindow {
 public:
  QToolBar* mSceneTB{nullptr};
  QLineEdit* m_filterInput{nullptr};

  ScenesTableModel* m_model{nullptr};
  SceneTableView* m_tableView{nullptr};

  SceneActionsSubscribe* sceneSub{nullptr};

  BearingWidget(QWidget* parent = nullptr) : QMainWindow{parent} {
    setWindowTitle("BearingWidget");
    setMinimumSize(1780, 768);

    m_filterInput = new QLineEdit("", parent);
    mSceneTB = g_SceneInPageActions().GetSceneToolbar();
    mSceneTB->addAction("Filter:");
    mSceneTB->addWidget(m_filterInput);

    addToolBar(Qt::ToolBarArea::TopToolBarArea, mSceneTB);

    m_model = new ScenesTableModel;
    m_tableView = new SceneTableView(m_model, this);
    setCentralWidget(m_tableView);

    if (sceneSub->BindWidget(m_tableView, m_model)) {
      sceneSub->operator()();
      connect(m_filterInput, &QLineEdit::textChanged, m_model, &ScenesTableModel::setFilterRegularExpression);
    }

    IntoANewPath("E:/115/MEN/Page120");
  }

  void IntoANewPath(const QString& path) {
    m_model->setRootPath(path);
    //    m_tableView->resizeColumnsToContents();
  }
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  BearingWidget sceneWid;
  sceneWid.show();
  return a.exec();
}
#endif
