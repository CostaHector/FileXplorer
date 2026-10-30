#include "RenameWidget_Case.h"
#include "NameTool.h"
#include "PublicMacro.h"
#include "Configuration.h"
#include "RenamerKey.h"

namespace {
enum CaseOperationType {
  NO_OPERATION = RenamerKey::NAME_CASE_CHECKED_INDEX.v.data.i, // -1
  UPPER_CASE = 0,
  LOWER_CASE,
  CAPITALIZE_KEEP_OTHER,
  CAPITALIZE_LOWER_OTHER,
  TOGGLE_CASE,
};

static_assert(CaseOperationType::NO_OPERATION == -1, "NO_OPERATION must be -1");

CaseOperationType EffectiveCaseOperation(const QAction* pCaseAct) {
  if (pCaseAct == nullptr) { // NO_OPERATION
    return NO_OPERATION;
  }
  bool bIsOk{false};
  int CaseOperationTypeInt = pCaseAct->property("CaseOperationType").toInt(&bIsOk);
  if (!bIsOk) {
    return NO_OPERATION;
  }
  if (!pCaseAct->isChecked()) {
    return NO_OPERATION;
  }
  return static_cast<CaseOperationType>(CaseOperationTypeInt);
}
}

QStringList RenameWidget_Case::RenameCore(const QStringList& replaceeList) {
  if (m_lowerCaseFileExtension->isChecked()) {
    m_nExtTE->setPlainText(NameTool::Lower(m_nExtTE->toPlainText()));
  } else {
    ResetNewExts();
  }

  QStringList ans;
  ans.reserve(replaceeList.size());
  const QAction* pCaseAct = m_nameCaseAg->checkedAction();
  const CaseOperationType caseOperation = EffectiveCaseOperation(pCaseAct);
  switch (caseOperation) {
    case UPPER_CASE:
      std::transform(replaceeList.cbegin(), replaceeList.cend(), std::back_inserter(ans), NameTool::Upper);
      break;
    case LOWER_CASE:
      std::transform(replaceeList.cbegin(), replaceeList.cend(), std::back_inserter(ans), NameTool::Lower);
      break;
    case CAPITALIZE_KEEP_OTHER:
      std::transform(replaceeList.cbegin(), replaceeList.cend(), std::back_inserter(ans), NameTool::CapitaliseFirstLetterKeepOther);
      break;
    case CAPITALIZE_LOWER_OTHER:
      std::transform(replaceeList.cbegin(), replaceeList.cend(), std::back_inserter(ans), NameTool::CapitaliseFirstLetterLowerOther);
      break;
    case TOGGLE_CASE:
      std::transform(replaceeList.cbegin(), replaceeList.cend(), std::back_inserter(ans), NameTool::ToggleSentenceCase);
      break;
    case NO_OPERATION:
    default:
      return replaceeList;
  }
  return ans;
}

void RenameWidget_Case::InitExtraCommonVariable() {
  windowTitleFormat = "Case name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_CASE"));
}

void RenameWidget_Case::InitExtraMemberWidget() {
  m_upperCase = new (std::nothrow) QAction(QIcon(":img/RENAME_UPPER_CASE"), tr("Uppercase"), this);
  m_upperCase->setToolTip("Convert all letters to uppercase.");
  m_upperCase->setCheckable(true);
  m_upperCase->setProperty("CaseOperationType", CaseOperationType::UPPER_CASE);

  m_lowerCase = new (std::nothrow) QAction(QIcon(":img/RENAME_LOWER_CASE"), tr("Lowercase"), this);
  m_lowerCase->setToolTip("Convert all letters to lowercase.");
  m_lowerCase->setCheckable(true);
  m_lowerCase->setProperty("CaseOperationType", CaseOperationType::LOWER_CASE);

  m_capitalizeKeepOther = new (std::nothrow) QAction(QIcon(":img/CAPITALIZE_KEEP_OTHER"), tr("Capitalize first and keep other"), this);
  m_capitalizeKeepOther->setToolTip(
      "Capitalize the first letter of each word; leave the rest unchanged.<br/>"
      "Example:<br/>"
      "i like NBA -> I Like NBA");
  m_capitalizeKeepOther->setCheckable(true);
  m_capitalizeKeepOther->setProperty("CaseOperationType", CaseOperationType::CAPITALIZE_KEEP_OTHER);

  m_capitalizeLowerOther = new (std::nothrow) QAction(QIcon(":img/CAPITALIZE_LOWER_OTHER"), tr("Capitalize first and lower other"), this);
  m_capitalizeLowerOther->setToolTip(
      "Capitalize the first letter of each word; lowercase the rest.<br/>"
      "Example:<br/>"
      "i like NBA -> I Like Nba");
  m_capitalizeLowerOther->setCheckable(true);
  m_capitalizeLowerOther->setProperty("CaseOperationType", CaseOperationType::CAPITALIZE_LOWER_OTHER);

  m_toggleCase = new (std::nothrow) QAction(QIcon(":img/RENAME_TOGGLE_CASE"), tr("Toggle Case"), this);
  m_toggleCase->setToolTip("Alternate between uppercase and lowercase letters.");
  m_toggleCase->setCheckable(true);
  m_toggleCase->setProperty("CaseOperationType", CaseOperationType::TOGGLE_CASE);

  m_nameCaseAg = new (std::nothrow) QActionGroup(this);
  m_nameCaseAg->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  m_nameCaseAg->addAction(m_upperCase);
  m_nameCaseAg->addAction(m_lowerCase);
  m_nameCaseAg->addAction(m_capitalizeKeepOther);
  m_nameCaseAg->addAction(m_capitalizeLowerOther);
  m_nameCaseAg->addAction(m_toggleCase);
  m_nameCaseActs = m_nameCaseAg->actions();

  const int checkedIndex = getConfig(RenamerKey::NAME_CASE_CHECKED_INDEX).toInt();
  if (0 <= checkedIndex && checkedIndex < m_nameCaseActs.size()) {
    m_nameCaseActs[checkedIndex]->setChecked(true);
  }
  // no action get checked if out of range

  m_lowerCaseFileExtension = new (std::nothrow) QAction(QIcon(":img/RENAME_DOT_FILE_EXTENSION"), tr("Lowercase Extension"), this);
  m_lowerCaseFileExtension->setToolTip("Convert the file extension to lowercase. (Attention: Not recoverable)");
  m_lowerCaseFileExtension->setCheckable(true);
  m_lowerCaseFileExtension->setChecked(getConfig(RenamerKey::NAME_CASE_LOWER_CASE_FILE_EXTENSION).toBool());
}

QToolBar* RenameWidget_Case::InitControlTB() {
  QToolBar* caseControlTb{new (std::nothrow) QToolBar{"Case", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(caseControlTb);
  caseControlTb->addActions(m_nameCaseActs);
  caseControlTb->addSeparator();
  caseControlTb->addAction(m_lowerCaseFileExtension);
  caseControlTb->addSeparator();
  caseControlTb->addWidget(m_nameExtIndependent);
  caseControlTb->addWidget(m_recursiveCB);
  caseControlTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return caseControlTb;
}

void RenameWidget_Case::extraSubscribe() {
  connect(m_nameCaseAg, &QActionGroup::triggered, this, &RenameWidget_Case::onNameCaseActionTriggered);
  connect(m_lowerCaseFileExtension, &QAction::toggled, this, &RenameWidget_Case::onLowerCaseFileExtension);
}

void RenameWidget_Case::onNameCaseActionTriggered(QAction* pCaseAct) {
  CaseOperationType caseOperation = EffectiveCaseOperation(pCaseAct);
  setConfig(RenamerKey::NAME_CASE_CHECKED_INDEX, caseOperation);
  OnlyTriggerRenameCore();
}

void RenameWidget_Case::onLowerCaseFileExtension(bool bLowerCaseFileExtension) {
  setConfig(RenamerKey::NAME_CASE_LOWER_CASE_FILE_EXTENSION, bLowerCaseFileExtension);
  OnlyTriggerRenameCore();
}