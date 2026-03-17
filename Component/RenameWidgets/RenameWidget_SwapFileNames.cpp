#include "RenameWidget_SwapFileNames.h"
#include "PublicMacro.h"

RenameWidget_SwapFileNames::RenameWidget_SwapFileNames(QWidget* parent)  //
    : AdvanceRenamer(parent) {
}

void RenameWidget_SwapFileNames::initExclusiveSetting() {
  m_nameExtIndependent->setEnabled(false);
  m_nameExtIndependent->setChecked(false);
}

void RenameWidget_SwapFileNames::InitExtraCommonVariable() {
  windowTitleFormat = "Reverse file names | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
}

QToolBar* RenameWidget_SwapFileNames::InitControlTB() {
  QToolBar* reverseLstTb{new (std::nothrow) QToolBar{"Swap names list", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(reverseLstTb);
  auto* reverseLstLabel = new (std::nothrow) QLabel{tr("Swap 2 file names"), reverseLstTb};
  CHECK_NULLPTR_RETURN_NULLPTR(reverseLstLabel);
  reverseLstTb->addWidget(reverseLstLabel);
  reverseLstTb->addSeparator();
  reverseLstTb->addWidget(m_recursiveCB);
  reverseLstTb->addWidget(m_nameExtIndependent);
  return reverseLstTb;
}

QStringList RenameWidget_SwapFileNames::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  QStringList replacedList{replaceeList.crbegin(), replaceeList.crend()};
  for (auto& name : replacedList) {
    name += ' ';
  }
  return replacedList;
}
