#include "CustomStatusBar.h"
#include "PublicMacro.h"
#include "ViewActions.h"
#include <QStyle>

CustomStatusBar::CustomStatusBar(QWidget* parent)  //
  : QStatusBar{parent} {
  for (int labelIndex = ITEMS; labelIndex < BUTT; ++labelIndex) {
    auto* p = new QLabel{"", this};
    CHECK_NULLPTR_RETURN_VOID(p);
    mLabelsLst.push_back(p);

    const int stretch{labelIndex == MSG ? 1 : 0};
    addPermanentWidget(p, stretch);
  }
  mLabelsLst[MSG]->setObjectName("statusMessageLabel");

  m_viewsSwitcher = g_viewActions().GetViewTB(this); // right-down corner permanent widget
  CHECK_NULLPTR_RETURN_VOID(m_viewsSwitcher);
  addPermanentWidget(m_viewsSwitcher);

  setContentsMargins(0, 0, 0, 0);
}

void CustomStatusBar::onPathInfoChanged(const int count, const int index) {
  switch (index) {
    case ITEMS:
      mLabelsLst[ITEMS]->setText(QString("Total %1 item(s) |").arg(count));
      break;
    case SELECTED:
      mLabelsLst[SELECTED]->setText(QString("%1 selected |").arg(count));
      break;
    default:
      break;
  }
}

void CustomStatusBar::onMsgChanged(const QString& text, const STATUS_ALERT_LEVEL alertLvl) {
  mLabelsLst[MSG]->setProperty("alertLevel", (int)alertLvl);
  mLabelsLst[MSG]->style()->unpolish(mLabelsLst[MSG]);
  mLabelsLst[MSG]->style()->polish(mLabelsLst[MSG]);
  mLabelsLst[MSG]->setText(text);
}

