#include "AddressBarActions.h"

AddressBarActions::AddressBarActions(QObject* parent)
    : QObject{parent},
      _BACK_TO{new QAction{QIcon(":/themes/PATH_BACK_TO"), tr("back to")}},
      _FORWARD_TO{new QAction{QIcon(":/themes/PATH_FORWARD_TO"), tr("forward to")}},
      _UP_TO{new QAction{QIcon(":/themes/PATH_UP_TO"), tr("up to")}},
      ADDRESS_CONTROLS(GetAddressBarActions()) {}

QActionGroup* AddressBarActions::GetAddressBarActions() {
  auto* addressControlAg = new QActionGroup(this);
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
