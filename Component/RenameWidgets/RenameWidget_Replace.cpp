#include "RenameWidget_Replace.h"
#include "Tools/RenameHelper.h"
#include "PublicVariable.h"

RenameWidget_Replace::RenameWidget_Replace(QWidget* parent)  //
    : AdvanceRenamer(parent), oldStrCB(new QComboBox), newStrCB(new QComboBox), regex(new QCheckBox("Regex")) {}

QToolBar* RenameWidget_Replace::InitControlTB() {
  QToolBar* replaceControl = new QToolBar;
  replaceControl->addWidget(new QLabel("Old:"));
  replaceControl->addWidget(oldStrCB);
  replaceControl->addWidget(new QLabel("New:"));
  replaceControl->addWidget(newStrCB);
  replaceControl->addSeparator();
  replaceControl->addWidget(regex);
  replaceControl->addWidget(EXT_INSIDE_FILENAME);
  replaceControl->addWidget(ITEMS_INSIDE_SUBDIR);
  replaceControl->addSeparator();
  replaceControl->addWidget(regexValidLabel);
  return replaceControl;
}
void RenameWidget_Replace::extraSubscribe() {
  connect(oldStrCB, &QComboBox::currentTextChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
  connect(regex, &QCheckBox::stateChanged, this, &RenameWidget_Replace::onRegex);
  connect(newStrCB, &QComboBox::currentTextChanged, this, &RenameWidget_Replace::OnlyTriggerRenameCore);
}

auto RenameWidget_Replace::InitExtraMemberWidget() -> void {
  oldStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_OLD_STR_LIST.name, MemoryKey::RENAMER_OLD_STR_LIST.v).toStringList());
  oldStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  oldStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  oldStrCB->setEditable(true);
  oldStrCB->setCompleter(nullptr);  // block auto complete

  newStrCB->addItems(PreferenceSettings().value(MemoryKey::RENAMER_NEW_STR_LIST.name, MemoryKey::RENAMER_NEW_STR_LIST.v).toStringList());
  newStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  newStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
  newStrCB->setEditable(true);
  newStrCB->setCompleter(nullptr);

  regex->setToolTip("Regular expression enable switch");
}

QStringList RenameWidget_Replace::RenameCore(const QStringList& replaceeList) {
  const QString& oldString = oldStrCB->currentText();
  const QString& newString = newStrCB->currentText();
  const bool regexEnable = regex->isChecked();
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

void RenameWidget_Delete::InitExtraCommonVariable() {
  windowTitleFormat = "Delete name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_DELETER_PATH"));
}

RenameWidget_Delete::RenameWidget_Delete(QWidget* parent)  //
    : RenameWidget_Replace(parent) {
  newStrCB->setCurrentText("");
  newStrCB->setEnabled(false);
  newStrCB->setToolTip("New str is identically equal to empty str");
}
