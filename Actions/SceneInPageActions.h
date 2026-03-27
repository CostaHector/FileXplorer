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

  QAction* _REVERSE_ORDER{nullptr};

  QAction* _SORT_RANGE_CURRENT_PAGE{nullptr};

  SceneSortOrderHelper::SortDimE GetSortDimension() const {
    return mSortOrderIntAction.curVal();
  }
  bool GetSortOrderReverse() const {
    return _REVERSE_ORDER != nullptr && _REVERSE_ORDER->isChecked();
  }
  bool GetSortRangeCurrentPageOnly() const {
    return _SORT_RANGE_CURRENT_PAGE != nullptr && _SORT_RANGE_CURRENT_PAGE->isChecked();
  }

 signals:
  void sceneSortDimensionChanged(SceneSortOrderHelper::SortDimE newSortDimension);
  void sceneSortReverseOrderChanged(bool bReverse);

  // used in proxy model only
  void scenesSortPolicyChanged(SceneSortOrderHelper::SortDimE sortDimension, Qt::SortOrder order);

 private:
  void onReverseSortOrderToggled(bool bReverseDescend);
  void onSortDimensionTriggered(QAction* triggeredAct);
  void subscribe();
  EnumIntAction<SceneSortOrderHelper::SortDimE> mSortOrderIntAction;
};

SceneInPageActions& g_SceneInPageActions();

#endif  // SCENEINPAGEACTIONS_H
