#include "OfficeStyleMenu.h"

OfficeStyleMenu::OfficeStyleMenu():
    leafFileWid(LeafFile()),
    leafHomeWid(LeafHome()),
    leafShareWid(LeafShare()),
    leafViewWid(LeafView()),
    leafDatabaseWid(LeafDatabase()),
    leafMediaWid(LeafMediaTools())
{
    addTab(leafFileWid, "&File");
    addTab(leafHomeWid, "&Home");
    addTab(leafShareWid, "&Share");
    addTab(leafViewWid, "&View");
    addTab(leafDatabaseWid, "&Database");
    addTab(leafMediaWid, "&Media");

    setCurrentWidget(leafFileWid);
}
#include "FileLeafAction.h"
QToolBar *OfficeStyleMenu::LeafFile() const
{
    QToolBar* leafFileWid(new QToolBar);
    leafFileWid->addActions(FileLeafAction::ag()->actions());
    leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return leafFileWid;
}

QToolBar *OfficeStyleMenu::LeafHome() const
{
    return new QToolBar();
}

QToolBar *OfficeStyleMenu::LeafShare() const
{
    return new QToolBar();
}

QToolBar *OfficeStyleMenu::LeafView() const
{
    return new QToolBar();
}

QToolBar *OfficeStyleMenu::LeafDatabase() const
{
    return new QToolBar();
}

QToolBar *OfficeStyleMenu::LeafMediaTools() const
{
    return new QToolBar();
}
