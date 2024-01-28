#ifndef QUICKWHEREACTIONS_H
#define QUICKWHEREACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QObject>
#include <QToolBar>
#include <QToolButton>

class QuickWhereActions : public QObject {
  Q_OBJECT
 public:
  explicit QuickWhereActions(QObject* parent = nullptr);

  void onClauseClear();

  void onAkaHint(const bool autoCompleteAkaSwitch);


  QAction* RESET_CONDITION;
  QAction* SAVE_WHERE;
  QToolButton* HIST_WHERE;
  QAction* SHOW_HISTORY;
  QAction* CLEAR_ALL_WHERE;
  QAction* AUTO_COMPLETE_AKA_SWITCH;
  QAction* APPLY_AND_CLOSE;

  QMenu* m_historyWhereClauseMenu;
  QToolBar* m_whereClauseTB;

 private:
};

QuickWhereActions& g_quickWhereAg();
#endif  // QUICKWHEREACTIONS_H
