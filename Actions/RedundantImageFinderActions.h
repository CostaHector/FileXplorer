#ifndef REDUNDANTIMAGEFINDERACTIONS_H
#define REDUNDANTIMAGEFINDERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include "EnumIntAction.h"
#include "DuplicateImageDetectionCriteria.h"

extern template struct EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE>;

class RedundantImageFinderActions : public QObject {
 public:
  explicit RedundantImageFinderActions(QObject* parent = nullptr);

  QAction* FIND_DUPLICATE_IMGS_BY_LIBRARY{nullptr};
  QAction* FIND_DUPLICATE_IMGS_IN_A_PATH{nullptr};
  DuplicateImageDetectionCriteria::DICriteriaE GetCurFindDupBy() const { return mDecideByIntAction.curVal(); }
  EnumIntAction<DuplicateImageDetectionCriteria::DICriteriaE> mDecideByIntAction;

  QAction* RECYLE_NOW{nullptr};
  QAction* INCLUDING_EMPTY_IMAGES{nullptr};
  QAction* OPEN_BENCHMARK_FOLDER{nullptr};
  QAction* RELOAD_BENCHMARK_LIB{nullptr};
};

RedundantImageFinderActions& g_redunImgFinderAg();

#endif  // REDUNDANTIMAGEFINDERACTIONS_H
