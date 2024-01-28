#ifndef FILELEAFACTION_H
#define FILELEAFACTION_H
#include <QAction>
#include <QActionGroup>

class FileLeafActions : public QObject {
  Q_OBJECT
 public:
  explicit FileLeafActions(QObject* parent = nullptr);

  QActionGroup* GetLeafTabActions();
  QAction* _PREFERENCE_SETTING;
  QAction* _ABOUT_FILE_EXPLORER;
  QAction* _LANUAGE;

  QActionGroup* LEAF_FILE;
};

FileLeafActions& g_fileLeafActions();

#endif  // FILELEAFACTION_H
