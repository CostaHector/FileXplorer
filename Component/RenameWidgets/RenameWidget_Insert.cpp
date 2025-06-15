#include "RenameWidget_Insert.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"
#include "Tools/RenameHelper.h"

RenameWidget_Insert::RenameWidget_Insert(QWidget* parent)  //
    : AdvanceRenamer(parent) {
  insertStrCB = new (std::nothrow) QComboBox;
  CHECK_NULLPTR_RETURN_VOID(insertStrCB)
  insertAtCB = new (std::nothrow) QComboBox;
  CHECK_NULLPTR_RETURN_VOID(insertAtCB)
}

void RenameWidget_Insert::extraSubscribe() {
  connect(insertAtCB, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(insertStrCB, &QComboBox::currentTextChanged, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}

void RenameWidget_Insert::InitExtraMemberWidget() {
  insertStrCB->setEditable(true);
  insertStrCB->setCompleter(nullptr);
  insertStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_OLD_STR_LIST.name, MemoryKey::RENAMER_OLD_STR_LIST.v).toStringList());
  insertStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  insertStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);

  insertAtCB->setEditable(true);
  insertAtCB->setCompleter(nullptr);
  insertAtCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_INSERT_INDEXES_LIST.name, MemoryKey::RENAMER_INSERT_INDEXES_LIST.v).toStringList());
  insertAtCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  insertAtCB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
}

QStringList RenameWidget_Insert::RenameCore(const QStringList& replaceeList) {
  const QString& insertString = insertStrCB->currentText();
  const QString& insertAtStr = insertAtCB->currentText();
  bool isnumeric = false;
  int insertAt = insertAtStr.toInt(&isnumeric);
  if (!isnumeric) {
    qCritical("Insert index[%s] must be a number", qPrintable(insertAtStr));
    return replaceeList;
  }
  return RenameHelper::InsertRename(replaceeList, insertString, insertAt);
}

void RenameWidget_Insert::InitExtraCommonVariable() {
  windowTitleFormat = QString("Insert name string | %1 item(s) under [%2]");
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_INSERTER_PATH"));
}

QToolBar* RenameWidget_Insert::InitControlTB() {
  QToolBar* insertControlTb{new (std::nothrow) QToolBar{"Insert string", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(insertControlTb);
  auto* insertStringLabel = new (std::nothrow) QLabel{"String:", insertControlTb};
  CHECK_NULLPTR_RETURN_NULLPTR(insertStringLabel);
  auto* insertAtLabel = new (std::nothrow) QLabel{"Index:", insertControlTb};
  CHECK_NULLPTR_RETURN_NULLPTR(insertAtLabel);

  insertControlTb->addWidget(insertStringLabel);
  insertControlTb->addWidget(insertStrCB);
  insertControlTb->addWidget(insertAtLabel);
  insertControlTb->addWidget(insertAtCB);
  insertControlTb->addSeparator();
  insertControlTb->addWidget(m_nameExtIndependent);
  insertControlTb->addWidget(m_recursiveCB);
  insertControlTb->addSeparator();
  insertControlTb->addWidget(regexValidLabel);
  return insertControlTb;
}
