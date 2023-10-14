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

#include "PublicVariable.h"
#include "PublicTool.h"
#include "Actions/RenameActions.h"
#include <QToolButton>
#include <QMenu>

QToolButton* DropListToolButton(QAction* defaultAction, QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                                const QString& updateToolTip="",
                                const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                const int iconSide = TABS_ICON_IN_MENU_1x1){
    QToolButton* tb = new QToolButton();
    if (dropdownActions.isEmpty()){
        return nullptr;
    }
    if (defaultAction == nullptr){
        defaultAction = dropdownActions[0];
    }

    tb->setDefaultAction(defaultAction);
    if (not updateToolTip.isEmpty()){
        defaultAction->setToolTip(updateToolTip);
    }
    tb->setPopupMode(popupMode);
    tb->setToolButtonStyle(toolButtonStyle);
    tb->setAutoRaise(true);
    tb->setStyleSheet("QToolButton { max-width: 128px; }");
    tb->setIconSize(QSize(iconSide, iconSide));

    QMenu* mn = new QMenu(tb);
    mn->addActions(dropdownActions);
    mn->setToolTipsVisible(true);
    tb->setMenu(mn);
    return tb;
}


QToolBar *RibbonMenu::LeafHome() const {
    auto* defaultRenameAction = FindQActionFromQActionGroupByActionName(
        PreferenceSettings().value(MemoryKey::DEFAULT_RENAME_CHOICE.name, MemoryKey::DEFAULT_RENAME_CHOICE.v).toString(),
        g_renameAg().RENAME_RIBBONS);
    QToolButton* renameToolButton = DropListToolButton(defaultRenameAction,
                                                       g_renameAg().RENAME_RIBBONS->actions(),
                                                       QToolButton::MenuButtonPopup);
    auto ChangeDefaultRenameActionInToolButton = [renameToolButton](QAction* newDefaultAction)->void{
        renameToolButton->setDefaultAction(newDefaultAction);
        PreferenceSettings().setValue(MemoryKey::DEFAULT_RENAME_CHOICE.name, newDefaultAction->text());
    };
    connect(renameToolButton, &QToolButton::triggered, this, ChangeDefaultRenameActionInToolButton);


    QToolBar* leafHomeWid = new QToolBar("LeafHome");

    leafHomeWid->addSeparator();
    leafHomeWid->addWidget(renameToolButton);
    leafHomeWid->addSeparator();

    return leafHomeWid;
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
