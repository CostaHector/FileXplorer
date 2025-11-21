#include "ScrollBarPolicyMenu.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

ScrollBarPolicyMenu::ScrollBarPolicyMenu(const QString &name, QWidget *parent)
  : QMenu{name, parent} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  CHECK_FALSE_RETURN_VOID(!name.isEmpty());

  _AS_NEEDED = addAction("As Needed");
  CHECK_NULLPTR_RETURN_VOID(_AS_NEEDED);
  _AS_NEEDED->setCheckable(true);

  _ALWAYS_OFF = addAction("Always Off");
  CHECK_NULLPTR_RETURN_VOID(_ALWAYS_OFF);
  _ALWAYS_OFF->setCheckable(true);

  _ALWAYS_ON = addAction("Always On");
  CHECK_NULLPTR_RETURN_VOID(_ALWAYS_ON);
  _ALWAYS_ON->setCheckable(true);

  mScrollBarPolicyIntAction.init(
      {
          {_AS_NEEDED, Qt::ScrollBarPolicy::ScrollBarAsNeeded},   //
          {_ALWAYS_OFF, Qt::ScrollBarPolicy::ScrollBarAlwaysOff}, //
          {_ALWAYS_ON, Qt::ScrollBarPolicy::ScrollBarAlwaysOn},   //
      },                                                         //
      Qt::ScrollBarPolicy::ScrollBarAsNeeded,
      QActionGroup::ExclusionPolicy::Exclusive);

  int defScrollBarPolicy = Configuration().value(GetName(), Qt::ScrollBarPolicy::ScrollBarAsNeeded).toInt();
  QAction *checkedScrollBarPolicyAct = mScrollBarPolicyIntAction.setCheckedIfActionExist(defScrollBarPolicy);
  checkedScrollBarPolicyAct->setChecked(true);

  connect(this, &QMenu::triggered, this, &ScrollBarPolicyMenu::onActionInMenuTriggered);
}

ScrollBarPolicyMenu::~ScrollBarPolicyMenu() {
  const QString &scrollBarPolicyKey = GetName();
  if (!scrollBarPolicyKey.isEmpty()) {
    Configuration().setValue(scrollBarPolicyKey, GetScrollBarPolicy());
  }
}

void ScrollBarPolicyMenu::onActionInMenuTriggered(const QAction *pScrollBarPolicyAct) {
  CHECK_NULLPTR_RETURN_VOID(pScrollBarPolicyAct);
  const Qt::ScrollBarPolicy newMode = mScrollBarPolicyIntAction.act2Enum(pScrollBarPolicyAct);
  emit reqScrollBarPolicyChanged(newMode);
}

Qt::ScrollBarPolicy ScrollBarPolicyMenu::GetScrollBarPolicy() const {
  return mScrollBarPolicyIntAction.curVal();
}
