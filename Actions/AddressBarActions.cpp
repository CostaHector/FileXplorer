#include "AddressBarActions.h"

AddressBarActions::AddressBarActions(QObject* parent) : QObject{parent} {
  _BACK_TO = new (std::nothrow) QAction{QIcon(":img/PATH_BACK_TO"), tr("back to")};
  _BACK_TO->setToolTip(QString("<b>%1 (Mouse BackButton)</b><br/> back to last path.").arg(_BACK_TO->text()));

  _FORWARD_TO = new (std::nothrow) QAction{QIcon(":img/PATH_FORWARD_TO"), tr("forward to")};
  _FORWARD_TO->setToolTip(QString("<b>%1 (Mouse ForwardButton)</b><br/> forward to next path.").arg(_FORWARD_TO->text()));

  _UP_TO = new (std::nothrow) QAction{QIcon(":img/PATH_UP_TO"), tr("up to")};
  _UP_TO->setToolTip(QString("<b>%1 (Backspace)</b><br/> up to parent path.").arg(_UP_TO->text()));

  ADDRESS_CONTROLS = GetAddressBarActions();
}

QActionGroup* AddressBarActions::GetAddressBarActions() {
  auto* addressControlAg = new (std::nothrow) QActionGroup(this);
  addressControlAg->addAction(_BACK_TO);
  addressControlAg->addAction(_FORWARD_TO);
  addressControlAg->addAction(_UP_TO);
  addressControlAg->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  return addressControlAg;
}

AddressBarActions& g_addressBarActions() {
  static AddressBarActions ins;
  return ins;
}
