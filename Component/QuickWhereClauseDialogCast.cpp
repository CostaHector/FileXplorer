#include "QuickWhereClauseDialogCast.h"
#include "Notificator.h"
#include "CastPsonFileHelper.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "QuickWhereClauseHelper.h"
#include "CastAkasManager.h"
#include "NoEnterLineEdit.h"

void QuickWhereClauseDialogCast::CreatePrivateWidget() {
  m_Name = new (std::nothrow) NoEnterLineEdit{this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Name);
  m_Name->setPlaceholderText("Henry Cavill&Chris Evans");
  m_Name->setToolTip(                     //
      "A: search 1 person\n"              //
      "A & B: search 2 person BOTH\n"     //
      "A1 | A2: search 2 person EITHER\n" //
      "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");

  m_Rate = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Rate);
  m_Rate->setPlaceholderText(R"(>8)");

  m_Ori = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Ori);
  m_Ori->setPlaceholderText(R"(Orientations here)");

  m_Tags = new (std::nothrow) NoEnterLineEdit{this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Tags);
  m_Tags->setPlaceholderText(R"(Superhero|Documentary)");

  m_Height = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Height);
  m_Height->setPlaceholderText(R"(Height(cm) here)");

  m_Size = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Size);
  m_Size->setPlaceholderText(">1000000000&<1500000000");

#ifdef RUNNING_UNIT_TESTS
  mLineEditsList.push_back(m_Name);
  mLineEditsList.push_back(m_Tags);
  mLineEditsList.push_back(m_Rate);
  mLineEditsList.push_back(m_Ori);
  mLineEditsList.push_back(m_Height);
  mLineEditsList.push_back(m_Size);
#endif

  m_Name->setFocus();
  setWindowTitle(QString{"SELECT * FROM `%1` WHERE ?"}.arg(DB_TABLE::PERFORMERS));
}

void QuickWhereClauseDialogCast::InitPrivateLayout() {
  using namespace PERFORMER_DB_HEADER_KEY;
  m_Layout->addRow(ENUM_2_STR(Name), m_Name);
  m_Layout->addRow(ENUM_2_STR(Rate), m_Rate);
  m_Layout->addRow(ENUM_2_STR(Tags), m_Tags);
  m_Layout->addRow(ENUM_2_STR(Ori), m_Ori);
  m_Layout->addRow(ENUM_2_STR(Height), m_Height);
  m_Layout->addRow(ENUM_2_STR(Size), m_Size);
}

void QuickWhereClauseDialogCast::onConditionsChanged() {
  const QString STR_FILTER_PATTERN{m_strFilterPatternCB->currentText()};

  QStringList conditionsLst;
  conditionsLst.reserve(20);

  const bool bAutoCompleteAka = AUTO_COMPLETE_AKA_SWITCH->isChecked();
  static QHash<QString, QString> EMPTY_HASH;
  const auto* p2AkaHash = &EMPTY_HASH;
  if (bAutoCompleteAka) {
    static auto& dbTM = CastAkasManager::getInst();
    p2AkaHash = &dbTM.CastAkaMap();
  }

  // DB_TABLE::PERFORMERS
  using namespace PERFORMER_DB_HEADER_KEY;
  using namespace QuickWhereClauseHelper;
  const QString& nameClause = InfixNotation2RPN2Value(ENUM_2_STR(Name), m_Name->text(), STR_FILTER_PATTERN, *p2AkaHash);
  if (!nameClause.isEmpty()) {
    conditionsLst << nameClause;
  }

  const QString& rateClause = InfixNotation2RPN2Value(ENUM_2_STR(Rate), m_Rate->text(), OPEATOR_RELATION);
  if (!rateClause.isEmpty()) {
    conditionsLst << rateClause;
  }
  const QString& tagsClause = InfixNotation2RPN2Value(ENUM_2_STR(Tags), m_Tags->text(), STR_FILTER_PATTERN);
  if (!tagsClause.isEmpty()) {
    conditionsLst << tagsClause;
  }
  const QString& oriClause = InfixNotation2RPN2Value(ENUM_2_STR(Ori), m_Ori->text(), STR_FILTER_PATTERN);
  if (!oriClause.isEmpty()) {
    conditionsLst << oriClause;
  }

  const QString& sizeClause = InfixNotation2RPN2Value(ENUM_2_STR(Size), m_Size->text(), OPEATOR_RELATION);
  if (!sizeClause.isEmpty()) {
    conditionsLst << sizeClause;
  }
  const QString& heightClause = InfixNotation2RPN2Value(ENUM_2_STR(Height), m_Height->text(), OPEATOR_RELATION);
  if (!heightClause.isEmpty()) {
    conditionsLst << heightClause;
  }

  m_whereLineEdit->setText(conditionsLst.join(" AND "));
}

void QuickWhereClauseDialogCast::PrivateSubscribe() {
  connect(m_Name, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogCast::onConditionsChanged);
  connect(m_Rate, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogCast::onConditionsChanged);
  connect(m_Tags, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogCast::onConditionsChanged);
  connect(m_Ori, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogCast::onConditionsChanged);
  connect(m_Height, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogCast::onConditionsChanged);
  connect(m_Size, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogCast::onConditionsChanged);
}
