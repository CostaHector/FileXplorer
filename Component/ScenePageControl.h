#ifndef SCENEPAGECONTROL_H
#define SCENEPAGECONTROL_H

#include <QAction>
#include <QLineEdit>
#include <QToolBar>
#include "EnumIntAction.h"
#include "ScenePageNaviHelper.h"

extern template struct EnumIntAction<ScenePageNaviHelper::PageNaviE>;

class ScenePageControl: public QToolBar {
  Q_OBJECT
 public:
  explicit ScenePageControl(const QString &title = "Pagination display", QWidget *parent = nullptr);
  void subscribe();

  bool SetScenesCountPerPage();
  void SetPageIndex();
  bool PageIndexIncDec(const QAction* pageAct);
 signals:
  void currentPageIndexChanged(int newPageIndex);
  void maxScenesCountPerPageChanged(int newMaxScenesCountEachPage);
 public slots:
  void onPagesCountChanged(int newPagesCount) {
    mPagesCount = newPagesCount;
  }

 private:
  QAction* _THE_FRONT_PAGE{nullptr};
  QAction* _PREVIOUS_PAGE{nullptr};
  QAction* _NEXT_PAGE{nullptr};
  QAction* _THE_BACK_PAGE{nullptr};

  int mPagesCount {1};

  QLineEdit* mPageDimensionLE{nullptr};
  QLineEdit* mPageIndexInputLE{nullptr};
  QToolBar* mPagesSelectTB{nullptr};
  EnumIntAction<ScenePageNaviHelper::PageNaviE> mPageNaviIntAction;
};

#endif  // SCENEPAGECONTROL_H
