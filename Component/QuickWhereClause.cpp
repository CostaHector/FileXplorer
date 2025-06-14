#include "QuickWhereClause.h"

#include <QButtonGroup>

#include <QDesktopServices>
#include <QHash>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStack>

#include "Actions/MovieDBActions.h"
#include "Actions/QuickWhereActions.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "public/PublicMacro.h"
#include "Tools/PerformersAkaManager.h"
#include "Tools/FileDescriptor/TableFields.h"
#include "public/StyleSheet.h"

QuickWhereClause::QuickWhereClause(QWidget* parent) : QDialog{parent} {
  m_Name = new QLineEdit;
  m_Size = new QLineEdit;
  m_Duration = new QLineEdit;
  m_Studio = new QLineEdit;
  m_Cast = new QLineEdit;
  m_Tags = new QLineEdit;
  m_whereLineEdit = new QLineEdit;

  m_Name->setPlaceholderText("Henry Cavill&Chris Evans");
  m_Name->setToolTip(                      //
      "A: search 1 person\n"               //
      "A & B: search 2 person BOTH\n"      //
      "A1 | A2: search 2 person EITHER\n"  //
      "(A1|A2)&B: search 2 person BOTH(One person also known as A2)");
  m_Size->setPlaceholderText(">1000000000&<1500000000");
  m_Duration->setPlaceholderText(">6000");                    // 6s
  m_Studio->setPlaceholderText(R"(Fox|Hong Meng)");           //
  m_Cast->setPlaceholderText(R"(Henry Cavill&Chris Evans)");  //
  m_Tags->setPlaceholderText(R"(Comedy|Documentary)");        //

  dbb = new (std::nothrow) QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
  dbb->button(QDialogButtonBox::StandardButton::Ok)->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  using namespace MOVIE_TABLE;
  QFormLayout* lo = new QFormLayout(this);
  lo->addRow(g_quickWhereAg().m_whereClauseTB);
  lo->addRow(ENUM_2_STR(Name), m_Name);  // (Ricky Martin|Ricky)&Adam Lambert
  lo->addRow(ENUM_2_STR(Size), m_Size);
  lo->addRow(ENUM_2_STR(Duration), m_Duration);
  lo->addRow(ENUM_2_STR(Studio), m_Studio);
  lo->addRow(ENUM_2_STR(Cast), m_Cast);
  lo->addRow(ENUM_2_STR(Tags), m_Tags);
  lo->addRow(m_whereLineEdit);
  lo->addWidget(dbb);

  connect(m_Name, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Size, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Duration, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Studio, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Cast, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);
  connect(m_Tags, &QLineEdit::returnPressed, this, &QuickWhereClause::onClauseChanged);

  connect(dbb->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &QuickWhereClause::accept);
  connect(dbb->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &QDialog::reject);
  connect(g_quickWhereAg().SAVE_WHERE, &QAction::triggered, this, &QuickWhereClause::onClauseSave);
  connect(g_quickWhereAg().HIST_WHERE, &QToolButton::triggered, this, [this](QAction* act) {  //
    m_whereLineEdit->setText(act->text());
  });
  connect(g_quickWhereAg().APPLY_AND_CLOSE, &QAction::triggered, this, [this]() {
    dbb->setFocus();
    accept();
  });

  setWindowIcon(g_dbAct().QUICK_WHERE_CLAUSE->icon());
  setWindowTitle(g_dbAct().QUICK_WHERE_CLAUSE->text());

  m_Name->setFocus();
}

void QuickWhereClause::onClauseSave() {
  const QString& curClause = GetWhereString();
  g_quickWhereAg().m_historyWhereClauseMenu->addAction(new (std::nothrow) QAction(curClause, this));
  const QString& before = PreferenceSettings().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString();
  PreferenceSettings().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, curClause + '\n' + GetWhereString());
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
  if (not dbb->hasFocus()) {
    return;
  }
  QDialog::accept();
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
