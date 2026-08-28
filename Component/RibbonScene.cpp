#include "RibbonScene.h"
#include "SceneInPageActions.h"
#include "JsonActions.h"
#include "ViewActions.h"
#include "ActionsContainerWid.h"

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

  m_scenePageControl = new (std::nothrow) ScenePageControl{"PaginationControl", this};
  addWidget(m_scenePageControl);
  addSeparator();

  ActionsContainerWid* tagsContainer = new ActionsContainerWid{Qt::Orientation::Vertical, 3, this};
  tagsContainer->AddActions(JsonActions::GetInst()._ADD_TAGS_ACTIONS_SCENE->actions(), Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  addWidget(tagsContainer);

  setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
}