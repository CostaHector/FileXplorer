#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
class ViewActions : public QObject {
  Q_OBJECT
 public:
  explicit ViewActions(QObject* parent = nullptr);

  QToolBar* GetViewTB(QWidget* parent = nullptr);

  QAction* _ADVANCE_SEARCH_VIEW{nullptr};
  QAction* _MOVIE_VIEW{nullptr};
  QAction* _LIST_VIEW{nullptr};
  QAction* _TABLE_VIEW{nullptr};
  QAction* _TREE_VIEW{nullptr};
  QAction* _SCENE_VIEW{nullptr};
  QAction* _FLOATING_PREVIEW{nullptr};
  QAction* _CAST_VIEW{nullptr};
  QAction* _JSON_VIEW{nullptr};
  QActionGroup* _VIEWS_AG{nullptr};

  QAction* NAVIGATION_PANE{nullptr};
  QAction* _JSON_EDITOR_PANE{nullptr};
  QAction* _VIDEO_PLAYER_EMBEDDED{nullptr};
  QActionGroup* _VIEW_ACTIONS{nullptr};

  QAction* _SYS_VIDEO_PLAYERS{nullptr};
  QActionGroup* _VIDEO_PLAYERS{nullptr};

  QAction* _HAR_VIEW{nullptr};

 private:
  QActionGroup* Get_NAVIGATION_PANE_Actions();
  QActionGroup* GetPlayersActions();
  QActionGroup* GetViewsAG();
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
