#ifndef FILELEAFACTION_H
#define FILELEAFACTION_H
#include <QAction>
#include <QActionGroup>

class FileLeafActions : public QObject {
 public:
  explicit FileLeafActions(QObject* parent = nullptr);

  QActionGroup* GetLeafTabActions();

  QAction* _SETTINGS{nullptr};
  QAction* _ABOUT_FILE_EXPLORER{nullptr};
  QAction* _LANUAGE{nullptr};

  QActionGroup* _LEAF_FILE{nullptr};
};

FileLeafActions& g_fileLeafActions();

#endif  // FILELEAFACTION_H
