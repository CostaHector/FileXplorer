#include "ActionsContainerWid.h"
#include <QToolButton>

ActionsContainerWid::ActionsContainerWid(Qt::Orientation arrangeOrientation, int eachRankCnt, QWidget* parent)//
  : QWidget{parent}, mArrangeOrientation{arrangeOrientation}, mEachRankCnt{eachRankCnt <= 0 ? 1 : eachRankCnt} {
  mGridLo = new QGridLayout{this};
  mGridLo->setSpacing(0);
  mGridLo->setContentsMargins(0, 0, 0, 0);
}

void ActionsContainerWid::AddActions(const QList<QAction*>& acts, Qt::ToolButtonStyle tbs) {

  const int startDeviation = mGridLo->count();
  for (int i = 0; i < acts.size(); ++i) {
    QToolButton* btn = new QToolButton{this};
    btn->setDefaultAction(acts[i]);
    btn->setToolButtonStyle(tbs);
    btn->setAutoRaise(true);
    int eleIndex = i + startDeviation;
    if (mArrangeOrientation == Qt::Orientation::Horizontal) {
      mGridLo->addWidget(btn, eleIndex / mEachRankCnt, eleIndex % mEachRankCnt);
    } else {
      mGridLo->addWidget(btn, eleIndex % mEachRankCnt, eleIndex / mEachRankCnt);
    }
  }
}
