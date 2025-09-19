#include "QuickWhereClauseDialog.h"
#include "QuickWhereClauseHelper.h"
#include "CastAkasManager.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "StringTool.h"
#include "TableFields.h"
#include <QHash>
#include <QPushButton>
#include <QInputDialog>

constexpr char QuickWhereClauseDialog::WHERE_HIST_SPLIT_CHAR;

QuickWhereClauseDialog::QuickWhereClauseDialog(QWidget* parent)//
  : QDialog{parent} {
  AUTO_COMPLETE_AKA_SWITCH = new (std::nothrow) QAction{"Name Alias", this};
  AUTO_COMPLETE_AKA_SWITCH->setCheckable(true);
  AUTO_COMPLETE_AKA_SWITCH->setChecked(false);

  _RMV_WHERE_CLAUSE_FROM_HISTORY = new (std::nothrow) QAction{QIcon{":img/WHERE_CLAUSE_HISTORY_REMOVED"}, "Remove", this};
  _CLEAR_WHERE_CLAUSE_FROM_HISTORY = new (std::nothrow) QAction{QIcon{":img/WHERE_CLAUSE_HISTORY_CLEAR"}, "Clear", this};
  mWhereClauseHistoryDecMenu = new (std::nothrow) QMenu{"Where Clause Edit(Dec) Menu", this};
  mWhereClauseHistoryDecMenu->addAction(_CLEAR_WHERE_CLAUSE_FROM_HISTORY);
  mWhereClauseHistoryDecTb = new (std::nothrow) QToolButton{this};
  mWhereClauseHistoryDecTb->setDefaultAction(_RMV_WHERE_CLAUSE_FROM_HISTORY);
  mWhereClauseHistoryDecTb->setMenu(mWhereClauseHistoryDecMenu);
  mWhereClauseHistoryDecTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mWhereClauseHistoryDecTb->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);

  _ADD_WHERE_CLAUSE_TO_HISTORY = new (std::nothrow) QAction{QIcon{":img/WHERE_CLAUSE_HISTORY_ADD"}, "Add", this};
  _EDIT_WHERE_CLAUSE_HISTORY = new (std::nothrow) QAction{QIcon{":img/WHERE_CLAUSE_HISTORY_EDIT"}, "Edit", this};
  mWhereClauseHistoryIncMenu = new (std::nothrow) QMenu{"Where Clause Edit(Inc) Menu", this};
  mWhereClauseHistoryIncMenu->addAction(_EDIT_WHERE_CLAUSE_HISTORY);
  mWhereClauseHistoryIncTb = new (std::nothrow) QToolButton{this};
  mWhereClauseHistoryIncTb->setDefaultAction(_ADD_WHERE_CLAUSE_TO_HISTORY);
  mWhereClauseHistoryIncTb->setMenu(mWhereClauseHistoryIncMenu);
  mWhereClauseHistoryIncTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mWhereClauseHistoryIncTb->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);

  /* DB_TABLE::MOVIES exclusive */
  {
    m_Name = new (std::nothrow) QLineEdit{this}; // shared
    CHECK_NULLPTR_RETURN_VOID(m_Name)
    m_Name->setPlaceholderText("Henry Cavill&Chris Evans");
    m_Name->setToolTip(                      //
        "A: search 1 person\n"               //
        "A & B: search 2 person BOTH\n"      //
        "A1 | A2: search 2 person EITHER\n"  //
        "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");

    m_Size = new (std::nothrow) QLineEdit{this};
    CHECK_NULLPTR_RETURN_VOID(m_Size)
    m_Size->setPlaceholderText(">1000000000&<1500000000");

    m_Duration = new (std::nothrow) QLineEdit{this};
    CHECK_NULLPTR_RETURN_VOID(m_Duration)
    m_Duration->setPlaceholderText(">6000"); // 6s
    m_Duration->setToolTip("unit: ms");

    m_Studio = new (std::nothrow) QLineEdit{this};
    CHECK_NULLPTR_RETURN_VOID(m_Studio)
    m_Studio->setPlaceholderText(R"(20th Century Studios|Marvel Studios)");

    m_Cast = new (std::nothrow) QLineEdit{this};
    CHECK_NULLPTR_RETURN_VOID(m_Cast)
    m_Cast->setPlaceholderText(R"(Henry Cavill&Chris Evans)");

    m_Tags = new (std::nothrow) QLineEdit{this}; // shared
    CHECK_NULLPTR_RETURN_VOID(m_Tags)
    m_Tags->setPlaceholderText(R"(Superhero|Documentary)");
  }
  /* DB_TABLE::PERFORMERS exclusive */
  {
    m_Rate = new (std::nothrow) QLineEdit{this};
    CHECK_NULLPTR_RETURN_VOID(m_Rate)
    m_Rate->setPlaceholderText(R"(>8)");

    m_Ori = new (std::nothrow) QLineEdit{this};
    CHECK_NULLPTR_RETURN_VOID(m_Ori)
    m_Ori->setPlaceholderText(R"(Orientations here)");
  }

  m_strFilterPatternCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_strFilterPatternCB)
  m_strFilterPatternCB->setEditable(false);
  m_strFilterPatternCB->insertItem((int)Qt::CaseSensitivity::CaseInsensitive, QuickWhereClauseHelper::FUZZY_LIKE);
  m_strFilterPatternCB->insertItem((int)Qt::CaseSensitivity::CaseSensitive, QuickWhereClauseHelper::FUZZY_INSTR);

  m_whereHistComboBox = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereHistComboBox)
  m_whereHistComboBox->setEditable(false);
  m_whereHistComboBox->setInsertPolicy(QComboBox::InsertAtTop);

  QString hists {Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString()};
  if (hists.isEmpty()) {
    hists += R"(`NAME` LIKE "%%")";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(INSTR(`NAME`, "")>0)";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(`Size`>1024000000)";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(`Ori` LIKE "%%")";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(INSTR(`Ori`, "")>0)";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(`Rate`>7)";
  }
  mStrListModel = new (std::nothrow) QStringListModel{this};
  mStrListModel->setStringList(hists.split(WHERE_HIST_SPLIT_CHAR));
  m_whereHistComboBox->setModel(mStrListModel);

  m_whereLineEdit = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereLineEdit)
  m_whereLineEdit->setPlaceholderText("Where Clause here");

  mDialogButtonBox = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
  CHECK_NULLPTR_RETURN_VOID(mDialogButtonBox)
  auto* pOkBtn = mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok);
  pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Apply where clause right now.")  //
                         .arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
  pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  m_Layout = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(m_Layout)
  {
    m_Layout->addRow(DB_TABLE::MOVIES+":", (QWidget*)nullptr);
    using namespace MOVIE_TABLE;
    m_Layout->addRow(ENUM_2_STR(Name), m_Name);
    m_Layout->addRow(ENUM_2_STR(Size), m_Size);
    m_Layout->addRow(ENUM_2_STR(Duration), m_Duration);
    m_Layout->addRow(ENUM_2_STR(Studio), m_Studio);
    m_Layout->addRow(ENUM_2_STR(Cast), m_Cast);
    m_Layout->addRow(ENUM_2_STR(Tags), m_Tags);
  }
  {
    m_Layout->addRow(DB_TABLE::PERFORMERS+":", (QWidget*)nullptr);
    using namespace PERFORMER_DB_HEADER_KEY;
    m_Layout->addRow(ENUM_2_STR(Rate), m_Rate);
    m_Layout->addRow(ENUM_2_STR(Ori), m_Ori);
  }
  {
    m_Layout->addRow("WHERE clause:", (QWidget*)nullptr);
    m_Layout->addRow("Pattern:", m_strFilterPatternCB);
    m_Layout->addRow(mWhereClauseHistoryDecTb, m_whereHistComboBox);
    m_Layout->addRow(mWhereClauseHistoryIncTb, m_whereLineEdit);
  }
  m_Layout->addWidget(mDialogButtonBox);

#ifdef RUNNING_UNIT_TESTS
  mLineEditsList.push_back(m_Name);
  mLineEditsList.push_back(m_Size);
  mLineEditsList.push_back(m_Duration);
  mLineEditsList.push_back(m_Studio);
  mLineEditsList.push_back(m_Cast);
  mLineEditsList.push_back(m_Tags);
  mLineEditsList.push_back(m_Rate);
  mLineEditsList.push_back(m_Ori);
#endif

  subscribe();

  setWindowIcon(QIcon{":img/QUICK_WHERE_FILTERS"});
  setWindowTitle(QString{"SELECT * FROM `%1|%2` WHERE ?"}.arg(DB_TABLE::MOVIES).arg(DB_TABLE::PERFORMERS));

  m_Name->setFocus();
}

QuickWhereClauseDialog::~QuickWhereClauseDialog() {
#ifndef RUNNING_UNIT_TESTS
  WriteUniqueHistoryToQSetting();
#endif
}

int QuickWhereClauseDialog::WriteUniqueHistoryToQSetting() {
  QStringList hists = mStrListModel->stringList();
  StringTool::SearchHistoryListProc(hists);
  Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, hists.join(WHERE_HIST_SPLIT_CHAR));
  return hists.size();
}

void QuickWhereClauseDialog::onConditionsChanged() {
  const QString STR_FILTER_PATTERN{m_strFilterPatternCB->currentText()};

  QStringList conditionsLst;
  conditionsLst.reserve(20);
  {
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
  }
  {
    // DB_TABLE::PERFORMERS
    using namespace PERFORMER_DB_HEADER_KEY;
    using namespace QuickWhereClauseHelper;
    const QString& rateClause = InfixNotation2RPN2Value(ENUM_2_STR(Rate), m_Rate->text(), OPEATOR_RELATION);
    if (!rateClause.isEmpty()) {
      conditionsLst << rateClause;
    }
    const QString& oriClause = InfixNotation2RPN2Value(ENUM_2_STR(Ori), m_Ori->text(), STR_FILTER_PATTERN);
    if (!oriClause.isEmpty()) {
      conditionsLst << oriClause;
    }
  }

  m_whereLineEdit->setText(conditionsLst.join(" AND "));
}

void QuickWhereClauseDialog::accept() {
  if (!mDialogButtonBox->hasFocus()) {
    return;
  }
  QDialog::accept();
}

void QuickWhereClauseDialog::subscribe() {
  {
    connect(m_Name, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
    connect(m_Size, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
    connect(m_Duration, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
    connect(m_Studio, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
    connect(m_Cast, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
    connect(m_Tags, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
  }
  {
    connect(m_Rate, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
    connect(m_Ori, &QLineEdit::returnPressed, this, &QuickWhereClauseDialog::onConditionsChanged);
  }
  connect(m_strFilterPatternCB, &QComboBox::currentTextChanged, this, &QuickWhereClauseDialog::onConditionsChanged);

  connect(mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QuickWhereClauseDialog::accept);
  connect(mDialogButtonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &QDialog::reject);

  connect(_RMV_WHERE_CLAUSE_FROM_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onRemoveAHistory);
  connect(_CLEAR_WHERE_CLAUSE_FROM_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onClearHistory);
  connect(_ADD_WHERE_CLAUSE_TO_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onAddAHistory);
  connect(_EDIT_WHERE_CLAUSE_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onEditHistory);

  connect(m_whereHistComboBox, &QComboBox::textActivated, m_whereLineEdit, &QLineEdit::setText);
}

void QuickWhereClauseDialog::SetStrPatternCaseSensitive(Qt::CaseSensitivity caseSen) {
  const int patternsCnt = m_strFilterPatternCB->count();
  if (caseSen >= patternsCnt) {
    LOG_W("Name str pattern index[%d] out of range[0, %d)", (int)caseSen, patternsCnt);
    return;
  }
  m_strFilterPatternCB->setCurrentIndex((int)caseSen);
}

bool QuickWhereClauseDialog::onRemoveAHistory() {
  int n = mStrListModel->rowCount();
  if (n <= 0) {
    LOG_D("History already empty, skip delete");
    return true;
  }
  int index = m_whereHistComboBox->currentIndex();
  if (index >= n) {
    LOG_W("index[%d] out of range [0,%d), skip delete", index, n);
    return false;
  }
  mStrListModel->removeRow(index);
  return true;
}

int QuickWhereClauseDialog::onClearHistory() {
  const int n = mStrListModel->rowCount();
  if (n <= 0) {
    LOG_D("model already empty, skip clear");
    return 0;
  }
  mStrListModel->removeRows(0, n);
  return n;
}

bool QuickWhereClauseDialog::onAddAHistory() {
  const QString newHistLine{m_whereLineEdit->text().trimmed()};
  if (newHistLine.isEmpty()) {
    LOG_D("Where clause[%s] empty or already exist. skip insert", qPrintable(newHistLine));
    return false;
  }
  mStrListModel->insertRow(0);
  mStrListModel->setData(mStrListModel->index(0), newHistLine);
  return true;
}
int QuickWhereClauseDialog::onEditHistory() {
  int beforeRowCnt = mStrListModel->rowCount();
  QString curHists = mStrListModel->stringList().join(WHERE_HIST_SPLIT_CHAR);
#ifndef RUNNING_UNIT_TESTS
  bool editOk{false};
  curHists = QInputDialog::getMultiLineText(this, "Edit where history Below", QString::number(beforeRowCnt), //
                                            curHists, &editOk);
  if (!editOk) {
    LOG_D("User Cancel edit history");
    return 0;
  }  
  const QStringList newHists{curHists.split(WHERE_HIST_SPLIT_CHAR)};
#else
  const QStringList newHists = newWhereHistsList;
#endif
  int afterRowCnt = newHists.size();
  mStrListModel->setStringList(newHists);
  return afterRowCnt - beforeRowCnt;
}
