#include "QuickWhereClause.h"

#include <QButtonGroup>

#include <QDesktopServices>
#include <QHash>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStack>

#include "MovieDBActions.h"
#include "QuickWhereActions.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include "PerformersAkaManager.h"
#include "TableFields.h"
#include "StyleSheet.h"

QuickWhereClause::QuickWhereClause(QWidget* parent) : QDialog{parent} {
  m_Name = new (std::nothrow) QLineEdit{this};
  m_Name->setPlaceholderText("Henry Cavill&Chris Evans");
  m_Name->setToolTip(                      //
      "A: search 1 person\n"               //
      "A & B: search 2 person BOTH\n"      //
      "A1 | A2: search 2 person EITHER\n"  //
      "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");

  m_Size = new (std::nothrow) QLineEdit{this};
  m_Size->setPlaceholderText(">1000000000&<1500000000");

  m_Duration = new (std::nothrow) QLineEdit{this};
  m_Duration->setPlaceholderText(">6000");                    // 6s
  m_Duration->setToolTip("unit: ms");

  m_Studio = new (std::nothrow) QLineEdit{this};
  m_Studio->setPlaceholderText(R"(Fox|Hong Meng)");           //

  m_Cast = new (std::nothrow) QLineEdit{this};
  m_Cast->setPlaceholderText(R"(Henry Cavill&Chris Evans)");  //

  m_Tags = new (std::nothrow) QLineEdit{this};
  m_Tags->setPlaceholderText(R"(Comedy|Documentary)");        //

  m_whereLineEdit = new (std::nothrow) QLineEdit{this};

  mDialogButtonBox = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
  mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok)->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  using namespace MOVIE_TABLE;
  m_Layout = new (std::nothrow) QFormLayout{this};
  m_Layout->addRow(g_quickWhereAg().m_whereClauseTB);
  m_Layout->addRow(ENUM_2_STR(Name), m_Name);  // (Ricky Martin|Ricky)&Adam Lambert
  m_Layout->addRow(ENUM_2_STR(Size), m_Size);
  m_Layout->addRow(ENUM_2_STR(Duration), m_Duration);
  m_Layout->addRow(ENUM_2_STR(Studio), m_Studio);
  m_Layout->addRow(ENUM_2_STR(Cast), m_Cast);
  m_Layout->addRow(ENUM_2_STR(Tags), m_Tags);
  m_Layout->addRow(m_whereLineEdit);
  m_Layout->addRow(mDialogButtonBox);

  subscribe();

  setWindowIcon(g_dbAct().QUICK_WHERE_CLAUSE->icon());
  setWindowTitle(g_dbAct().QUICK_WHERE_CLAUSE->text());

  m_Name->setFocus();
}

void QuickWhereClause::onClauseSave() {
  const QString& curClause = GetWhereString();
  g_quickWhereAg().m_historyWhereClauseMenu->addAction(new (std::nothrow) QAction(curClause, this));
  const QString& before = Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString();
  Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, curClause + '\n' + GetWhereString());
}

void QuickWhereClause::onClauseChanged() {
  using namespace MOVIE_TABLE;
  m_whereAndClause.clear();
  bool bAutoCompleteAka{g_quickWhereAg().AUTO_COMPLETE_AKA_SWITCH->isChecked()};
  static QHash<QString, QString> EMPTY_HASH;
  const auto* p2AkaHash = &EMPTY_HASH;
  if (bAutoCompleteAka) {
    static auto& dbTM = PerformersAkaManager::getIns();
    p2AkaHash = &dbTM.m_akaPerf;
  }

  const QString& nameClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Name), m_Name->text(), PerformersAkaManager::FUZZY_LIKE, *p2AkaHash);
  if (!nameClause.isEmpty()) {
    m_whereAndClause << nameClause;
  }

  const QString& sizeClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Size), m_Size->text(), "%1%2");
  if (!sizeClause.isEmpty()) {
    m_whereAndClause << sizeClause;
  }
  const QString& durationClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Duration), m_Duration->text(), "%1%2");
  if (!durationClause.isEmpty()) {
    m_whereAndClause << durationClause;
  }
  const QString& studioClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Studio), m_Studio->text());
  if (!studioClause.isEmpty()) {
    m_whereAndClause << studioClause;
  }
  const QString& castClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Cast), m_Cast->text());
  if (!castClause.isEmpty()) {
    m_whereAndClause << castClause;
  }
  const QString& tagsClause = PerformersAkaManager::PlainLogicSentence2FuzzySqlWhere(ENUM_2_STR(Tags), m_Tags->text());
  if (!tagsClause.isEmpty()) {
    m_whereAndClause << tagsClause;
  }
  m_whereLineEdit->setText(m_whereAndClause.join(" AND "));
}

void QuickWhereClause::accept() {
  if (not mDialogButtonBox->hasFocus()) {
    return;
  }
  QDialog::accept();
}

void QuickWhereClause::subscribe() {
  connect(m_Name, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Size, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Duration, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Studio, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Cast, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Tags, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);

  connect(mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QuickWhereClause::accept);
  connect(mDialogButtonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &QDialog::reject);

  auto& quickWhereInst =g_quickWhereAg();
  connect(quickWhereInst.SAVE_WHERE, &QAction::triggered, this, &QuickWhereClause::onClauseSave);
  connect(quickWhereInst.HIST_WHERE, &QToolButton::triggered, this, [this](QAction* act) { m_whereLineEdit->setText(act->text()); });
  connect(quickWhereInst.APPLY_AND_CLOSE, &QAction::triggered, this, [this]() {
    mDialogButtonBox->setFocus();
    accept();
  });
}

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  QuickWhereClause qwc;
  auto retCode = qwc.exec();
  if (retCode == QDialog::DialogCode::Accepted) {
    qDebug("retCode: %d", retCode);
    qDebug("%s", qPrintable(qwc.GetWhereString()));
  }
  return 0;
}
#endif
