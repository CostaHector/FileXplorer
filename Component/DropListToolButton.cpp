#include "DropListToolButton.h"
#include <QAction>
#include <QActionGroup>
#include <QMenu>
QToolButton* DropListToolButton(QAction* defaultAction,
                                QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode,
                                const QString& updateToolTip,
                                const Qt::ToolButtonStyle toolButtonStyle,
                                const int iconSize) {
  if (dropdownActions.isEmpty()) {
    return nullptr;
  }
  if (defaultAction == nullptr) {
    defaultAction = dropdownActions[0];
  }

  QToolButton* tb = new QToolButton;
  tb->setDefaultAction(defaultAction);
  if (not updateToolTip.isEmpty()) {
    defaultAction->setToolTip(updateToolTip);
  }
  tb->setPopupMode(popupMode);
  tb->setToolButtonStyle(toolButtonStyle);
  tb->setAutoRaise(true);
  tb->setStyleSheet("QToolButton { max-width: 256px; }");
  tb->setIconSize(QSize(iconSize, iconSize));

  QMenu* mn = new QMenu(tb);
  mn->addActions(dropdownActions);
  mn->setToolTipsVisible(true);
  tb->setMenu(mn);
  return tb;
}

auto FindQActionFromQActionGroupByActionName(const QString& actionName, QActionGroup* ag) -> QAction* {
  if (ag == nullptr) {
    return nullptr;
  }
  for (QAction* act : ag->actions()) {
    if (act->text() == actionName) {
      return act;
    }
  }
  return nullptr;
}
