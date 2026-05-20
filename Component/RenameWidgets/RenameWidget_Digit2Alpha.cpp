#include "RenameWidget_Digit2Alpha.h"
#include "PublicMacro.h"
#include "RenameHelper.h"
#include "SpacerWidget.h"

void RenameWidget_Digit2Alpha::InitExtraCommonVariable() {
  windowTitleFormat = "Replace digit 2 alpha | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon{":img/DIGIT_2_ALPHA"});
}

QToolBar* RenameWidget_Digit2Alpha::InitControlTB() {
  QToolBar* prependTb{new (std::nothrow) QToolBar{"ReplaceDigit2AlphaToolBar", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(prependTb);
  auto* pSpacer = SpacerWidget::GetSpacerWidget(prependTb, Qt::Orientation::Horizontal);
  prependTb->addWidget(pSpacer);
  prependTb->addWidget(m_nameExtIndependent);
  prependTb->addWidget(m_recursiveCB);
  prependTb->addWidget(regexValidLabel);
  return prependTb;
}

QStringList RenameWidget_Digit2Alpha::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  return RenameHelper::SpecialDigitReplace2Char(replaceeList);
}
