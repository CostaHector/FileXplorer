#include "RenameWidget_Replace.h"
#include "Tools/RenameHelper.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"

RenameWidget_Replace::RenameWidget_Replace(QWidget* parent)  //
    : AdvanceRenamer(parent) {
  m_oldStrCB = new QComboBox;
  m_newStrCB = new QComboBox;  //
  m_regexCB = new QCheckBox("Regex");
}

QToolBar* RenameWidget_Replace::InitControlTB() {
  QToolBar* replaceControl = new QToolBar;
  replaceControl->addWidget(new QLabel("Old:"));
  replaceControl->addWidget(m_oldStrCB);
  replaceControl->addWidget(new QLabel("New:"));
  replaceControl->addWidget(m_newStrCB);
  replaceControl->addSeparator();
  replaceControl->addWidget(m_regexCB);
  replaceControl->addWidget(m_extensionInNameCB);
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
  m_oldStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_OLD_STR_LIST.name, MemoryKey::RENAMER_OLD_STR_LIST.v).toStringList());
  m_oldStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_oldStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_oldStrCB->setEditable(true);
  m_oldStrCB->setCompleter(nullptr);  // block auto complete

  m_newStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_NEW_STR_LIST.name, MemoryKey::RENAMER_NEW_STR_LIST.v).toStringList());
  m_newStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_newStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  m_newStrCB->setEditable(true);
  m_newStrCB->setCompleter(nullptr);

  m_regexCB->setToolTip("Regular expression enable switch");
}

QStringList RenameWidget_Replace::RenameCore(const QStringList& replaceeList) {
  const QString& oldString = m_oldStrCB->currentText();
  const QString& newString = m_newStrCB->currentText();
  const bool regexEnable = m_regexCB->isChecked();
  if (regexEnable) {
    QRegularExpression repRegex(oldString);
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
  m_newStrCB->setCurrentText("");
  m_newStrCB->setEnabled(false);
  m_newStrCB->setToolTip("New str is identically equal to empty str");
}

void RenameWidget_Delete::InitExtraCommonVariable() {
  windowTitleFormat = "Delete name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_DELETER_PATH"));
}
