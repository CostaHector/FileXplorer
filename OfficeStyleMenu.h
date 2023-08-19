#ifndef OFFICESTYLEMENU_H
#define OFFICESTYLEMENU_H

#include <QTabWidget>
#include <QToolBar>

class OfficeStyleMenu : public QTabWidget
{
public:
    OfficeStyleMenu();
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

#endif // OFFICESTYLEMENU_H
