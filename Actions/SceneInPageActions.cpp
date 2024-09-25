#include "SceneInPageActions.h"
#include <QLineEdit>
#include <QToolBar>
#include <QLabel>

SceneInPageActions& g_SceneInPageActions() {
  static SceneInPageActions ins;
  return ins;
}

SceneInPageActions::SceneInPageActions(QObject* parent) : QObject{parent} {
  _ASCENDING = new QAction("Ascending", this);
  _ASCENDING->setCheckable(true);
  _DESCENDING = new QAction("Descending", this);
  _DESCENDING->setCheckable(true);
  _ORDER_AG = new QActionGroup(this);
  _ORDER_AG->addAction(_ASCENDING);
  _ORDER_AG->addAction(_DESCENDING);
  _ORDER_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);

  _GROUP_BY_PAGE = new QAction("Rows:", this);
  _GROUP_BY_PAGE->setCheckable(true);
  _THE_LAST_PAGE = new QAction("The Last>>", this);
  _LAST_PAGE = new QAction("Last<", this);
  _NEXT_PAGE = new QAction("Next>", this);
  _THE_FIRST_PAGE = new QAction("The First<<", this);
}

QToolBar* SceneInPageActions::GetSceneToolbar() {
  mRowsInputLE = new QLineEdit("5");
  mPageIndexInputLE = new QLineEdit("0");
  mColumnsInputLE = new QLineEdit("4");
  mPagesSelectTB = new QToolBar("Page Select");
  mPagesSelectTB->addWidget(mRowsInputLE);
  mPagesSelectTB->addActions({_THE_FIRST_PAGE, _LAST_PAGE});
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addWidget(mPageIndexInputLE);
  mPagesSelectTB->addSeparator();
  mPagesSelectTB->addActions({_NEXT_PAGE, _THE_LAST_PAGE});

  mSceneTB = new QToolBar("scene toolbar");
  mSceneTB->addActions(_ORDER_AG->actions());
  mSceneTB->addSeparator();
  mSceneTB->addAction(_GROUP_BY_PAGE);
  mSceneTB->addWidget(mPagesSelectTB);
  mSceneTB->addSeparator();
  mSceneTB->addWidget(new QLabel("Columns:"));
  mSceneTB->addWidget(mColumnsInputLE);

  return mSceneTB;
}
