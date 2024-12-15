#ifndef DROPLISTTOOLBUTTON_H
#define DROPLISTTOOLBUTTON_H
#include "PublicVariable.h"
#include <QToolButton>
#include <QString>
#include <QList>
class QAction;
class QActionGroup;
QToolButton* DropListToolButton(QAction* defaultAction,
                                QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                                const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                const int iconSize = TABS_ICON_IN_MENU_1x1);
QToolButton* DropListToolButtonWithoutDefAction(const QIcon &icon, const QString &text, const QString& toolTip,
                                QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode = QToolButton::ToolButtonPopupMode::InstantPopup,
                                const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                const int iconSize = TABS_ICON_IN_MENU_1x1);
auto FindQActionFromQActionGroupByActionName(const QString& actionName, QActionGroup* ag) -> QAction*;
bool EnableAutoChangeDefaultActionInToolButton(QToolButton* pTb, const QString& MEMORY_KEY);
#endif // DROPLISTTOOLBUTTON_H
