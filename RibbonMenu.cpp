#include "RibbonMenu.h"

RibbonMenu::RibbonMenu():
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
QToolBar *RibbonMenu::LeafFile() const
{
    QToolBar* leafFileWid(new QToolBar);
    leafFileWid->addActions(FileLeafAction::ag()->actions());
    leafFileWid->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextUnderIcon);
    return leafFileWid;
}

QToolBar *RibbonMenu::LeafHome() const
{
    return new QToolBar();
}

QToolBar *RibbonMenu::LeafShare() const
{
    return new QToolBar();
}

QToolBar *RibbonMenu::LeafView() const
{
    return new QToolBar();
}

QToolBar *RibbonMenu::LeafDatabase() const
{
    return new QToolBar();
}

QToolBar *RibbonMenu::LeafMediaTools() const
{
    return new QToolBar();
}
