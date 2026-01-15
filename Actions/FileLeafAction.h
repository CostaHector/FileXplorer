#ifndef FILELEAFACTION_H
#define FILELEAFACTION_H
#include <QAction>
#include <QActionGroup>
#include <QToolButton>

class FileLeafActions : public QObject {
  Q_OBJECT
 public:
  explicit FileLeafActions(QObject* parent = nullptr);

  QAction* _SETTINGS{nullptr};
  QAction* _PWD_BOOK{nullptr};
  QAction* _ABOUT_FILE_EXPLORER{nullptr};
  QAction* _LANUAGE{nullptr};
  QAction* _CPU_MEMORY_USAGE_MONITOR{nullptr};

  QAction* _ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU{nullptr};
  QAction* _RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU{nullptr};
  QToolButton* GetSystemContextMenu(QWidget* parent);

  QActionGroup* _LEAF_FILE{nullptr};
};

FileLeafActions& g_fileLeafActions();

#endif  // FILELEAFACTION_H
