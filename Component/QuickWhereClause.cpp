#include "QuickWhereClause.h"

#include "PublicMacro.h"
#include "PerformersAkaManager.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "TableFields.h"
#include <QHash>
#include <QPushButton>
#include <QInputDialog>

constexpr char QuickWhereClause::WHERE_HIST_SPLIT_CHAR;

QuickWhereClause::QuickWhereClause(QWidget* parent) : QDialog{parent} {
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

  m_whereComboBox = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereComboBox)
  m_whereComboBox->setEditable(false);
  m_whereComboBox->setInsertPolicy(QComboBox::InsertAtTop);

  QString hists {Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString()};
  if (hists.isEmpty()) {
    hists += R"(INSTR(`NAME`, "")>0)";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(`Size`>1024000000)";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(INSTR(`Ori`, "")>0)";
    hists += WHERE_HIST_SPLIT_CHAR;
    hists += R"(`Rate`>7)";
  }
  mStrListModel = new (std::nothrow) QStringListModel{this};
  mStrListModel->setStringList(hists.split(WHERE_HIST_SPLIT_CHAR));
  m_whereComboBox->setModel(mStrListModel);

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
    m_Layout->addRow(mWhereClauseHistoryDecTb, m_whereComboBox);
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

  setWindowIcon(QIcon{":img/FILE_SYSTEM_FILTER"});
  setWindowTitle(QString{"SELECT * FROM `%1|%2` WHERE ?"}.arg(DB_TABLE::MOVIES).arg(DB_TABLE::PERFORMERS));

  m_Name->setFocus();
}

QuickWhereClause::~QuickWhereClause() {
#ifndef RUNNING_UNIT_TESTS
  WriteUniqueHistoryToQSetting();
#endif
}

int QuickWhereClause::WriteUniqueHistoryToQSetting() {
  QStringList hists = mStrListModel->stringList();
  std::sort(hists.begin(), hists.end());
  auto firstDupIt = std::unique(hists.begin(), hists.end());
  hists.erase(firstDupIt, hists.end());

  Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, hists.join(WHERE_HIST_SPLIT_CHAR));
  return hists.size();
}

void QuickWhereClause::onConditionsChanged() {
  QStringList conditionsLst;
  conditionsLst.reserve(20);
  {
    const bool bAutoCompleteAka = AUTO_COMPLETE_AKA_SWITCH->isChecked();
    static QHash<QString, QString> EMPTY_HASH;
    const auto* p2AkaHash = &EMPTY_HASH;
    if (bAutoCompleteAka) {
      static auto& dbTM = PerformersAkaManager::getIns();
      p2AkaHash = &dbTM.m_akaPerf;
    }

    // DB_TABLE::MOVIES
    using namespace MOVIE_TABLE;
    const QString& nameClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Name), m_Name->text(), PerformersAkaManager::FUZZY_LIKE, *p2AkaHash);
    if (!nameClause.isEmpty()) {
      conditionsLst << nameClause;
    }
    const QString& sizeClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Size), m_Size->text(), PerformersAkaManager::OPEATOR_RELATION);
    if (!sizeClause.isEmpty()) {
      conditionsLst << sizeClause;
    }
    const QString& durationClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Duration), m_Duration->text(), PerformersAkaManager::OPEATOR_RELATION);
    if (!durationClause.isEmpty()) {
      conditionsLst << durationClause;
    }
    const QString& studioClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Studio), m_Studio->text());
    if (!studioClause.isEmpty()) {
      conditionsLst << studioClause;
    }
    const QString& castClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Cast), m_Cast->text());
    if (!castClause.isEmpty()) {
      conditionsLst << castClause;
    }
    const QString& tagsClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Tags), m_Tags->text());
    if (!tagsClause.isEmpty()) {
      conditionsLst << tagsClause;
    }
  }
  {
    // DB_TABLE::PERFORMERS
    using namespace PERFORMER_DB_HEADER_KEY;
    const QString& rateClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Rate), m_Rate->text(), PerformersAkaManager::OPEATOR_RELATION);
    if (!rateClause.isEmpty()) {
      conditionsLst << rateClause;
    }
    const QString& oriClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Ori), m_Ori->text());
    if (!oriClause.isEmpty()) {
      conditionsLst << oriClause;
    }
  }

  m_whereLineEdit->setText(conditionsLst.join(" AND "));
}

void QuickWhereClause::accept() {
  if (!mDialogButtonBox->hasFocus()) {
    return;
  }
  QDialog::accept();
}

void QuickWhereClause::subscribe() {
  {
    connect(m_Name, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
    connect(m_Size, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
    connect(m_Duration, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
    connect(m_Studio, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
    connect(m_Cast, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
    connect(m_Tags, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
  }
  {
    connect(m_Rate, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
    connect(m_Ori, &QLineEdit::returnPressed, this, &QuickWhereClause::onConditionsChanged);
  }
  connect(mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QuickWhereClause::accept);
  connect(mDialogButtonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &QDialog::reject);

  connect(_RMV_WHERE_CLAUSE_FROM_HISTORY, &QAction::triggered, this, &QuickWhereClause::onRemoveAHistory);
  connect(_CLEAR_WHERE_CLAUSE_FROM_HISTORY, &QAction::triggered, this, &QuickWhereClause::onClearHistory);
  connect(_ADD_WHERE_CLAUSE_TO_HISTORY, &QAction::triggered, this, &QuickWhereClause::onAddAHistory);
  connect(_EDIT_WHERE_CLAUSE_HISTORY, &QAction::triggered, this, &QuickWhereClause::onEditHistory);

  connect(m_whereComboBox, &QComboBox::textActivated, m_whereLineEdit, &QLineEdit::setText);
}

bool QuickWhereClause::onRemoveAHistory() {
  int n = mStrListModel->rowCount();
  if (n <= 0) {
    qDebug("History already empty, skip delete");
    return true;
  }
  int index = m_whereComboBox->currentIndex();
  if (index >= n) {
    qWarning("index[%d] out of range [0,%d), skip delete", index, n);
    return false;
  }
  mStrListModel->removeRow(index);
  return true;
}

int QuickWhereClause::onClearHistory() {
  const int n = mStrListModel->rowCount();
  if (n <= 0) {
    qDebug("model already empty, skip clear");
    return 0;
  }
  mStrListModel->removeRows(0, n);
  return n;
}

bool QuickWhereClause::onAddAHistory() {
  const QString newHistLine{m_whereLineEdit->text().trimmed()};
  if (newHistLine.isEmpty()) {
    qDebug("Where clause[%s] empty or already exist. skip insert", qPrintable(newHistLine));
    return false;
  }
  mStrListModel->insertRow(0);
  mStrListModel->setData(mStrListModel->index(0), newHistLine);
  return true;
}
int QuickWhereClause::onEditHistory() {
  int beforeRowCnt = mStrListModel->rowCount();
  QString curHists = mStrListModel->stringList().join(WHERE_HIST_SPLIT_CHAR);
#ifndef RUNNING_UNIT_TESTS
  bool editOk{false};
  curHists = QInputDialog::getMultiLineText(this, "Edit where history Below", QString::number(beforeRowCnt), //
                                            curHists, &editOk);
  if (!editOk) {
    qDebug("User Cancel edit history");
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
