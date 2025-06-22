#ifndef FILESYSTEMMENU_H
#define FILESYSTEMMENU_H

#include <QMenu>

class RightClickMenu : public QMenu {
 public:
  explicit RightClickMenu(const QString& title, QWidget* parent = nullptr);
  QMenu* GetNewMenu();
  QMenu* GetRenameMenu();

 private:
  QMenu* NEW_MENU;
};

#endif  // FILESYSTEMMENU_H
