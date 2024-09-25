#include "SceneTableView.h"
#include "Model/ScenesTableModel.h"
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QMessageBox>

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

  horizontalHeader()->setDefaultSectionSize(600 * 3 / 4);
  horizontalHeader()->setVisible(true);
  verticalHeader()->setDefaultSectionSize(337 * 3 / 4 + 50);
  horizontalHeader()->setStretchLastSection(true);

  mAlignDelegate = new AlignDelegate;
  setItemDelegate(mAlignDelegate);

  //  BindMenu(g_performersManagerActions().GetRightClickMenu());
  //  AppendVerticalHeaderMenuAGS(g_performersManagerActions().GetVerAGS());
  //  AppendHorizontalHeaderMenuAGS(g_performersManagerActions().GetHorAGS());
}

void SceneTableView::setRootPath(const QString& rootPath) {
  if (rootPath.count('/') < 3) {
    qDebug("rootPath[%s] may contains a large item(s)", qPrintable(rootPath));
    const auto ret = QMessageBox::warning(this, "Large folder alert(May cause LAG)", rootPath,
                                          QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::No);
    if (ret != QMessageBox::StandardButton::Yes) {
      qDebug("User cancel setRootPath on a large path[%s]", qPrintable(rootPath));
      return;
    }
  }
  _sceneModel->setRootPath(rootPath);
}

#include <QMainWindow>
#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QLabel>
#include <QLineEdit>
#include "Actions/SceneInPageActions.h"

class BearingWidget : public QMainWindow {
 public:
  QToolBar* mSceneTB{nullptr};
  QLineEdit* filterInput{nullptr};

  ScenesTableModel* model{nullptr};
  SceneTableView* tb{nullptr};

  BearingWidget(QWidget* parent = nullptr) : QMainWindow{parent} {
    setWindowTitle("BearingWidget");
    setMinimumSize(1780, 768);

    filterInput = new QLineEdit("", parent);
    mSceneTB = g_SceneInPageActions().GetSceneToolbar();
    mSceneTB->addSeparator();
    mSceneTB->addWidget(new QLabel{"Filter:"});
    mSceneTB->addWidget(filterInput);

    addToolBar(Qt::ToolBarArea::TopToolBarArea, mSceneTB);

    model = new ScenesTableModel;
    tb = new SceneTableView(model, this);
    setCentralWidget(tb);

    subscribe();
    IntoANewPath("E:/115/MEN/Page120");
  }

  void subscribe() {
    auto& ags = g_SceneInPageActions();
    connect(ags._ORDER_AG, &QActionGroup::triggered, this, &BearingWidget::SortIt);
    connect(ags._GROUP_BY_PAGE, &QAction::triggered, this, &BearingWidget::SetScenesGroupByPage);
    connect(ags.mRowsInputLE, &QLineEdit::textChanged, this, &BearingWidget::SetScenesPerColumn);
    connect(ags.mPageIndexInputLE, &QLineEdit::textChanged, this, &BearingWidget::SetPageIndex);
    connect(ags.mColumnsInputLE, &QLineEdit::textChanged, this, &BearingWidget::SetScenesPerRow);
    connect(filterInput, &QLineEdit::textChanged, this->model, &ScenesTableModel::setFilterRegExp);
    connect(ags.mPagesSelectTB, &QToolBar::actionTriggered, this, &BearingWidget::PageIndexIncDec);

    ags._GROUP_BY_PAGE->setChecked(false);
    emit ags._GROUP_BY_PAGE->triggered(false);
  }

  bool PageIndexIncDec(const QAction* pageAct) {
    auto& ags = g_SceneInPageActions();
    QString beforeIndexStr = ags.mPageIndexInputLE->text();
    bool isNumber{false};
    int beforePageInd = beforeIndexStr.toInt(&isNumber);
    if (not isNumber) {
      qDebug("Error before index");
      return false;
    }
    int dstPageInd = beforePageInd;
    int maxPage = model->GetPageCnt();
    if (pageAct == ags._NEXT_PAGE) {
      ++dstPageInd;
      dstPageInd %= maxPage;
    } else if (pageAct == ags._LAST_PAGE) {
      --dstPageInd;
      if (dstPageInd < 0) {
        dstPageInd = maxPage - 1;
      }
    } else if (pageAct == ags._THE_FIRST_PAGE) {
      dstPageInd = 0;
    } else if (pageAct == ags._THE_LAST_PAGE) {
      dstPageInd = maxPage - 1;
    } else {
      qDebug("nothing triggered");
      return false;
    }

    if (dstPageInd == beforePageInd) {
      qDebug("Page remains %d, ignore switch page", beforePageInd);
      return true;
    }
    qDebug("page index changed: %d->%d", beforePageInd, dstPageInd);
    ags.mPageIndexInputLE->setText(QString::number(dstPageInd));
    emit ags.mPageIndexInputLE->textChanged(ags.mPageIndexInputLE->text());
    return true;
  }

  void SetScenesGroupByPage(bool groupByPageAction) {
    auto& ags = g_SceneInPageActions();
    ags.mPagesSelectTB->setEnabled(groupByPageAction);
    if (groupByPageAction) {
      SetScenesPerColumn();
    }
  }

  void SetPageIndex() {
    auto& ags = g_SceneInPageActions();
    const QString& pageIndStr = ags.mPageIndexInputLE->text();
    bool isNumber{false};
    int pageIndex = pageIndStr.toInt(&isNumber);
    if (not isNumber) {
      qDebug("Page Index str[%s] invalid", qPrintable(pageIndStr));
      return;
    }
    model->SetPageIndex(pageIndex);
  }

  bool SetScenesPerColumn() {
    auto& ags = g_SceneInPageActions();
    const QString& rowCntStr = ags.mRowsInputLE->text();
    const QString& pageIndStr = ags.mPageIndexInputLE->text();
    bool isRowCntNumber{false}, isPageIndNumber{false};
    const int rowCnt = rowCntStr.toInt(&isRowCntNumber);
    const int pageInd = pageIndStr.toInt(&isPageIndNumber);
    if (not isRowCntNumber) {
      qDebug("Row Count str[%s] invalid", qPrintable(rowCntStr));
      return false;
    }
    if (not isPageIndNumber) {
      qDebug("Page Index str[%s] invalid", qPrintable(pageIndStr));
      return false;
    }
    model->ChangeRowsCnt(rowCnt, pageInd);
    return true;
  }

  bool SetScenesPerRow() {
    auto& ags = g_SceneInPageActions();
    const QString& columnCntStr = ags.mColumnsInputLE->text();
    const QString& pageIndStr = ags.mPageIndexInputLE->text();
    bool isColCntNumber{false}, isPageIndNumber{false};
    const int colCnt = columnCntStr.toInt(&isColCntNumber);
    const int pageInd = pageIndStr.toInt(&isPageIndNumber);
    if (not isColCntNumber) {
      qDebug("Column Count str[%s] invalid", qPrintable(columnCntStr));
      return false;
    }
    if (not isPageIndNumber) {
      qDebug("Page Index str[%s] invalid", qPrintable(pageIndStr));
      return false;
    }
    model->ChangeColumnsCnt(colCnt, pageInd);
    return true;
  }

  void SortIt(QAction* triggerAct) {
    if (triggerAct == nullptr) {
      qWarning("triggerAct is nullptr");
      return;
    }
    model->SortOrder(triggerAct->text() == "Descending");
  }

  void IntoANewPath(const QString& path) {
    model->setRootPath(path);
    //    tb->resizeColumnsToContents();
  }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QMainWindow>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  BearingWidget sceneWid;
  sceneWid.show();
  return a.exec();
}
#endif
