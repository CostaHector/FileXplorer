#include "ActionsContainerWid.h"
#include <QToolButton>

ActionsContainerWid::ActionsContainerWid(Qt::Orientation arrangeOrientation, int eachRankCnt, QWidget* parent)//
  : QScrollArea{parent}, mArrangeOrientation{arrangeOrientation}, mEachRankCnt{eachRankCnt <= 0 ? 1 : eachRankCnt} {
  QWidget* mWid = new QWidget{this};
  mGridLo = new QGridLayout{mWid};
  mGridLo->setSpacing(0);
  mGridLo->setContentsMargins(0, 0, 0, 0);

  setWidget(mWid);
  setWidgetResizable(true);
  setContentsMargins(0, 0, 0, 0);
  setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  setAlignment(Qt::AlignLeft);
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
      mGridLo->addWidget(btn, eleIndex / mEachRankCnt, eleIndex % mEachRankCnt, Qt::AlignLeft);
    } else {
      mGridLo->addWidget(btn, eleIndex % mEachRankCnt, eleIndex / mEachRankCnt, Qt::AlignLeft);
    }
  }
}
