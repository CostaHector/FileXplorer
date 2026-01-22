#ifndef FILERENAMERULERACTIONS_H
#define FILERENAMERULERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class FileRenameRulerActions : public QObject {
  Q_OBJECT
 public:
  explicit FileRenameRulerActions(QObject* parent = nullptr);
  void subscribe();

  QAction* _NAME_RULER{nullptr};

  QAction* _EDIT_STUDIOS{nullptr};
  QAction* _RELOAD_STUDIOS{nullptr};

  QAction* _EDIT_ACTORS{nullptr};
  QAction* _RELOAD_ACTORS{nullptr};

  QAction* _EDIT_ACTORS_ALIAS{nullptr};
  QAction* _RELOAD_ACTORS_ALIAS{nullptr};

  QAction* _RENAME_RULE_STAT{nullptr};

  QList<QAction*> NAME_RULES_ACTIONS_LIST;
 private:
  static bool onEditLocalFile(const QString& rel2File);

  static bool onEditStudiosListFile();
  static bool onEditActorsListFile();
  static bool onEditActorsAliasListFile();

  static int onReloadStudiosListFile();
  static int onReloadActorsListFile();
  static int onReloadActorsAliasListFile();

  static void onShowRenameRuleStatistics();
};

FileRenameRulerActions& g_NameRulerActions();

#endif  // FILERENAMERULERACTIONS_H
