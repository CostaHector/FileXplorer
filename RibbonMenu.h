#ifndef RIBBONMENU_H
#define RIBBONMENU_H

#include <QTabWidget>
#include <QToolBar>
#include <QLineEdit>

class RibbonMenu : public QTabWidget {
 public:
  RibbonMenu();
  QToolBar* GetMenuRibbonCornerWid(QWidget* attached = nullptr);

  QToolBar* LeafFile() const;
  QToolBar* LeafHome() const;
  QToolBar* LeafShare() const;
  QToolBar* LeafView() const;
  QToolBar* LeafDatabase();
  QToolBar* LeafMediaTools() const;

  void Subscribe();

 private:
  QToolBar* leafFileWid;
  QToolBar* leafHomeWid;
  QToolBar* leafShareWid;
  QToolBar* leafViewWid;
  QToolBar* leafDatabaseWid;

  QToolBar* leafMediaWid;
  QToolBar* menuRibbonCornerWid;

  static constexpr int MAX_WIDGET_HEIGHT = 400;
};

#endif  // RIBBONMENU_H
