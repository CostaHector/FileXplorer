#include "SceneInPageActions.h"
#include <QLineEdit>
#include <QToolBar>
#include <QLabel>
#include <QIntValidator>

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent) : QObject{parent} {
  _ASCENDING = new QAction(QIcon(":/themes/ASCENDING_ORDER"), "Ascending", this);
  _ASCENDING->setCheckable(true);
  _DESCENDING = new QAction(QIcon(":/themes/DESCENDING_ORDER"), "Descending", this);
  _DESCENDING->setCheckable(true);
  _ORDER_AG = new QActionGroup(this);
  _ORDER_AG->addAction(_ASCENDING);
  _ORDER_AG->addAction(_DESCENDING);
  _ORDER_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

  _GROUP_BY_PAGE = new QAction("Enable Group by page:\nrow x column", this);
  _GROUP_BY_PAGE->setCheckable(true);
  _THE_LAST_PAGE = new QAction("The Last>>", this);
  _LAST_PAGE = new QAction("Last<", this);
  _NEXT_PAGE = new QAction("Next>", this);
  _THE_FIRST_PAGE = new QAction("The First<<", this);
}

QToolBar* SceneInPageActions::GetSceneToolbar() {
  mRowsInputLE = new QLineEdit("5");
  mColumnsInputLE = new QLineEdit("4");
  mRowsInputLE->setMaximumWidth(80);
  mColumnsInputLE->setMaximumWidth(80);

  mRowsInputLE->setValidator(new QIntValidator{1, 10000});
  mColumnsInputLE->setValidator(new QIntValidator{1, 10000});

  mPageIndexInputLE = new QLineEdit("0");
  mPageIndexInputLE->setMaximumWidth(80);
  mPageIndexInputLE->setValidator(new QIntValidator{-1, 10000});

  mOrderTB = new QToolBar("Scene Order");
  mOrderTB->addActions(_ORDER_AG->actions());
  mOrderTB->setOrientation(Qt::Orientation::Vertical);
  mOrderTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);

  mRowByColumnTB = new QToolBar("Page Row-by-Column");
  mRowByColumnTB->addWidget(mRowsInputLE);
  mRowByColumnTB->addWidget(new QLabel("-by-"));
  mRowByColumnTB->addWidget(mColumnsInputLE);

  mEnablePageTB = new QToolBar{"Enable Page"};
  mEnablePageTB->addAction(_GROUP_BY_PAGE);
  mEnablePageTB->addWidget(mRowByColumnTB);
  mEnablePageTB->setOrientation(Qt::Orientation::Vertical);

  mPagesSelectTB = new QToolBar("Page Select");
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addActions({_THE_FIRST_PAGE, _LAST_PAGE});
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addWidget(mPageIndexInputLE);
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addActions({_NEXT_PAGE, _THE_LAST_PAGE});

  mSceneTB = new QToolBar("scene toolbar");
  mSceneTB->addWidget(mOrderTB);
  mSceneTB->addSeparator();
  mSceneTB->addWidget(mEnablePageTB);
  mSceneTB->addSeparator();
  mSceneTB->addWidget(mPagesSelectTB);
  return mSceneTB;
}
