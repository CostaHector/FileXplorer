#ifndef RIBBONMENU_H
#define RIBBONMENU_H

#include <QTabWidget>
#include <QToolBar>

class RibbonMenu : public QTabWidget
{
public:
    RibbonMenu();
    QToolBar* LeafFile()const;
    QToolBar* LeafHome()const;
    QToolBar* LeafShare()const;
    QToolBar* LeafView()const;
    QToolBar* LeafDatabase()const;
    QToolBar* LeafMediaTools()const;
private:
    QToolBar* leafFileWid;
    QToolBar* leafHomeWid;
    QToolBar* leafShareWid;
    QToolBar* leafViewWid;
    QToolBar* leafDatabaseWid;
    QToolBar* leafMediaWid;
};

#endif // RIBBONMENU_H
