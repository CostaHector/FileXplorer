#include "RightClickMenuActions.h"

RightClickMenuActions::RightClickMenuActions(QObject* parent)  //
    : QObject{parent}                                          //
{
  _SEARCH_IN_NET_EXPLORER = new (std::nothrow) QAction(QIcon(":img/_SEARCH_IN_NET_EXPLORER"), "Search in Net Explorer");
  if (_SEARCH_IN_NET_EXPLORER == nullptr) {
    qCritical("_SEARCH_IN_NET_EXPLORER is nullptr");
    return;
  }
  _SEARCH_IN_NET_EXPLORER->setShortcut(QKeySequence(Qt::Key::Key_F1));
  _SEARCH_IN_NET_EXPLORER->setShortcutVisibleInContextMenu(true);
  _SEARCH_IN_NET_EXPLORER->setToolTip(
      QString("<b>%1 (%2)</b><br/>Search selected item file name in default net explorer.").arg(_SEARCH_IN_NET_EXPLORER->text(), _SEARCH_IN_NET_EXPLORER->shortcut().toString()));

  _CALC_MD5_ACT = new (std::nothrow) QAction(QIcon(":img/MD5_FILE_IDENTIFIER_PATH"), "MD5");
  if (_CALC_MD5_ACT == nullptr) {
    qCritical("_CALC_MD5_ACT is nullptr");
    return;
  }
  _PROPERTIES = new (std::nothrow) QAction(QIcon(":img/PROPERTIES"), "Properties");
  if (_PROPERTIES == nullptr) {
    qCritical("_PROPERTIES is nullptr");
    return;
  }
  _PROPERTIES->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_Return));
  _PROPERTIES->setShortcutVisibleInContextMenu(true);
  _PROPERTIES->setToolTip(QString("<b>%1 (%2)</b><br/>Show the properties for the selected item(s)").arg(_PROPERTIES->text(), _PROPERTIES->shortcut().toString()));

  _FORCE_REFRESH_FILESYSTEMMODEL = new (std::nothrow) QAction{QIcon(":img/REFRESH_THIS_PATH"), "Refresh"};
  if (_FORCE_REFRESH_FILESYSTEMMODEL == nullptr) {
    qCritical("_FORCE_REFRESH_FILESYSTEMMODEL is nullptr");
    return;
  }
  _FORCE_REFRESH_FILESYSTEMMODEL->setShortcut(QKeySequence(Qt::Key::Key_F5));
  _FORCE_REFRESH_FILESYSTEMMODEL->setShortcutVisibleInContextMenu(true);
  _FORCE_REFRESH_FILESYSTEMMODEL->setToolTip(
      QString("<b>%1 (%2)</b><br/>Force refresh file system mode after item change.").arg(_FORCE_REFRESH_FILESYSTEMMODEL->text(), _FORCE_REFRESH_FILESYSTEMMODEL->shortcut().toString()));
}

RightClickMenuActions& g_rightClickActions() {
  static RightClickMenuActions rightClickActions;
  return rightClickActions;
}
