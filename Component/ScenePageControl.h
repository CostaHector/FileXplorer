#ifndef SCENEPAGECONTROL_H
#define SCENEPAGECONTROL_H

#include <QWidget>
#include <QSpinBox>
#include <QActionGroup>

class ScenePageControl : public QWidget {
  Q_OBJECT
public:
  static ScenePageControl* GInstance(ScenePageControl* pInitValue=nullptr, bool bReset=false);
  explicit ScenePageControl(QWidget *parent = nullptr);

  void subscribe();

  void SetPageIndex();
  bool onPageIndexFrontBackActTriggered(const QAction* pageAct);
signals:
  void currentPageIndexChanged(int newPageIndex);
  void maxScenesCountPerPageChanged(int newMaxScenesCountEachPage);
public slots:
  bool EmitScenesCountPerPageChanged(int scenesPerPageCnt);
  void onPagesCountChanged(int newPagesCount);

private:
  QAction* _THE_FRONT_PAGE{nullptr};
  QAction* _THE_BACK_PAGE{nullptr};
  QActionGroup* _PAGE_NAVI_AG{nullptr};

  QSpinBox* mScenesPerPageInputSB{nullptr};
  QSpinBox* mPageIndexInputSB{nullptr};
};
#endif  // SCENEPAGECONTROL_H
