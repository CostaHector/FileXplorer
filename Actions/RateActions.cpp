#include "RateActions.h"
#include "PublicMacro.h"
#include "RateHelper.h"

RateActions& RateActions::GetInst() {
  static RateActions inst;
  return inst;
}

RateActions::RateActions(QObject* parent)
  : QObject{parent} {
  RATE_AGS = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(RATE_AGS);
  RATE_AGS->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);

  for (int rate = RateHelper::MIN_V; rate < RateHelper::BUTT_V; ++rate) {
    QAction* pAct = new (std::nothrow) QAction{QIcon(RateHelper::GetRatePixmap(rate)), //
                                               QString::asprintf("Rate %d", rate),                 //
                                               this};
    CHECK_NULLPTR_RETURN_VOID(pAct);
    pAct->setData(rate);
    const int keypad = (rate == 10) ? Qt::Key_Plus : (Qt::Key_0 + rate);
    pAct->setShortcut(QKeySequence(Qt::ControlModifier | Qt::KeypadModifier | keypad));
    RATE_AGS->addAction(pAct);
    RATE_ACTIONS_LIST += pAct;
  }

  RATE_ACTIONS_LIST += nullptr;

  _RATE_RECURSIVELY = new (std::nothrow) QAction{QIcon{":img/LIKE_RECURSIVELY"}, "Rate Recusively", this};
  CHECK_NULLPTR_RETURN_VOID(_RATE_RECURSIVELY);
  RATE_ACTIONS_LIST += _RATE_RECURSIVELY;

  subscribe();
}

void RateActions::subscribe() {
  connect(RATE_AGS, &QActionGroup::triggered, this, &RateActions::onRateActionTriggered);
  connect(_RATE_RECURSIVELY, &QAction::triggered, this, &RateActions::MovieRateRecursivelyChanged);
}

void RateActions::onRateActionTriggered(QAction* pActTriggered) {
  bool bOk = false;
  int newRate = pActTriggered->data().toInt(&bOk);
  if (!bOk) {
    LOG_W("data property in QAction is not a number");
    return;
  }
  emit MovieRateChanged(newRate);
}
