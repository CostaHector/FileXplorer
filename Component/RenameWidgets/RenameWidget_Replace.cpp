#include "RenameWidget_Replace.h"
#include "RenameHelper.h"
#include "RenamerKey.h"
#include "Configuration.h"
#include "PublicMacro.h"
#include "PathTool.h"

#include <QLineEdit>
#include <algorithm>

#define OLD_STR_DEFAULT_MODE_MAPPING \
OLD_STR_DEFAULT_MODE_ITEM(KEEP_EMPTY, 0, "keep empty")\
    OLD_STR_DEFAULT_MODE_ITEM(LONGEST_COMMON_PREFIX, 1, "longest common prefix")\
    OLD_STR_DEFAULT_MODE_ITEM(SHORTEST_ONE, 2, "shortest one")\
    OLD_STR_DEFAULT_MODE_ITEM(LONGEST_ONE, 3, "longest one")\

    enum OldStrDefaultMode{
      OLD_STR_DEFAULT_MODE_BEGIN,
#define OLD_STR_DEFAULT_MODE_ITEM(enumName, enumVal, enumStr) OLD_STR_DEFAULT_MODE_##enumName = enumVal,
      OLD_STR_DEFAULT_MODE_MAPPING
#undef OLD_STR_DEFAULT_MODE_ITEM
          OLD_STR_DEFAULT_MODE_BUTT
    };

bool isOldStrDefaultModeValid(int oldStrDefaultMode) {
  return OLD_STR_DEFAULT_MODE_BEGIN <= oldStrDefaultMode && oldStrDefaultMode < OLD_STR_DEFAULT_MODE_BUTT;
}

OldStrDefaultMode GetOldStrDefaultModeFromMemory() {
  bool bOk{false};
  int oldStrDefaultMode = getConfig(RenamerKey::OLD_STR_DEFAULT_MODE_SELECT).toInt(&bOk);
  if (!bOk || !isOldStrDefaultModeValid(oldStrDefaultMode)) {
    return OLD_STR_DEFAULT_MODE_BEGIN;
  }
  return static_cast<OldStrDefaultMode>(oldStrDefaultMode);;
}

RenameWidget_Replace::~RenameWidget_Replace() {
  Configuration().setValue(RenamerKey::OLD_STR_DEFAULT_MODE_SELECT.name, m_oldStrDefaultModeCB->currentIndex());
}

QToolBar* RenameWidget_Replace::InitControlTB() {
  QToolBar* replaceControl = new (std::nothrow) QToolBar{"replace tb", this};
  CHECK_NULLPTR_RETURN_NULLPTR(replaceControl);
  auto* pOldLabel = new (std::nothrow) QLabel{tr("Old:"), replaceControl};
  CHECK_NULLPTR_RETURN_NULLPTR(pOldLabel);
  auto* pNewLabel = new (std::nothrow) QLabel{tr("New:"), replaceControl};
  CHECK_NULLPTR_RETURN_NULLPTR(pNewLabel);

  replaceControl->addWidget(pOldLabel);
  replaceControl->addWidget(m_oldStrCB);
  replaceControl->addWidget(pNewLabel);
  replaceControl->addWidget(m_newStrCB);
  replaceControl->addSeparator();
  replaceControl->addWidget(m_oldStrDefaultModeCB);
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

QString GetOldStrDefault(const QStringList& oldStrList, OldStrDefaultMode oldStrDefaultMode) {
  switch (oldStrDefaultMode) {
    case OLD_STR_DEFAULT_MODE_KEEP_EMPTY: return "";
    case OLD_STR_DEFAULT_MODE_LONGEST_COMMON_PREFIX: return PathTool::longestCommonPrefix(oldStrList);
    case OLD_STR_DEFAULT_MODE_SHORTEST_ONE:
      return oldStrList.isEmpty() ? "" : *std::min_element(oldStrList.begin(), oldStrList.end(),
          [](const QString& a, const QString& b) { return a.length() < b.length(); });
    case OLD_STR_DEFAULT_MODE_LONGEST_ONE:
      return oldStrList.isEmpty() ? "" : *std::max_element(oldStrList.begin(), oldStrList.end(),
          [](const QString& a, const QString& b) { return a.length() < b.length(); });
    default:
      return "";
  }
}

bool RenameWidget_Replace::UpdateOldStrCbCurrentText() {
  // return false when invalid or unchange
  int oldStrDefaultMode = m_oldStrDefaultModeCB->currentIndex();
  if (!isOldStrDefaultModeValid(oldStrDefaultMode)) {
    LOG_E("oldStrDefaultMode[%d] out of range", oldStrDefaultMode);
    return false;
  }
  OldStrDefaultMode defMode = static_cast<OldStrDefaultMode>(oldStrDefaultMode);
  const QString beforeOldName = m_oldStrCB->currentText();
  const QString afterOldName = GetOldStrDefault(mNames, defMode);
  if (afterOldName == beforeOldName) {
    return false;
  }
  m_oldStrCB->setCurrentText(afterOldName);
  return true;
}

void RenameWidget_Replace::onOldStrDefaultModeChanged(int oldStrDefaultMode) {
  if (!UpdateOldStrCbCurrentText()) {
    return;
  }
  OnlyTriggerRenameCore();
}

const QStringList& GetOldStrDefaultModeCandidatesList() {
  static const QStringList candidates{
#define OLD_STR_DEFAULT_MODE_ITEM(enumName, enumVal, enumStr) enumStr,
      OLD_STR_DEFAULT_MODE_MAPPING
#undef OLD_STR_DEFAULT_MODE_ITEM
  };
  return candidates;
}

void RenameWidget_Replace::InitExtraMemberWidget() {
  m_oldStrCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_oldStrCB)
  m_oldStrCB->addItems(MultiLineStr2StrList(RenamerKey::REP_OLD_STR_LIST));
  m_oldStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_oldStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  m_oldStrCB->setEditable(true);
  m_oldStrCB->setCompleter(nullptr);  // block auto complete

  m_newStrCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_newStrCB)
  m_newStrCB->addItems(MultiLineStr2StrList(RenamerKey::NEW_STR_LIST));
  m_newStrCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
  m_newStrCB->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
  m_newStrCB->setEditable(true);
  m_newStrCB->setCompleter(nullptr);

  m_oldStrDefaultModeCB = new (std::nothrow) QComboBox{this};
  {
    CHECK_NULLPTR_RETURN_VOID(m_oldStrDefaultModeCB)
    m_oldStrDefaultModeCB->addItems(GetOldStrDefaultModeCandidatesList());
    m_oldStrDefaultModeCB->setCurrentIndex(GetOldStrDefaultModeFromMemory());
    m_oldStrDefaultModeCB->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    m_oldStrDefaultModeCB->setSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);
    m_oldStrDefaultModeCB->setEditable(false);
  }

  m_regexCB = new (std::nothrow) QCheckBox{tr("Regex"), this};
  CHECK_NULLPTR_RETURN_VOID(m_regexCB)
  m_regexCB->setIcon(QIcon(":img/MATCH_REGEX"));
  m_regexCB->setToolTip("Enable regex");
  m_regexCB->setChecked(getConfig(RenamerKey::REGEX_ENABLED).toBool());
}

QStringList RenameWidget_Replace::RenameCore(const QStringList& replaceeList) {
  if (!m_oldStrInited) {
    // init lineedit only at first time. when lineedit editted by user. lineedit should not init
    m_oldStrInited = true;
    UpdateOldStrCbCurrentText();
    using CURRENT_INDEX_CHANGED_MEMBER_FUNC = void (QComboBox::*)(int);
    connect(m_oldStrDefaultModeCB, (CURRENT_INDEX_CHANGED_MEMBER_FUNC)&QComboBox::currentIndexChanged, this, &RenameWidget_Replace::onOldStrDefaultModeChanged);
  }
  const QString& oldString = m_oldStrCB->currentText();
  const QString& newString = m_newStrCB->currentText();
  const bool regexEnable = m_regexCB->isChecked();
  if (regexEnable) {
    QRegularExpression repRegex{oldString};
    if (repRegex.isValid()) {
      regexValidLabel->ToSaved();
    } else {
      regexValidLabel->ToNotSaved();
      LOG_W("regular expression invalid[%s]", qPrintable(oldString));
      return {};
    }
  }
  return RenameHelper::ReplaceRename(replaceeList, oldString, newString, regexEnable);
}

void RenameWidget_Replace::setOldLineEditDisabled(bool bDisabled) {
  m_oldStrCB->setDisabled(bDisabled);
}

void RenameWidget_Replace::setOldNameAndNewName(const QString& oldName, const QString& newName) {
  m_oldStrCB->setCurrentText(oldName);
  m_newStrCB->setCurrentText(newName);
}

void RenameWidget_Replace::InitExtraCommonVariable() {
  windowTitleFormat = QString("Replace name string | %1 item(s) under [%2]");
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_REPLACER_PATH"));
}

RenameWidget_Delete::RenameWidget_Delete(QWidget* parent)  //
    : RenameWidget_Replace(parent) {}

void RenameWidget_Delete::initExclusiveSetting() {
  m_newStrCB->setCurrentText("");
  m_newStrCB->setEnabled(false);
  m_newStrCB->setToolTip("New str is identically equal to empty str");
  regexValidLabel->setVisible(true);
}

void RenameWidget_Delete::InitExtraCommonVariable() {
  windowTitleFormat = "Delete name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_DELETER_PATH"));
}
