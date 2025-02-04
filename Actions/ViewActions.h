#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>

class ViewActions : public QObject {
  Q_OBJECT
 public:
  explicit ViewActions(QObject* parent = nullptr);

  QActionGroup* Get_NAVIGATION_PANE_Actions();

  QActionGroup* GetPlayersActions();

  QActionGroup* GetViewsAG();
  QAction *_ADVANCE_SEARCH_VIEW, *_MOVIE_VIEW, *_LIST_VIEW, *_TABLE_VIEW, *_TREE_VIEW, *_SCENE_VIEW;
  QActionGroup* _VIEWS_AG;

  QAction* NAVIGATION_PANE;
  QAction* PREVIEW_PANE_HTML;
  QAction* _JSON_EDITOR_PANE;
  QAction* _VIDEO_PLAYER_EMBEDDED;
  QActionGroup* _VIEW_ACTIONS;

  QAction* _SYS_VIDEO_PLAYERS = nullptr;
  QActionGroup* _VIDEO_PLAYERS;

  QAction* _HAR_VIEW = nullptr;
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
