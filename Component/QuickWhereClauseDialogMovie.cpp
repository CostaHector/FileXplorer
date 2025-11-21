#include "QuickWhereClauseDialogMovie.h"
#include "PublicMacro.h"
#include "TableFields.h"
#include "PublicVariable.h"
#include "QuickWhereClauseHelper.h"
#include "CastAkasManager.h"
#include "NoEnterLineEdit.h"

void QuickWhereClauseDialogMovie::CreatePrivateWidget() {
  m_Name = new (std::nothrow) NoEnterLineEdit{this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Name);
  m_Name->setPlaceholderText("Henry Cavill&Chris Evans");
  m_Name->setToolTip(                     //
      "A: search 1 person\n"              //
      "A & B: search 2 person BOTH\n"     //
      "A1 | A2: search 2 person EITHER\n" //
      "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");

  m_Size = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Size);
  m_Size->setPlaceholderText(">1000000000&<1500000000");

  m_Duration = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Duration);
  m_Duration->setPlaceholderText(">6000"); // 6s
  m_Duration->setToolTip("unit: ms");

  m_Studio = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Studio);
  m_Studio->setPlaceholderText(R"(20th Century Studios|Marvel Studios)");

  m_Cast = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_Cast);
  m_Cast->setPlaceholderText(R"(Henry Cavill&Chris Evans)");

  m_Tags = new (std::nothrow) NoEnterLineEdit{this}; // shared
  CHECK_NULLPTR_RETURN_VOID(m_Tags);
  m_Tags->setPlaceholderText(R"(Superhero|Documentary)");

#ifdef RUNNING_UNIT_TESTS
  mLineEditsList.push_back(m_Name);
  mLineEditsList.push_back(m_Size);
  mLineEditsList.push_back(m_Duration);
  mLineEditsList.push_back(m_Studio);
  mLineEditsList.push_back(m_Cast);
  mLineEditsList.push_back(m_Tags);
#endif

  m_Name->setFocus();
  setWindowTitle(QString{"SELECT * FROM `%1` WHERE ?"}.arg(DB_TABLE::MOVIES));
}

void QuickWhereClauseDialogMovie::PrivateSubscribe() {
  connect(m_Name, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogMovie::onConditionsChanged);
  connect(m_Size, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogMovie::onConditionsChanged);
  connect(m_Duration, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogMovie::onConditionsChanged);
  connect(m_Studio, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogMovie::onConditionsChanged);
  connect(m_Cast, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogMovie::onConditionsChanged);
  connect(m_Tags, &QLineEdit::returnPressed, this, &QuickWhereClauseDialogMovie::onConditionsChanged);
}

void QuickWhereClauseDialogMovie::onConditionsChanged() {
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

  // DB_TABLE::MOVIES
  using namespace MOVIE_TABLE;
  using namespace QuickWhereClauseHelper;
  const QString& nameClause = InfixNotation2RPN2Value(ENUM_2_STR(Name), m_Name->text(), STR_FILTER_PATTERN, *p2AkaHash);
  if (!nameClause.isEmpty()) {
    conditionsLst << nameClause;
  }
  const QString& sizeClause = InfixNotation2RPN2Value(ENUM_2_STR(Size), m_Size->text(), OPEATOR_RELATION);
  if (!sizeClause.isEmpty()) {
    conditionsLst << sizeClause;
  }
  const QString& durationClause = InfixNotation2RPN2Value(ENUM_2_STR(Duration), m_Duration->text(), OPEATOR_RELATION);
  if (!durationClause.isEmpty()) {
    conditionsLst << durationClause;
  }
  const QString& studioClause = InfixNotation2RPN2Value(ENUM_2_STR(Studio), m_Studio->text(), STR_FILTER_PATTERN);
  if (!studioClause.isEmpty()) {
    conditionsLst << studioClause;
  }
  const QString& castClause = InfixNotation2RPN2Value(ENUM_2_STR(Cast), m_Cast->text(), STR_FILTER_PATTERN);
  if (!castClause.isEmpty()) {
    conditionsLst << castClause;
  }
  const QString& tagsClause = InfixNotation2RPN2Value(ENUM_2_STR(Tags), m_Tags->text(), STR_FILTER_PATTERN);
  if (!tagsClause.isEmpty()) {
    conditionsLst << tagsClause;
  }

  m_whereLineEdit->setText(conditionsLst.join(" AND "));
}

void QuickWhereClauseDialogMovie::InitPrivateLayout() {
  using namespace MOVIE_TABLE;
  m_Layout->addRow(ENUM_2_STR(Name), m_Name);
  m_Layout->addRow(ENUM_2_STR(Size), m_Size);
  m_Layout->addRow(ENUM_2_STR(Duration), m_Duration);
  m_Layout->addRow(ENUM_2_STR(Studio), m_Studio);
  m_Layout->addRow(ENUM_2_STR(Cast), m_Cast);
  m_Layout->addRow(ENUM_2_STR(Tags), m_Tags);
}
