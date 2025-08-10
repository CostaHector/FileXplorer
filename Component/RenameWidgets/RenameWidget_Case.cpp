#include "RenameWidget_Case.h"
#include "NameTool.h"
#include "RenameActions.h"
#include "PublicMacro.h"

typedef enum tagRENAME_CASE_E {
  UPPER_CASE = 0,
  LOWER_CASE,
  CAPITALIZE_FIRST_LETTER,
  CAPITALIZE_FIRST_LETTER_AND_LOWER_OTHER,
  TOGGLE_CASE,
  BUTTON_CASE,
} RENAME_CASE_E;

struct StringCaseOperator {
  QStringList operator()(const QStringList& lst, const QAction* caseAct) const;
};

QStringList StringCaseOperator::operator()(const QStringList& lst, const QAction* caseAct) const {
  static const QHash<const QAction*, decltype(NameTool::Upper)*> RULE_OPS_MAP{
      {g_renameAg()._UPPER_CASE, NameTool::Upper},
      {g_renameAg()._LOWER_CASE, NameTool::Lower},
      {g_renameAg()._CAPITALIZE_KEEP_OTHER, NameTool::CapitaliseFirstLetterKeepOther},
      {g_renameAg()._CAPITALIZE_LOWER_OTHER, NameTool::CapitaliseFirstLetterLowerOther},
      {g_renameAg()._TOGGLE_CASE, NameTool::ToggleSentenceCase},
  };

  auto funcIt = RULE_OPS_MAP.find(caseAct);
  if (funcIt == RULE_OPS_MAP.cend()) {
    qWarning("Case rule type not found");
    return {};
  }
  QStringList replacedList;
  replacedList.reserve(lst.size());
  const auto pCaseFunc = funcIt.value();
  if (pCaseFunc == nullptr) {
    qWarning("pCaseFunc is nullptr");
    return {};
  }
  for (const QString& nm : lst) {
    replacedList.append(pCaseFunc(nm));
  }
  return replacedList;
}

RenameWidget_Case::RenameWidget_Case(QWidget* parent)  //
    : AdvanceRenamer(parent) {}

auto RenameWidget_Case::RenameCore(const QStringList& replaceeList) -> QStringList {
  const QAction* pCaseAct = g_renameAg().NAME_CASE->checkedAction();  // todo checked
  if (pCaseAct == nullptr) {
    qWarning("pCaseAct is nullptr");
    return replaceeList;
  }
  const StringCaseOperator sco;
  return sco(replaceeList, pCaseAct);
}

void RenameWidget_Case::InitExtraCommonVariable() {
  windowTitleFormat = "Case name string | %1 item(s) under [%2]";
  setWindowTitle(windowTitleFormat);
  setWindowIcon(QIcon(":img/NAME_STR_CASE"));
}

QToolBar* RenameWidget_Case::InitControlTB() {
  QToolBar* caseControlTb{new (std::nothrow) QToolBar{"Case", this}};
  CHECK_NULLPTR_RETURN_NULLPTR(caseControlTb);
  caseControlTb->addActions(g_renameAg().NAME_CASE->actions());
  caseControlTb->addSeparator();
  caseControlTb->addWidget(m_nameExtIndependent);
  caseControlTb->addWidget(m_recursiveCB);
  caseControlTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return caseControlTb;
}

void RenameWidget_Case::extraSubscribe() {                   //
  connect(g_renameAg().NAME_CASE, &QActionGroup::triggered,  //
          this, &AdvanceRenamer::OnlyTriggerRenameCore);
}
