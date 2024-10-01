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
                                const QString& updateToolTip = "",
                                const Qt::ToolButtonStyle toolButtonStyle = Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
                                const int iconSize = TABS_ICON_IN_MENU_1x1);

auto FindQActionFromQActionGroupByActionName(const QString& actionName, QActionGroup* ag) -> QAction*;
#endif // DROPLISTTOOLBUTTON_H
