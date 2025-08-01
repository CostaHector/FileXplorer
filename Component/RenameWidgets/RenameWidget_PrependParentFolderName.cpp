#include "RenameWidget_PrependParentFolderName.h"
#include "public/PublicMacro.h"
#include "Tools/RenameHelper.h"

RenameWidget_PrependParentFolderName::RenameWidget_PrependParentFolderName(QWidget* parent)  //
  : AdvanceRenamer{parent} {
  m_recursiveCB->setEnabled(false);
  m_recursiveCB->setChecked(true);
}
void RenameWidget_PrependParentFolderName::InitExtraCommonVariable() {
  windowTitleFormat = "Prepend parend folder names | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(""));
}

QToolBar* RenameWidget_PrependParentFolderName::InitControlTB() {
  QToolBar* prependTb{new (std::nothrow) QToolBar{"PrependParentFolderNamesTB", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(prependTb);
  prependTb->addSeparator();
  prependTb->addWidget(m_recursiveCB);
  return prependTb;
}

void RenameWidget_PrependParentFolderName::extraSubscribe() {}

void RenameWidget_PrependParentFolderName::InitExtraMemberWidget() {}

QStringList RenameWidget_PrependParentFolderName::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty() || mRelToNameWithNoRoot.isEmpty()) {
    return replaceeList;
  }
  const QStringList& suffixs = m_oExtTE->toPlainText().split(NAME_SEP);
  return RenameHelper::PrependParentFolderNameToFileName(mRelToNameWithNoRoot, replaceeList, suffixs);
}
