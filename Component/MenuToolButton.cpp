#include "MenuToolButton.h"
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
  if (!tooltip.isEmpty()) {
    setToolTip(tooltip);
  }
}

void MenuToolButton::InitDefaultActionFromQSetting(const KV& kv, bool enablePersistentBehavior) {
  const QString& memoryKey = kv.name;
  const QString& initalValue = Configuration().value(kv.name, kv.v).toString();
  if (memoryKey.isEmpty()) {
    LOG_W("memoryKey[%s]:memoryValue[%s] is Empty", qPrintable(memoryKey), qPrintable(initalValue));
  }

  auto* pMenu = menu();
  if (pMenu == nullptr || pMenu->isEmpty()) {
    LOG_W("No menu attached to tool button or No action attached to Menu");
    return;
  }
  const auto& actsList = pMenu->actions();
  foreach(QAction* act, actsList) {
    if (act->text() == initalValue) {
      setDefaultAction(act);
      break;
    }
  }

  if (defaultAction() == nullptr) {
    LOG_W("Action[%s] not find from %d actions in menu, use first by default", qPrintable(initalValue), actsList.size());
    setDefaultAction(actsList.front());
  }

  if (!enablePersistentBehavior) {
    return;
  }
  connect(this, &QToolButton::triggered, this, [this, memoryKey](QAction* pAct){
    CHECK_NULLPTR_RETURN_VOID(pAct);
    setDefaultAction(pAct);
    Configuration().setValue(memoryKey, pAct->text());
  });
}

void MenuToolButton::UpdateCaptionForInstantPopMode() { // for user experience after click one action. update Caption instead of defaultAction
  connect(this, &QToolButton::triggered, this, [this](const QAction* pAct){
    CHECK_NULLPTR_RETURN_VOID(pAct);
    SetCaption(pAct->icon(), pAct->text(), pAct->toolTip());
  });
}
