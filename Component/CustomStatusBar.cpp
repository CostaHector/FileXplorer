#include "CustomStatusBar.h"
#include "PublicMacro.h"

CustomStatusBar::CustomStatusBar(QWidget* viewsSwitcherTb, QWidget* parent)  //
    : QStatusBar{parent} {
  mProcess = new (std::nothrow) QProgressBar{this};
  CHECK_NULLPTR_RETURN_VOID(mProcess);
  mProcess->setRange(0, 100);
  mProcess->setValue(0);

  for (int labelIndex = ITEMS; labelIndex < BUTT; ++labelIndex) {
    auto* p = new QLabel{"", parent};
    CHECK_NULLPTR_RETURN_VOID(p);
    mLabelsLst.push_back(p);
    const int stretch{labelIndex == MSG ? 1 : 0};
    addPermanentWidget(p, stretch);  // start=1, dev=0
  }

  addPermanentWidget(mProcess);
  if (viewsSwitcherTb != nullptr) {
    addPermanentWidget(viewsSwitcherTb);  // -1
  } else {
    qDebug("viewsSwitcherTb is nullptr");
  }
  setContentsMargins(0, 0, 0, 0);
}

void CustomStatusBar::pathInfo(const int count, const int index) {
  if (index == ITEMS) {
    mLabelsLst[ITEMS]->setText(QString("Total %1 item(s) |").arg(count));
  } else if (index == SELECTED) {
    mLabelsLst[SELECTED]->setText(QString("%1 selected |").arg(count));
  }
}

void CustomStatusBar::msg(const QString& text, const STATUS_STR_TYPE statusStrType) {
  if (statusStrType == STATUS_STR_TYPE::ABNORMAL) {  // abnormal
    mLabelsLst[MSG]->setStyleSheet("QLabel{color:red;font-weight:bold;}");
    qWarning("%s", qPrintable(text));
  } else {  // normal;
    mLabelsLst[MSG]->setStyleSheet("");
  }
  mLabelsLst[MSG]->setText(text);
}

void CustomStatusBar::SetProgressValue(int value) {
  mProcess->setValue(GetValidProgressValue(value));
}

int CustomStatusBar::GetValidProgressValue(int value) {
  return (value > 100) ? 100               //
                       : ((value < 0) ? 0  //
                                      : value);
}
