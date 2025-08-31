#include "RenameWidget_Replace.h"
#include "RenameHelper.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

RenameWidget_Replace::RenameWidget_Replace(QWidget* parent)  //
    : AdvanceRenamer{parent}                                 //
{ }

QToolBar* RenameWidget_Replace::InitControlTB() {
  QToolBar* replaceControl = new (std::nothrow) QToolBar;
  CHECK_NULLPTR_RETURN_NULLPTR(replaceControl);
  auto* pOldLabel = new (std::nothrow) QLabel{"Old:", replaceControl};
  CHECK_NULLPTR_RETURN_NULLPTR(pOldLabel);
  auto* pNewLabel = new (std::nothrow) QLabel{"New:", replaceControl};
  CHECK_NULLPTR_RETURN_NULLPTR(pNewLabel);

  replaceControl->addWidget(pOldLabel);
  replaceControl->addWidget(m_oldStrCB);
  replaceControl->addWidget(pNewLabel);
  replaceControl->addWidget(m_newStrCB);
  replaceControl->addSeparator();
  replaceControl->addWidget(m_regexCB);
  replaceControl->addWidget(m_nameExtIndependent);
  replaceControl->addWidget(m_recursiveCB);
  replaceControl->addSeparator();
  replaceControl->addWidget(regexValidLabel);
  return replaceControl;
}

void RenameWidget_Replace::extraSubscribe() {
  connect(m_oldStrCB, &QComboBox::currentTextChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
  connect(m_regexCB, &QCheckBox::stateChanged, this, &RenameWidget_Replace::onRegex);
  connect(m_newStrCB, &QComboBox::currentTextChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
}

auto RenameWidget_Replace::InitExtraMemberWidget() -> void {
  m_oldStrCB = new (std::nothrow) QComboBox;
  CHECK_NULLPTR_RETURN_VOID(m_oldStrCB)
  m_oldStrCB->addItems(Configuration().value(MemoryKey::RENAMER_OLD_STR_LIST.name, MemoryKey::RENAMER_OLD_STR_LIST.v).toStringList());
  m_oldStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_oldStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  m_oldStrCB->setEditable(true);
  m_oldStrCB->setCompleter(nullptr);  // block auto complete

  m_newStrCB = new (std::nothrow) QComboBox;
  CHECK_NULLPTR_RETURN_VOID(m_newStrCB)
  m_newStrCB->addItems(Configuration().value(MemoryKey::RENAMER_NEW_STR_LIST.name, MemoryKey::RENAMER_NEW_STR_LIST.v).toStringList());
  m_newStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_newStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  m_newStrCB->setEditable(true);
  m_newStrCB->setCompleter(nullptr);

  m_regexCB = new (std::nothrow) QCheckBox{"Regex"};
  CHECK_NULLPTR_RETURN_VOID(m_regexCB)
  m_regexCB->setToolTip("Enable regex");
  m_regexCB->setChecked(Configuration().value(MemoryKey::RENAMER_REGEX_ENABLED.name, MemoryKey::RENAMER_REGEX_ENABLED.v).toBool());
}

QStringList RenameWidget_Replace::RenameCore(const QStringList& replaceeList) {
  const QString& oldString = m_oldStrCB->currentText();
  const QString& newString = m_newStrCB->currentText();
  const bool regexEnable = m_regexCB->isChecked();
  if (regexEnable) {
    QRegularExpression repRegex{oldString};
    if (repRegex.isValid()) {
      regexValidLabel->ToSaved();
    } else {
      regexValidLabel->ToNotSaved();
      qWarning("regular expression invalid[%s]", qPrintable(oldString));
      return {};
    }
  }
  return RenameHelper::ReplaceRename(replaceeList, oldString, newString, regexEnable);
}

void RenameWidget_Replace::InitExtraCommonVariable() {
  windowTitleFormat = QString("Replace name string | %1 item(s) under [%2]");
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_REPLACER_PATH"));
}

RenameWidget_Delete::RenameWidget_Delete(QWidget* parent)  //
    : RenameWidget_Replace(parent) {
}

void RenameWidget_Delete::initExclusiveSetting() {
  m_newStrCB->setCurrentText("");
  m_newStrCB->setEnabled(false);
  m_newStrCB->setToolTip("New str is identically equal to empty str");
}

void RenameWidget_Delete::InitExtraCommonVariable() {
  windowTitleFormat = "Delete name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_DELETER_PATH"));
}
