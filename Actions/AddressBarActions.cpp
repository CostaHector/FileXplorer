#include "AddressBarActions.h"

AddressBarActions::AddressBarActions(QObject* parent) : QObject{parent} {
  _BACK_TO = new QAction{QIcon(":/themes/PATH_BACK_TO"), tr("back to")};
  _FORWARD_TO = new QAction{QIcon(":/themes/PATH_FORWARD_TO"), tr("forward to")};
  _UP_TO = new QAction{QIcon(":/themes/PATH_UP_TO"), tr("up to")};
  ADDRESS_CONTROLS = GetAddressBarActions();

  _LAST_FOLDER = new QAction{QIcon(""), "^"};
  _NEXT_FOLDER = new QAction{QIcon(""), "v"};
  _FOLDER_IT_CONTROLS = GetFolderIteratorActions();

  _LAST_FOLDER->setToolTip("last folder of parent path in current view");
  _NEXT_FOLDER->setToolTip("next folder of parent path in current view");
}

QActionGroup* AddressBarActions::GetAddressBarActions() {
  auto* addressControlAg = new QActionGroup(this);
  addressControlAg->addAction(_BACK_TO);
  addressControlAg->addAction(_FORWARD_TO);
  addressControlAg->addAction(_UP_TO);
  addressControlAg->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  return addressControlAg;
}

QActionGroup* AddressBarActions::GetFolderIteratorActions() {
  auto* folderItControlAg = new QActionGroup(this);
  folderItControlAg->addAction(_LAST_FOLDER);
  folderItControlAg->addAction(_NEXT_FOLDER);
  folderItControlAg->setExclusionPolicy(QActionGroup::ExclusionPolicy::ExclusiveOptional);
  return folderItControlAg;
}

AddressBarActions& g_addressBarActions() {
  static AddressBarActions ins;
  return ins;
}
