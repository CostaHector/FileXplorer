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

  QAction* _EDIT_PERFS{nullptr};
  QAction* _RELOAD_PERFS{nullptr};

  QAction* _EDIT_PERF_AKA{nullptr};
  QAction* _RELOAD_PERF_AKA{nullptr};

  QAction* _RENAME_RULE_STAT{nullptr};

  QList<QAction*> NAME_RULES_ACTIONS_LIST;
 private:
  void onEditLocalFile(const QString& rel2File);
  std::pair<bool, QString> mLastTimeEditFileInfo{false, ""};
  int mLastTimeCntDelta{-1};
};

FileRenameRulerActions& g_NameRulerActions();

#endif  // FILERENAMERULERACTIONS_H
