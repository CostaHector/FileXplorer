#include "RenameWidget_ConsecutiveFileNo.h"
#include "Tools/ToConsecutiveFileNameNo.h"
#include "Component/NotificatorFrame.h"

RenameWidget_ConsecutiveFileNo::RenameWidget_ConsecutiveFileNo(QWidget* parent)  //
    : AdvanceRenamer(parent) {
  EXT_INSIDE_FILENAME->setCheckState(Qt::CheckState::Checked);
  ITEMS_INSIDE_SUBDIR->setEnabled(false);
  ITEMS_INSIDE_SUBDIR->setCheckState(Qt::CheckState::Unchecked);
};

QStringList RenameWidget_ConsecutiveFileNo::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  const QString& startNoStr = m_fileNoStartIndex->text();
  bool isnumeric = false;
  int startNo = startNoStr.toInt(&isnumeric);
  if (not isnumeric) {
    qCritical("start no[%s] must be a number", qPrintable(startNoStr));
    Notificator::critical("start no[%s] must be a number", startNoStr);
    return replaceeList;
  }
  return ToConsecutiveFileNameNo(startNo)(replaceeList);
}

void RenameWidget_ConsecutiveFileNo::InitExtraCommonVariable() {
  windowTitleFormat = QString("Consecutive file number | %1 item(s) under [%2]");
  setWindowTitle(windowTitleFormat);
}

QToolBar* RenameWidget_ConsecutiveFileNo::InitControlTB() {
  QToolBar* replaceControl = new QToolBar;
  replaceControl->addWidget(new QLabel("start no:"));
  replaceControl->addWidget(m_fileNoStartIndex);
  return replaceControl;
}

void RenameWidget_ConsecutiveFileNo::extraSubscribe() {  //
  connect(m_fileNoStartIndex, &QLineEdit::textEdited, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}
