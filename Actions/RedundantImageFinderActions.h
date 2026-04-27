#ifndef REDUNDANTIMAGEFINDERACTIONS_H
#define REDUNDANTIMAGEFINDERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include "EnumIntAction.h"
#include "DuplicateImagesHelper.h"

extern template struct EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE>;

class QToolBar;
class QWidget;

class RedundantImageFinderActions : public QObject {
  Q_OBJECT
public:
  explicit RedundantImageFinderActions(QObject* parent = nullptr);
  QWidget* GetSearchByToolButton(QWidget* parent);
  void AddRelatedAction(QToolBar* tb);
  DuplicateImageDetectionCriteria::DICriteriaE GetCurFindDupBy() const { return mDecideByIntAction.curVal(); }
  bool isIncludeEmptyImage() const { return INCLUDING_EMPTY_IMAGES != nullptr && INCLUDING_EMPTY_IMAGES->isChecked(); }
  bool isDiabledImageDecoration() const { return _DISABLE_IMAGE_DECORATION != nullptr && _DISABLE_IMAGE_DECORATION->isChecked(); }

  QAction* RECYLE_NOW{nullptr};
  QAction* INCLUDING_EMPTY_IMAGES{nullptr};

signals:
  void disableImageDecorationChanged(bool bDisabled);
  void findByChanged(DuplicateImageDetectionCriteria::DICriteriaE findBy);

private:
  void subscribe();
  void onDisableImageDecorationToggled(bool bDisabled);
  void onFindByActionTriggered(const QAction* findByAct);
  static bool onOpenBenchmarkFolder();

  QAction* OPEN_BENCHMARK_FOLDER{nullptr};
  QAction* RELOAD_BENCHMARK_LIB{nullptr};
  QAction* _DISABLE_IMAGE_DECORATION{nullptr};

  QAction* FIND_DUPLICATE_IMGS_BY_LIBRARY{nullptr};
  QAction* FIND_DUPLICATE_IMGS_IN_A_PATH{nullptr};
  EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE> mDecideByIntAction;
};

RedundantImageFinderActions& g_redunImgFinderAg();

#endif // REDUNDANTIMAGEFINDERACTIONS_H
