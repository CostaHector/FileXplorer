#include "RenameWidget_ReverseNames.h"
#include "Component/NotificatorFrame.h"
#include "PublicVariable.h"
#include "Tools/NameSectionArrange.h"

RenameWidget_ReverseNames::RenameWidget_ReverseNames(QWidget* parent)//
    : AdvanceRenamer(parent) {
  m_extensionInNameCB->setEnabled(false);
  m_extensionInNameCB->setChecked(false);
}
void RenameWidget_ReverseNames::InitExtraCommonVariable() {
  windowTitleFormat = "Reverse file names | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(""));
}
QToolBar* RenameWidget_ReverseNames::InitControlTB() {
  QToolBar* replaceControl(new QToolBar);
  replaceControl->addWidget(new QLabel("Reverse rename"));
  replaceControl->addSeparator();
  replaceControl->addWidget(m_recursiveCB);
  replaceControl->addWidget(m_extensionInNameCB);
  return replaceControl;
}

void RenameWidget_ReverseNames::extraSubscribe() {
}

void RenameWidget_ReverseNames::InitExtraMemberWidget() {
}

QStringList RenameWidget_ReverseNames::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  QStringList replacedList{replaceeList.crbegin(), replaceeList.crend()};
  for (auto& name: replacedList) {
    name += ' ';
  }
  return replacedList;
}
