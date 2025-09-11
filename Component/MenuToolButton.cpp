#include "MenuToolButton.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include <QMenu>

MenuToolButton::MenuToolButton(QList<QAction*> dropdownActions,             //
                                       QToolButton::ToolButtonPopupMode popupMode,  //
                                       const Qt::ToolButtonStyle toolButtonStyle,   //
                                       const int iconSize,                          //
                                       QWidget* parent)
  : QToolButton{parent} {
  setPopupMode(popupMode);
  setToolButtonStyle(toolButtonStyle);
  setAutoRaise(true);
  setStyleSheet("QToolButton { max-width: 256px; }");
  setIconSize(QSize{iconSize, iconSize});

  QMenu* pDropdownMenu{new (std::nothrow) QMenu{this}};
  CHECK_NULLPTR_RETURN_VOID(pDropdownMenu);
  for (auto* pAct : dropdownActions) {
    if (pAct == nullptr) {
      pDropdownMenu->addSeparator();
    } else {
      pDropdownMenu->addAction(pAct);
    }
  }
  pDropdownMenu->setToolTipsVisible(true);
  setMenu(pDropdownMenu);
}

void MenuToolButton::SetCaption(const QIcon& icon, const QString& text, const QString& tooltip) {
  setIcon(icon);
  setText(text);
  setToolTip(tooltip);
}

void MenuToolButton::MemorizeCurrentAction(const QString& memoryKey) {
  m_memoryKey = memoryKey;
  connect(this, &QToolButton::triggered,  //
          this, &MenuToolButton::onToolButtonActTriggered);
}

void MenuToolButton::BindForInstantPop() {
  connect(this, &QToolButton::triggered,  //
          this, &MenuToolButton::onInstantPopActTriggered);
}

bool MenuToolButton::FindAndSetDefaultAction(const QString& memoryValue) {
  const auto& actsList = menu()->actions();
  foreach(QAction* act, actsList) {
    if (act->text() == memoryValue) {
      setDefaultAction(act);
      return true;
    }
  }
  LOG_W("Action[%s] not find by from %d actions in menu", qPrintable(memoryValue), actsList.size());
  setDefaultAction(actsList.front());
  return false;
}

void MenuToolButton::onToolButtonActTriggered(QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  setDefaultAction(pAct);
  if (m_memoryKey.isEmpty()) {
    return;
  }
  Configuration().setValue(m_memoryKey, pAct->text());
}

void MenuToolButton::onInstantPopActTriggered(QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  SetCaption(pAct->icon(), pAct->text(), pAct->toolTip());
}
