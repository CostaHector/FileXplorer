#ifndef SCENEINPAGEACTIONS_H
#define SCENEINPAGEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class QToolBar;

class SceneInPageActions : public QObject {
 public:
  explicit SceneInPageActions(QObject* parent = nullptr);

  QToolBar* GetOrderToolBar(QWidget* parent);

  QAction* _COMBINE_MEDIAINFOS_JSON{nullptr};

  QAction* _BY_MOVIE_NAME{nullptr};
  QAction* _BY_MOVIE_SIZE{nullptr};
  QAction* _BY_RATE{nullptr};
  QAction* _BY_UPLOADED_TIME{nullptr};
  QAction* _REVERSE_SORT{nullptr};
  QActionGroup* _ORDER_AG{nullptr};

  // order | page action
  // order | ----------
  // order | line edit
  // order | ----------
  // order | page select
};

SceneInPageActions& g_SceneInPageActions();

#endif  // SCENEINPAGEACTIONS_H
