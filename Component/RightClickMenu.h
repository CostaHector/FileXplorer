#ifndef FILESYSTEMMENU_H
#define FILESYSTEMMENU_H

#include <QMenu>

class RightClickMenu : public QMenu {
 public:
  explicit RightClickMenu(const QString& title, QWidget* parent = nullptr);
};

#endif  // FILESYSTEMMENU_H
