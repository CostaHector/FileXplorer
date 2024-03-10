#ifndef MOVIEDATABASEMENU_H
#define MOVIEDATABASEMENU_H

#include <QMenu>
class MovieDatabaseMenu : public QMenu {
 public:
  MovieDatabaseMenu(const QString& title, QWidget* parent = nullptr);
};

#endif  // MOVIEDATABASEMENU_H
