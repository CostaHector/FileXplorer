#ifndef SCENEINPAGEACTIONS_H
#define SCENEINPAGEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include <QToolBar>
#include "EnumIntAction.h"
#include "ScenePageNaviHelper.h"

extern template struct EnumIntAction<SceneSortOrderHelper::SortDimE>;

class SceneInPageActions : public QObject {
  Q_OBJECT
 public:
  explicit SceneInPageActions(QObject* parent = nullptr);
  ~SceneInPageActions();

  QToolBar* GetOrderToolBar(QWidget* parent);

  QAction* _UPDATE_JSON{nullptr};
  QAction* _UPDATE_SCN{nullptr};
  QAction* _CLEAR_SCN_FILE{nullptr};

  QAction* _BY_MOVIE_PATH{nullptr};
  QAction* _BY_MOVIE_SIZE{nullptr};
  QAction* _BY_RATE{nullptr};
  QAction* _BY_UPLOADED_TIME{nullptr};
  QAction* _REVERSE_SORT{nullptr};

  std::pair<SceneSortOrderHelper::SortDimE, Qt::SortOrder> GetSortSetting() const;
 signals:
  void scenesSortPolicyChanged(SceneSortOrderHelper::SortDimE sortDimension, Qt::SortOrder order); // sortDimension is type of SceneSortOrderHelper::SortDimE
 private:
  void EmitScenesSortPolicyChangedSignal();
  void subscribe();
  EnumIntAction<SceneSortOrderHelper::SortDimE> mSortOrderIntAction;
};

SceneInPageActions& g_SceneInPageActions();

#endif  // SCENEINPAGEACTIONS_H
