#include "RenameWidget_ConsecutiveFileNo.h"
#include "Tools/ToConsecutiveFileNameNo.h"
#include "Component/Notificator.h"
#include "public/PublicMacro.h"

RenameWidget_ConsecutiveFileNo::RenameWidget_ConsecutiveFileNo(QWidget* parent)  //
    : AdvanceRenamer(parent)                                                     //
{
  m_recursiveCB->setEnabled(false);
}

QStringList RenameWidget_ConsecutiveFileNo::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  const QString& startNoStr = m_fileNoStartIndex->text();
  bool isnumeric = false;
  int startNo = startNoStr.toInt(&isnumeric);
  if (!isnumeric) {
    LOG_CRITICAL("Start no must be a number", startNoStr);
    return replaceeList;
  }
  return ToConsecutiveFileNameNo(startNo)(replaceeList);
}

void RenameWidget_ConsecutiveFileNo::InitExtraCommonVariable() {
  m_fileNoStartIndex = new (std::nothrow) QLineEdit{"0", this};
  CHECK_NULLPTR_RETURN_VOID(m_fileNoStartIndex)
  m_nameExtIndependent->setCheckState(Qt::CheckState::Checked);
  m_recursiveCB->setCheckState(Qt::CheckState::Unchecked);

  windowTitleFormat = "Consecutive file number | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
}

QToolBar* RenameWidget_ConsecutiveFileNo::InitControlTB() {
  QToolBar* consecutiveControlTb = new (std::nothrow) QToolBar{"Consecutive file no", this};
  CHECK_NULLPTR_RETURN_NULLPTR(consecutiveControlTb);
  auto* startNoLabel = new (std::nothrow) QLabel{"start no:", consecutiveControlTb};
  CHECK_NULLPTR_RETURN_NULLPTR(startNoLabel);
  consecutiveControlTb->addWidget(startNoLabel);
  consecutiveControlTb->addWidget(m_fileNoStartIndex);
  return consecutiveControlTb;
}

void RenameWidget_ConsecutiveFileNo::extraSubscribe() {  //
  connect(m_fileNoStartIndex, &QLineEdit::textEdited, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}
