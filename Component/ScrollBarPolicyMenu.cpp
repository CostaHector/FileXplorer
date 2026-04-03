#include "ScrollBarPolicyMenu.h"
#include "MemoryKey.h"
#include "PublicMacro.h"

ScrollBarPolicyMenu::ScrollBarPolicyMenu(const QString& menuName, const QString& belongToName, QWidget* parent)
    : QMenu{menuName, parent}, m_memoryName{belongToName + "/ScrollBarPolicy"} {
  CHECK_NULLPTR_RETURN_VOID(parent);
  CHECK_FALSE_RETURN_VOID(!belongToName.isEmpty());

  const bool isHor{belongToName.contains("hor", Qt::CaseInsensitive)};
  const QIcon alwaysOnIcon{isHor ? ":img/SCROLL_BAR_POLICY_HOR" : ":img/SCROLL_BAR_POLICY_VER"};
  setIcon(alwaysOnIcon);

  _AS_NEEDED = addAction(tr("As Needed"));
  CHECK_NULLPTR_RETURN_VOID(_AS_NEEDED);
  _AS_NEEDED->setCheckable(true);

  _ALWAYS_OFF = addAction(tr("Always Off"));
  CHECK_NULLPTR_RETURN_VOID(_ALWAYS_OFF);
  _ALWAYS_OFF->setCheckable(true);

  _ALWAYS_ON = addAction(alwaysOnIcon, tr("Always On"));
  CHECK_NULLPTR_RETURN_VOID(_ALWAYS_ON);
  _ALWAYS_ON->setCheckable(true);

  mScrollBarPolicyIntAction.init(
      {
          {_AS_NEEDED, Qt::ScrollBarPolicy::ScrollBarAsNeeded},    //
          {_ALWAYS_OFF, Qt::ScrollBarPolicy::ScrollBarAlwaysOff},  //
          {_ALWAYS_ON, Qt::ScrollBarPolicy::ScrollBarAlwaysOn},    //
      },                                                           //
      Qt::ScrollBarPolicy::ScrollBarAsNeeded, QActionGroup::ExclusionPolicy::Exclusive);

  int defScrollBarPolicy = Configuration().value(GetName(), Qt::ScrollBarPolicy::ScrollBarAsNeeded).toInt();
  mScrollBarPolicyIntAction.setCheckedIfActionExist(defScrollBarPolicy);

  connect(this, &QMenu::triggered, this, &ScrollBarPolicyMenu::onActionInMenuTriggered);
}

ScrollBarPolicyMenu::~ScrollBarPolicyMenu() {
  Configuration().setValue(GetName(), GetScrollBarPolicy());
}

void ScrollBarPolicyMenu::onActionInMenuTriggered(const QAction* pScrollBarPolicyAct) {
  CHECK_NULLPTR_RETURN_VOID(pScrollBarPolicyAct);
  const Qt::ScrollBarPolicy newMode = mScrollBarPolicyIntAction.act2Enum(pScrollBarPolicyAct);
  emit reqScrollBarPolicyChanged(newMode);
}

Qt::ScrollBarPolicy ScrollBarPolicyMenu::GetScrollBarPolicy() const {
  return mScrollBarPolicyIntAction.curVal();
}
