#ifndef VIEWACTIONS_H
#define VIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include <QList>

class ViewActions : public QObject {
  Q_OBJECT
public:
  friend ViewActions& g_viewActions();
  ~ViewActions();

  QAction* _NAVIGATION_PANE{nullptr};

  QAction* _LIST_VIEW{nullptr};
  QAction* _TABLE_VIEW{nullptr};
  QAction* _TREE_VIEW{nullptr};
  QAction* _ADVANCE_SEARCH_VIEW{nullptr};
  QAction* _MOVIE_VIEW{nullptr};
  QAction* _CAST_VIEW{nullptr};
  QAction* _SCENE_VIEW{nullptr};
  QAction* _JSON_VIEW{nullptr};
  QList<QAction*> _ALL_VIEWS;

  QAction* _VIEW_BACK_TO{nullptr};
  QAction* _VIEW_FORWARD_TO{nullptr};
  QList<QAction*> _VIEWS_NAVIGATE;

  QAction* _PREVIEW_PANEL{nullptr};

  QAction* _SYS_VIDEO_PLAYERS{nullptr};
  QAction* _HAR_VIEW{nullptr};

  QAction* _ROW_HEIGHT{nullptr};

private:
  explicit ViewActions(QObject* parent = nullptr);
};

ViewActions& g_viewActions();

#endif  // VIEWACTIONS_H
