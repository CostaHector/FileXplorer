#include "DropListToolButton.h"
#include "MemoryKey.h"
#include <QMenu>

DropdownToolButton::DropdownToolButton(QList<QAction*> dropdownActions,             //
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
  if (pDropdownMenu == nullptr) {
    qCritical("pDropdownMenu is nullptr");
    return;
  }
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

void DropdownToolButton::SetCaption(const QIcon& icon, const QString& text, const QString& tooltip) {
  setIcon(icon);
  setText(text);
  setToolTip(tooltip);
}

void DropdownToolButton::MemorizeCurrentAction(const QString& memoryKey) {
  m_memoryKey = memoryKey;
  QToolButton::connect(this, &QToolButton::triggered,  //
                       this, &DropdownToolButton::onToolButtonActTriggered);
}

bool DropdownToolButton::FindAndSetDefaultAction(const QString& memoryValue) {
  auto* pMenu = menu();
  const auto& actsList = pMenu->actions();
  if (actsList.isEmpty()) {
    qWarning("No actions found");
    return false;
  }
  foreach(QAction* act, actsList) {
    if (act->text() == memoryValue) {
      setDefaultAction(act);
      return true;
    }
  }
  qWarning("default action not find by memoryValue[%s] from %d actions",//
         qPrintable(memoryValue), actsList.size());
  setDefaultAction(actsList.front());
  return false;
}

void DropdownToolButton::onToolButtonActTriggered(QAction* pAct) {
  if (pAct == nullptr) {
    qCritical("pAct is nullptr");
    return;
  }
  setDefaultAction(pAct);
  if (m_memoryKey.isEmpty()) {
    return;
  }
  Configuration().setValue(m_memoryKey, pAct->text());
}
