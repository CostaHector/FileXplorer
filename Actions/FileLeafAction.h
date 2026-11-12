#ifndef FILELEAFACTION_H
#define FILELEAFACTION_H
#include <QAction>
#include <QActionGroup>
#include <QToolButton>

class FileLeafActions : public QObject {
  Q_OBJECT
 public:
  static FileLeafActions& GetInst();

  QAction* _SETTINGS{nullptr};
  QAction* _PWD_BOOK{nullptr}, *_PWD_BOOK_SERVER{nullptr};
  QAction* _ABOUT_FILE_EXPLORER{nullptr};
  QAction* _LANUAGE{nullptr};
  QAction* _CPU_MEMORY_USAGE_MONITOR{nullptr};

  QAction* _ADD_THIS_PROGRAM_TO_SYSTEM_CONTEXT_MENU{nullptr};
  QAction* _RMV_THIS_PROGRAM_FROM_SYSTEM_CONTEXT_MENU{nullptr};
  QToolButton* GetSystemContextMenu(QWidget* parent);

  QActionGroup* _LEAF_FILE{nullptr};

private:
  explicit FileLeafActions(QObject* parent = nullptr);
};

#endif  // FILELEAFACTION_H
