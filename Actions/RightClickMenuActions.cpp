#include "RightClickMenuActions.h"

RightClickMenuActions::RightClickMenuActions(QObject* parent)
    : QObject{parent},
      _CALC_MD5_ACT(new QAction(QIcon(":/themes/MD5_FILE_IDENTIFIER_PATH"), tr("MD5 (File Indentifier)"))),
      _PROPERTIES(new QAction(QIcon(":/themes/PROPERTIES"), tr("Properties")))

{
  _PROPERTIES->setShortcut(QKeySequence(Qt::KeyboardModifier::AltModifier | Qt::Key::Key_Return));
  _PROPERTIES->setShortcutVisibleInContextMenu(true);
  _PROPERTIES->setToolTip(
      QString("<b>%1 (%2)</b><br/>Show the properties fot the selected item(s)").arg(_PROPERTIES->text(), _PROPERTIES->shortcut().toString()));
}

RightClickMenuActions& g_rightClickActions()
{
  static RightClickMenuActions rightClickActions;
  return rightClickActions;
}
