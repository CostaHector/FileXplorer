#include "AddressBarActions.h"

AddressBarActions::AddressBarActions(QObject* parent) : QObject{parent} {
  _BACK_TO = new QAction{QIcon(":/themes/PATH_BACK_TO"), tr("back to")};
  _FORWARD_TO = new QAction{QIcon(":/themes/PATH_FORWARD_TO"), tr("forward to")};
  _UP_TO = new QAction{QIcon(":/themes/PATH_UP_TO"), tr("up to")};
  ADDRESS_CONTROLS = GetAddressBarActions();

  _LAST_FOLDER = new QAction{QIcon(""), "^"};
  _NEXT_FOLDER = new QAction{QIcon(""), "v"};
  _FOLDER_IT_CONTROLS = GetFolderIteratorActions();

  _LAST_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_BracketLeft));
  _NEXT_FOLDER->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_BracketRight));

  _LAST_FOLDER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Into last folder of parent path in current view.").arg(_LAST_FOLDER->text(), _LAST_FOLDER->shortcut().toString()));
  _NEXT_FOLDER->setToolTip(
      QString("<b>%1 (%2)</b><br/> Into next folder of parent path in current view.").arg(_NEXT_FOLDER->text(), _NEXT_FOLDER->shortcut().toString()));
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
