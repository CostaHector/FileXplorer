#include "QuickWhereClauseDialog.h"
#include "QuickWhereClauseHelper.h"
#include "CastAkasManager.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include "StyleSheet.h"
#include "StringTool.h"
#include "TableFields.h"
#include "CastPsonFileHelper.h"
#include <QHash>
#include <QPushButton>
#include <QInputDialog>
#include <QKeyEvent>

void NoEnterLineEdit::keyPressEvent(QKeyEvent* event) {
  if (event->key() == Qt::Key::Key_Enter || event->key() == Qt::Key::Key_Return) {
    event->accept();
    emit returnPressed();
    return;
  }
  QLineEdit::keyPressEvent(event);
}

constexpr char QuickWhereClauseDialog::WHERE_HIST_SPLIT_CHAR;

void QuickWhereClauseDialog::Init() {
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

  m_strFilterPatternCB = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_strFilterPatternCB)
  m_strFilterPatternCB->setEditable(false);
  m_strFilterPatternCB->insertItem((int) Qt::CaseSensitivity::CaseInsensitive, QuickWhereClauseHelper::FUZZY_LIKE);
  m_strFilterPatternCB->insertItem((int) Qt::CaseSensitivity::CaseSensitive, QuickWhereClauseHelper::FUZZY_INSTR);

  m_whereHistComboBox = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereHistComboBox)
  m_whereHistComboBox->setEditable(false);
  m_whereHistComboBox->setInsertPolicy(QComboBox::InsertAtTop);

  QString hists{Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString()};
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

  m_whereLineEdit = new (std::nothrow) NoEnterLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(m_whereLineEdit)
  m_whereLineEdit->setPlaceholderText("Where Clause here");

  mDialogButtonBox = new (std::nothrow)
      QDialogButtonBox(QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Cancel, this);
  CHECK_NULLPTR_RETURN_VOID(mDialogButtonBox)
  auto* pOkBtn = mDialogButtonBox->button(QDialogButtonBox::StandardButton::Ok);
  pOkBtn->setShortcut(QKeySequence(Qt::Key::Key_F10));
  pOkBtn->setToolTip(QString("<b>%1 (%2)</b><br/> Apply where clause right now.") //
                         .arg(pOkBtn->text(), pOkBtn->shortcut().toString()));
  pOkBtn->setStyleSheet(StyleSheet::SUBMIT_BTN_STYLE);

  m_Layout = new (std::nothrow) QFormLayout{this};
  CHECK_NULLPTR_RETURN_VOID(m_Layout);

  CreatePrivateWidget();
  InitPrivateLayout();

  m_Layout->addRow("WHERE clause:", (QWidget*) nullptr);
  m_Layout->addRow("Pattern:", m_strFilterPatternCB);
  m_Layout->addRow(mWhereClauseHistoryDecTb, m_whereHistComboBox);
  m_Layout->addRow(mWhereClauseHistoryIncTb, m_whereLineEdit);
  m_Layout->addWidget(mDialogButtonBox);

  subscribe();
}

QuickWhereClauseDialog::QuickWhereClauseDialog(QWidget* parent) //
  : QDialog{parent} {
  setWindowIcon(QIcon{":img/QUICK_WHERE_FILTERS"});
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

void QuickWhereClauseDialog::subscribe() {
  connect(m_strFilterPatternCB, &QComboBox::currentTextChanged, this, &QuickWhereClauseDialog::onConditionsChanged);

  connect(mDialogButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(mDialogButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  connect(_RMV_WHERE_CLAUSE_FROM_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onRemoveAHistory);
  connect(_CLEAR_WHERE_CLAUSE_FROM_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onClearHistory);
  connect(_ADD_WHERE_CLAUSE_TO_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onAddAHistory);
  connect(_EDIT_WHERE_CLAUSE_HISTORY, &QAction::triggered, this, &QuickWhereClauseDialog::onEditHistory);

  connect(m_whereHistComboBox, &QComboBox::textActivated, m_whereLineEdit, &QLineEdit::setText);
  PrivateSubscribe();
}

void QuickWhereClauseDialog::SetStrPatternCaseSensitive(Qt::CaseSensitivity caseSen) {
  const int patternsCnt = m_strFilterPatternCB->count();
  if (caseSen >= patternsCnt) {
    LOG_W("Name str pattern index[%d] out of range[0, %d)", (int) caseSen, patternsCnt);
    return;
  }
  m_strFilterPatternCB->setCurrentIndex((int) caseSen);
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
  curHists = QInputDialog::getMultiLineText(this,
                                            "Edit where history Below",
                                            QString::number(beforeRowCnt), //
                                            curHists,
                                            &editOk);
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
