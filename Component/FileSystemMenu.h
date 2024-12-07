#ifndef FILESYSTEMMENU_H
#define FILESYSTEMMENU_H

#include <QMenu>

class FileSystemMenu : public QMenu {
  Q_OBJECT
 public:
  explicit FileSystemMenu(const QString& title, QWidget* parent = nullptr);
  auto GetNewMenu() -> QMenu*;
  auto GetViewMenu() -> QMenu*;

  auto GetRenameMenu() -> QMenu*;

 private:
  QMenu* NEW_MENU;
  QMenu* VIEW_MENU;
};

#endif  // FILESYSTEMMENU_H
