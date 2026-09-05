#include "RibbonScene.h"
#include "ScenePageControl.h"
#include "SceneInPageActions.h"
#include "ViewActions.h"

RibbonScene::RibbonScene(const QString& title, QWidget* parent) //
  : QToolBar{title, parent}                                   //
{
  auto& ag = SceneInPageActions::GetInst();
  addAction(ViewActions::GetInst()._SCENE_VIEW);
  addSeparator();

  addAction(ag._UPDATE_JSON);
  addAction(ag._UPDATE_SCN);
  addSeparator();

  addAction(ag._DISABLE_IMAGE_DECORATION);
  addAction(ag._INCLUDEING_SUBDIRECTORIES);
  addSeparator();

  addAction(ag._CLEAR_SCN_FILE);
  addSeparator();

  QWidget* orderTB = ag.GetOrderToolBar(this);
  addWidget(orderTB);
  addSeparator();

  addAction(ag._ARCHIVE_BY_MOVIE_SCORE);
  addActions(ag._ARCHIVE_AG->actions());
  addSeparator();

  if (auto* p = ScenePageControl::GInstance()) {
    addWidget(p);
  }

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}