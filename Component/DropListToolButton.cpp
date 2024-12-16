#include "DropListToolButton.h"
#include <QAction>
#include <QActionGroup>
#include <QMenu>
QToolButton* DropListToolButton(QAction* defaultAction,
                                QList<QAction*> dropdownActions,
                                QToolButton::ToolButtonPopupMode popupMode,
                                const Qt::ToolButtonStyle toolButtonStyle,
                                const int iconSize) {
  if (dropdownActions.isEmpty() || defaultAction == nullptr) {
    qCritical("no action exist");
    return nullptr;
  }
  QToolButton* pDropdownButton{new (std::nothrow) QToolButton};
  if (pDropdownButton == nullptr) {
    qCritical("pDropdownButton is nullptr");
    return nullptr;
  }
  pDropdownButton->setDefaultAction(defaultAction);
  pDropdownButton->setPopupMode(popupMode);
  pDropdownButton->setToolButtonStyle(toolButtonStyle);
  pDropdownButton->setAutoRaise(true);
  pDropdownButton->setStyleSheet("QToolButton { max-width: 256px; }");
  pDropdownButton->setIconSize(QSize(iconSize, iconSize));

  QMenu* pDropdownMenu{new (std::nothrow) QMenu(pDropdownButton)};
  if (pDropdownMenu == nullptr) {
    qCritical("pDropdownMenu is nullptr");
    return nullptr;
  }
  for (auto* pAct : dropdownActions) {
    if (pAct == nullptr) {
      pDropdownMenu->addSeparator();
    } else {
      pDropdownMenu->addAction(pAct);
    }
  }
  pDropdownMenu->setToolTipsVisible(true);
  pDropdownButton->setMenu(pDropdownMenu);
  return pDropdownButton;
}

QToolButton* DropListToolButtonWithoutDefAction(const QIcon &icon, const QString &text, const QString& toolTip,
                                                QList<QAction*> dropdownActions,
                                                QToolButton::ToolButtonPopupMode popupMode,
                                                const Qt::ToolButtonStyle toolButtonStyle,
                                                const int iconSize) {
  if (dropdownActions.isEmpty()) {
    qCritical("no action exist");
    return nullptr;
  }

  QToolButton* pDropdownButton{new (std::nothrow) QToolButton};
  if (pDropdownButton == nullptr) {
    qCritical("pDropdownButton is nullptr");
    return nullptr;
  }
  pDropdownButton->setIcon(icon);
  pDropdownButton->setText(text);
  pDropdownButton->setToolTip(toolTip);
  pDropdownButton->setPopupMode(popupMode);
  pDropdownButton->setToolButtonStyle(toolButtonStyle);
  pDropdownButton->setAutoRaise(true);
  pDropdownButton->setStyleSheet("QToolButton { max-width: 256px; }");
  pDropdownButton->setIconSize(QSize(iconSize, iconSize));

  QMenu* pDropdownMenu{new (std::nothrow) QMenu(pDropdownButton)};
  if (pDropdownMenu == nullptr) {
    qCritical("pDropdownMenu is nullptr");
    return nullptr;
  }
  for (auto* pAct : dropdownActions) {
    if (pAct == nullptr) {
      pDropdownMenu->addSeparator();
    } else {
      pDropdownMenu->addAction(pAct);
    }
  }
  pDropdownMenu->setToolTipsVisible(true);
  pDropdownButton->setMenu(pDropdownMenu);
  return pDropdownButton;
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

bool EnableAutoChangeDefaultActionInToolButton(QToolButton* pTb, const QString& memoryKey) {
  if (pTb == nullptr) {
    qCritical("pTb is nullptr");
    return false;
  }
  auto onToolButtonActTriggered = [pTb, memoryKey](QAction* triggeredAct) -> void {
    if (pTb != nullptr) {
      pTb->setDefaultAction(triggeredAct);
    } else {
      qCritical("pTb is nullptr");
    }
    if (!memoryKey.isEmpty()) {
      PreferenceSettings().setValue(memoryKey, triggeredAct->text());
    }
  };
  QToolButton::connect(pTb, &QToolButton::triggered, onToolButtonActTriggered);
  return true;
}
