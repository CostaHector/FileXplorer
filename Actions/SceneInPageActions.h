#ifndef SCENEINPAGEACTIONS_H
#define SCENEINPAGEACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>
#include "EnumIntAction.h"
#include "SceneInfo.h"

extern template struct EnumIntAction<SceneInfo::Role>;

class SceneInPageActions : public QObject {
  Q_OBJECT
 public:
  static SceneInPageActions& GetInst();

  explicit SceneInPageActions(QObject* parent = nullptr);
  ~SceneInPageActions();

  QWidget* GetOrderToolBar(QWidget* parent);

  QAction* _UPDATE_JSON{nullptr};
  QAction* _UPDATE_SCN{nullptr};
  QAction* _CREATE_THUMBNAIL_FOR_JSON_RELATED_IMGS{nullptr};
  QAction* _DISABLE_IMAGE_DECORATION{nullptr};
  QAction* _INCLUDEING_SUBDIRECTORIES{nullptr};
  QAction* _CLEAR_SCN_FILE{nullptr};

  SceneInfo::Role GetSortRole() const {
    return mSortOrderIntAction.curVal();
  }
  bool GetSortOrderReverse() const {
    return _REVERSE_RESULT != nullptr && _REVERSE_RESULT->isChecked();
  }
  bool GetSortRangeCurrentPageOnly() const {
    return _SORT_RANGE_PAGE_BY_PAGE != nullptr && _SORT_RANGE_PAGE_BY_PAGE->isChecked();
  }

  bool GetbSubdirectories() const {
    return _INCLUDEING_SUBDIRECTORIES != nullptr && _INCLUDEING_SUBDIRECTORIES->isChecked();
  }

 signals:
  void disableImageDecorationChanged(bool bDisable);

  void sceneSortDimensionChanged(SceneInfo::Role newSortDimension);
  void sceneSortReverseOrderChanged(bool bReverse);

  void sortImplementerChanged(bool pageByPage);

 private:
  void onDisableImageDecorationToggled(bool bDisabled);
  void onReverseSortOrderToggled(bool bReverseDescend);
  void onSortDimensionTriggered(const QAction* triggeredAct);

  void subscribe();
  EnumIntAction<SceneInfo::Role> mSortOrderIntAction;

  QAction* _BY_NAME{nullptr};
  QAction* _BY_MOVIE_PATH{nullptr};
  QAction* _BY_MOVIE_SIZE{nullptr};
  QAction* _BY_RATE{nullptr};
  QAction* _BY_UPLOADED_TIME{nullptr};

  QAction* _REVERSE_RESULT{nullptr};

  QAction* _SORT_RANGE_PAGE_BY_PAGE{nullptr};
};

#endif  // SCENEINPAGEACTIONS_H
