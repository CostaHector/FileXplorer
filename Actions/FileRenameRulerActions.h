#ifndef FILERENAMERULERACTIONS_H
#define FILERENAMERULERACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QObject>

class FileRenameRulerActions : public QObject {
 public:
  explicit FileRenameRulerActions(QObject* parent = nullptr);

  void subscribe();

  QAction* _EDIT_STUDIOS{nullptr};
  QAction* _RELOAD_STUDIOS{nullptr};

  QAction* _EDIT_PERFS{nullptr};
  QAction* _RELOAD_PERFS{nullptr};

  QAction* _EDIT_PERF_AKA{nullptr};
  QAction* _RELOAD_PERF_AKA{nullptr};

  QAction* _RENAME_RULE_STAT{nullptr};

 private:
  void onEditLocalFile(const QString& rel2File);
  std::pair<bool, QString> mLastTimeEditFileInfo;
};

FileRenameRulerActions& g_ArrangeActions();

#endif  // FILERENAMERULERACTIONS_H
