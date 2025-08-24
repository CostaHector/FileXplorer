#include "QuickWhereActions.h"

#include "MemoryKey.h"
#include "PublicVariable.h"
#include <QDesktopServices>

QuickWhereActions::QuickWhereActions(QObject* parent)
    : QObject{parent},
      SAVE_WHERE(new (std::nothrow) QAction(QIcon(":img/PATH_BACK_TO"), tr("Save"), this)),
      HIST_WHERE(new QToolButton),
      SHOW_HISTORY(new (std::nothrow) QAction(QIcon(":img/WHERE_CLAUSE_HISTORY"), tr("HIST"), this)),
      CLEAR_ALL_WHERE(new (std::nothrow) QAction(QIcon(":img/CLEAR_VIDEOS_LIST"), tr("Clear"), this)),
      AUTO_COMPLETE_AKA_SWITCH(new (std::nothrow) QAction(QIcon(""), tr("Aka Completer"), this)),
      APPLY_AND_CLOSE(new (std::nothrow) QAction(QIcon(":img/SAVED"), tr("Apply"), this)),
      m_historyWhereClauseMenu(new QMenu(HIST_WHERE)),
      m_whereClauseTB(new QToolBar("Quick")) {
  m_historyWhereClauseMenu->setToolTipsVisible(true);
  const QString& histories = Configuration().value(MemoryKey::WHERE_CLAUSE_HISTORY.name, MemoryKey::WHERE_CLAUSE_HISTORY.v).toString();
  for (const QString& where : histories.split('\n')) {
    m_historyWhereClauseMenu->addAction(new (std::nothrow) QAction(where, m_historyWhereClauseMenu));
  }

  AUTO_COMPLETE_AKA_SWITCH->setCheckable(true);
  AUTO_COMPLETE_AKA_SWITCH->setChecked(
      Configuration().value(MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA.name, MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA.v).toBool());
  AUTO_COMPLETE_AKA_SWITCH->setToolTip(QString("<b>%1 (%2)</b><br/>Auto replace performers with its ALL AKA")
                                           .arg(AUTO_COMPLETE_AKA_SWITCH->text(), AUTO_COMPLETE_AKA_SWITCH->shortcut().toString()));
  APPLY_AND_CLOSE->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  APPLY_AND_CLOSE->setToolTip(
      QString("<b>%1 (%2)</b><br/> Apply and close this window").arg(APPLY_AND_CLOSE->text(), APPLY_AND_CLOSE->shortcut().toString()));

  HIST_WHERE->setDefaultAction(SHOW_HISTORY);
  HIST_WHERE->setPopupMode(QToolButton::ToolButtonPopupMode::InstantPopup);
  HIST_WHERE->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
  HIST_WHERE->setMenu(m_historyWhereClauseMenu);

  m_whereClauseTB->addAction(AUTO_COMPLETE_AKA_SWITCH);
  m_whereClauseTB->addSeparator();
  m_whereClauseTB->addAction(CLEAR_ALL_WHERE);
  m_whereClauseTB->addWidget(HIST_WHERE);
  m_whereClauseTB->addAction(SAVE_WHERE);
  m_whereClauseTB->addSeparator();
  m_whereClauseTB->addAction(APPLY_AND_CLOSE);
  m_whereClauseTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);

  connect(CLEAR_ALL_WHERE, &QAction::triggered, this, &QuickWhereActions::onClauseClear);
  connect(AUTO_COMPLETE_AKA_SWITCH, &QAction::triggered, this, &QuickWhereActions::onAkaHint);
}

void QuickWhereActions::onClauseClear() {
  m_historyWhereClauseMenu->clear();
  Configuration().setValue(MemoryKey::WHERE_CLAUSE_HISTORY.name, "");
}

void QuickWhereActions::onAkaHint(const bool autoCompleteAkaSwitch) {
  Configuration().setValue(MemoryKey::QUICK_WHERE_CLAUSE_AUTO_COMLETE_AKA.name, autoCompleteAkaSwitch);
}

QuickWhereActions& g_quickWhereAg() {
  static QuickWhereActions quickWhereActions;
  return quickWhereActions;
}
