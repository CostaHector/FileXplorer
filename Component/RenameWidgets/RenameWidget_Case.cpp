#include "RenameWidget_Case.h"
#include "NameTool.h"
#include "RenameActions.h"
#include "PublicMacro.h"

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
    LOG_W("Case rule type not found");
    return {};
  }
  QStringList replacedList;
  replacedList.reserve(lst.size());
  const auto pCaseFunc = funcIt.value();
  if (pCaseFunc == nullptr) {
    LOG_W("pCaseFunc is nullptr");
    return {};
  }
  for (const QString& nm : lst) {
    replacedList.append(pCaseFunc(nm));
  }
  return replacedList;
}

RenameWidget_Case::RenameWidget_Case(QWidget* parent)  //
    : AdvanceRenamer(parent) {}

QStringList RenameWidget_Case::RenameCore(const QStringList& replaceeList) {
  const QAction* pCaseAct = g_renameAg().NAME_CASE->checkedAction();
  if (pCaseAct == nullptr) {
    LOG_W("pCaseAct is nullptr");
    return replaceeList;
  }

  if (g_renameAg()._LOWER_CASE_FILE_EXTENSION->isChecked()) {
    m_nExtTE->setPlainText(NameTool::Lower(m_nExtTE->toPlainText()));
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
  caseControlTb->addAction(g_renameAg()._LOWER_CASE_FILE_EXTENSION);
  caseControlTb->addSeparator();
  caseControlTb->addWidget(m_nameExtIndependent);
  caseControlTb->addWidget(m_recursiveCB);
  caseControlTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  return caseControlTb;
}

void RenameWidget_Case::extraSubscribe() {                   //
  connect(g_renameAg().NAME_CASE, &QActionGroup::triggered, this, &AdvanceRenamer::OnlyTriggerRenameCore);
  connect(g_renameAg()._LOWER_CASE_FILE_EXTENSION, &QAction::toggled, this, &AdvanceRenamer::OnlyTriggerRenameCore);
}
