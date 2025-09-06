#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
class ViewActions : public QObject {
  Q_OBJECT
public:
  friend ViewActions& g_viewActions();
  QToolBar* GetViewTB(QWidget* parent = nullptr);

  QAction* _LIST_VIEW{nullptr};
  QAction* _TABLE_VIEW{nullptr};
  QAction* _TREE_VIEW{nullptr};
  QAction* _ADVANCE_SEARCH_VIEW{nullptr};
  QAction* _MOVIE_VIEW{nullptr};
  QAction* _SCENE_VIEW{nullptr};
  QAction* _CAST_VIEW{nullptr};
  QAction* _JSON_VIEW{nullptr};
  QAction* _VIEW_BACK_TO{nullptr};
  QAction* _VIEW_FORWARD_TO{nullptr};

  QAction* NAVIGATION_PANE{nullptr};
  QActionGroup* _NAVI_ACTIONS{nullptr};

  QAction* _SYS_VIDEO_PLAYERS{nullptr};
  QAction* _HAR_VIEW{nullptr};

private:
  explicit ViewActions(QObject* parent = nullptr);

  QActionGroup* Get_NAVIGATION_PANE_Actions();
  QActionGroup* GetPlayersActions();
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
