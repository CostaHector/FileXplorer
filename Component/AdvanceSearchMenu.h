#ifndef ADVANCESEARCHMENU_H
#define ADVANCESEARCHMENU_H

#include <QAction>
#include <QMenu>

class AdvanceSearchMenu : public QMenu {
  Q_OBJECT
 public:
  explicit AdvanceSearchMenu(const QString& title, QWidget* parent = nullptr);

  QAction* _FORCE_REFRESH_SEARCH_SOURCE = new QAction(QIcon(":img/REFRESH_SEARCH_SOURCE"), tr("Refresh search source"), this);
 private:
};

#endif  // ADVANCESEARCHMENU_H
