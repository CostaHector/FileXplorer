#include "LogActions.h"
#include "MemoryKey.h"
#include "PublicMacro.h"
#include <QStyle>
#include <QApplication>

LogActions::LogActions(QObject* parent)  //
    : QObject{parent}                    //
{
  _LOG_FILE = new (std::nothrow) QAction{QIcon(":img/RUNNING_LOGS"), "Logs"};
  CHECK_NULLPTR_RETURN_VOID(_LOG_FILE)

  _LOG_FOLDER = new (std::nothrow) QAction{"Logs folder"};
  CHECK_NULLPTR_RETURN_VOID(_LOG_FOLDER)

  _LOG_AGING = new (std::nothrow) QAction{QIcon(":img/AGING_LOGS"), "Aging logs"};
  CHECK_NULLPTR_RETURN_VOID(_LOG_AGING)

  _LOG_LEVEL_DEBUG = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_DEBUG"), "Debug"};
  CHECK_NULLPTR_RETURN_VOID(_LOG_LEVEL_DEBUG)

  _LOG_LEVEL_WARNING = new (std::nothrow) QAction{qApp->style()->standardIcon(QStyle::SP_MessageBoxWarning), "Warning"};
  CHECK_NULLPTR_RETURN_VOID(_LOG_LEVEL_WARNING)

  _FLUSH_INSTANTLY = new (std::nothrow) QAction{"Auto Flush Buffer"};
  CHECK_NULLPTR_RETURN_VOID(_FLUSH_INSTANTLY)

  _LOG_LEVEL_AG = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_LEVEL_AG)

  _LOG_FILE->setCheckable(false);
  _LOG_FILE->setShortcut(QKeySequence(Qt::Key::Key_F12));
  _LOG_FILE->setShortcutVisibleInContextMenu(true);
  _LOG_FILE->setToolTip(QString("<b>%1 (%2)</b><br/>Flush all buffered logs to file and open it in default editor").arg(_LOG_FILE->text(), _LOG_FILE->shortcut().toString()));

  _LOG_FOLDER->setCheckable(false);
  _LOG_FOLDER->setShortcutVisibleInContextMenu(true);
  _LOG_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Open log folder.").arg(_LOG_FOLDER->text(), _LOG_FOLDER->shortcut().toString()));

  _LOG_AGING->setCheckable(false);
  _LOG_AGING->setShortcutVisibleInContextMenu(true);
  _LOG_AGING->setToolTip(QString("<b>%1 (%2)</b><br/>Aging log file if file size >= 10MiB.").arg(_LOG_AGING->text(), _LOG_AGING->shortcut().toString()));

  _LOG_LEVEL_DEBUG->setCheckable(true);
  _LOG_LEVEL_DEBUG->setShortcutVisibleInContextMenu(true);
  _LOG_LEVEL_DEBUG->setToolTip(QString("<b>%1 (%2)</b><br/>Set log level to debug.").arg(_LOG_LEVEL_DEBUG->text(), _LOG_LEVEL_DEBUG->shortcut().toString()));

  _LOG_LEVEL_WARNING->setCheckable(true);
  _LOG_LEVEL_WARNING->setShortcutVisibleInContextMenu(true);
  _LOG_LEVEL_WARNING->setToolTip(QString("<b>%1 (%2)</b><br/>Set log level to warning.").arg(_LOG_LEVEL_WARNING->text(), _LOG_LEVEL_WARNING->shortcut().toString()));

  if (PreferenceSettings().value(MemoryKey::LOG_LEVEL_PRINT_INSTANTLY.name, MemoryKey::LOG_LEVEL_PRINT_INSTANTLY.v).toBool()) {
    _LOG_LEVEL_DEBUG->setChecked(true);
  } else {
    _LOG_LEVEL_WARNING->setChecked(true);
  }

  _LOG_LEVEL_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
  _LOG_LEVEL_AG->addAction(_LOG_LEVEL_DEBUG);
  _LOG_LEVEL_AG->addAction(_LOG_LEVEL_WARNING);

  _FLUSH_INSTANTLY->setCheckable(true);
  _FLUSH_INSTANTLY->setChecked(false);
  _FLUSH_INSTANTLY->setShortcutVisibleInContextMenu(true);
  _FLUSH_INSTANTLY->setToolTip(QString("<b>%1 (%2)</b><br/>Auto flush log stashed in buffers into log file instantly.").arg(_FLUSH_INSTANTLY->text(), _FLUSH_INSTANTLY->shortcut().toString()));

  _DROPDOWN_LIST << _LOG_FILE << _LOG_FOLDER << nullptr << _LOG_AGING << nullptr << _LOG_LEVEL_DEBUG << _LOG_LEVEL_WARNING << nullptr << _FLUSH_INSTANTLY;
}

LogActions& g_LogActions() {
  static LogActions ins;
  return ins;
}
