#include "QuickWhereActions.h"
#include "PublicVariable.h"

#include <QDesktopServices>

QuickWhereActions::QuickWhereActions(QObject* parent)
    : QObject{parent},
      RESET_CONDITION(new QAction(QIcon(":/themes/RESET"), "Reset", this)),
      SAVE_WHERE(new QAction(QIcon(":/themes/PATH_BACK_TO"), "Save", this)),
      HIST_WHERE(new QToolButton),
      SHOW_HISTORY(new QAction(QIcon(":/themes/WHERE_CLAUSE_HISTORY"), "HIST", this)),
      CLEAR_ALL_WHERE(new QAction(QIcon(":/themes/CLEAR_VIDEOS_LIST"), "Clear", this)),
      AUTO_COMPLETE_AKA_SWITCH(new QAction(QIcon(":/themes/PERFORMERS_LIST_HINT"), "Completer", this)),
      APPLY_AND_CLOSE(new QAction(QIcon(":/themes/SAVED"), "Apply", this)),
      UPDATE_AKA(new QAction(QIcon(":/themes/RELOAD_JOSN_FROM_FILE"), "Update AKA", this)),
      OPEN_AKA_TEXT(new QAction(QIcon(":/themes/NEW_TEXT_DOCUMENT"), "Open AKA", this)),
      m_historyWhereClauseMenu(new QMenu(HIST_WHERE)),
      m_whereClauseTB(new QToolBar("Quick")) {
  m_historyWhereClauseMenu->setToolTipsVisible(true);
  for (const QString& where :
       PreferenceSettings().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString().split('\n')) {
    m_historyWhereClauseMenu->addAction(new QAction(where, m_historyWhereClauseMenu));
  }

  AUTO_COMPLETE_AKA_SWITCH->setCheckable(true);
  AUTO_COMPLETE_AKA_SWITCH->setChecked(
      PreferenceSettings().value(MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA.name, MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA.v).toBool());
  AUTO_COMPLETE_AKA_SWITCH->setToolTip(QString("<b>%1 (%2)</b><br/>Auto replace performers with its ALL AKA")
                                           .arg(AUTO_COMPLETE_AKA_SWITCH->text(), AUTO_COMPLETE_AKA_SWITCH->shortcut().toString()));
  APPLY_AND_CLOSE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  APPLY_AND_CLOSE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Apply and close this window").arg(APPLY_AND_CLOSE->text(), APPLY_AND_CLOSE->shortcut().toString()));

  HIST_WHERE->setDefaultAction(SHOW_HISTORY);
  HIST_WHERE->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  HIST_WHERE->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  HIST_WHERE->setMenu(m_historyWhereClauseMenu);

  m_whereClauseTB->addAction(OPEN_AKA_TEXT);
  m_whereClauseTB->addAction(UPDATE_AKA);
  m_whereClauseTB->addAction(AUTO_COMPLETE_AKA_SWITCH);
  m_whereClauseTB->addSeparator();
  m_whereClauseTB->addAction(CLEAR_ALL_WHERE);
  m_whereClauseTB->addWidget(HIST_WHERE);
  m_whereClauseTB->addAction(SAVE_WHERE);
  m_whereClauseTB->addSeparator();
  m_whereClauseTB->addAction(RESET_CONDITION);
  m_whereClauseTB->addAction(APPLY_AND_CLOSE);
  m_whereClauseTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  connect(CLEAR_ALL_WHERE, &QAction::triggered, this, &QuickWhereActions::onClauseClear);
  connect(AUTO_COMPLETE_AKA_SWITCH, &QAction::triggered, this, &QuickWhereActions::onAkaHint);
  connect(OPEN_AKA_TEXT, &QAction::triggered, this, [this]() { QDesktopServices::openUrl(QUrl::fromLocalFile(SystemPath::AKA_PERFORMERS_TXT)); });
}

void QuickWhereActions::onClauseClear() {
  m_historyWhereClauseMenu->clear();
  PreferenceSettings().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, "");
}

void QuickWhereActions::onAkaHint(const bool autoCompleteAkaSwitch) {
  PreferenceSettings().setValue(MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA.name, autoCompleteAkaSwitch);
}

QuickWhereActions& g_quickWhereAg() {
  static QuickWhereActions quickWhereActions;
  return quickWhereActions;
}
