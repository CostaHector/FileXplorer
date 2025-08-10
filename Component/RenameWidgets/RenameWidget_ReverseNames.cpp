#include "RenameWidget_ReverseNames.h"
#include "PublicMacro.h"

RenameWidget_ReverseNames::RenameWidget_ReverseNames(QWidget* parent)  //
    : AdvanceRenamer(parent) {
  m_nameExtIndependent->setEnabled(false);
  m_nameExtIndependent->setChecked(false);
}
void RenameWidget_ReverseNames::InitExtraCommonVariable() {
  windowTitleFormat = "Reverse file names | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(""));
}

QToolBar* RenameWidget_ReverseNames::InitControlTB() {
  QToolBar* reverseLstTb{new (std::nothrow) QToolBar{"Reverse names list", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(reverseLstTb);
  auto* reverseLstLabel = new (std::nothrow) QLabel{"Reverse rename", reverseLstTb};
  CHECK_NULLPTR_RETURN_NULLPTR(reverseLstLabel);
  reverseLstTb->addWidget(reverseLstLabel);
  reverseLstTb->addSeparator();
  reverseLstTb->addWidget(m_recursiveCB);
  reverseLstTb->addWidget(m_nameExtIndependent);
  return reverseLstTb;
}

void RenameWidget_ReverseNames::extraSubscribe() {}

void RenameWidget_ReverseNames::InitExtraMemberWidget() {}

QStringList RenameWidget_ReverseNames::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  QStringList replacedList{replaceeList.crbegin(), replaceeList.crend()};
  for (auto& name : replacedList) {
    name += ' ';
  }
  return replacedList;
}
