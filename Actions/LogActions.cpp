#include "LogActions.h"
#include "MemoryKey.h"
#include "Logger.h"
#include "PublicMacro.h"

LogActions::LogActions(QObject* parent)  //
  : QObject{parent}                    //
{
  _LOG_FILE = new (std::nothrow) QAction{QIcon{":img/LOG_FILES"}, "Open logs file", this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_FILE)
  _LOG_FILE->setCheckable(false);
  _LOG_FILE->setShortcutVisibleInContextMenu(true);
  _LOG_FILE->setToolTip(QString("<b>%1 (%2)</b><br/>Call FFlush first then open log file in system default editor").arg(_LOG_FILE->text(), _LOG_FILE->shortcut().toString()));

  _LOG_FOLDER = new (std::nothrow) QAction{QIcon{":img/LOG_FOLDERS"}, "Open logs folder", this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_FOLDER)
  _LOG_FOLDER->setCheckable(false);
  _LOG_FOLDER->setShortcutVisibleInContextMenu(true);
  _LOG_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Open folder logs where located in.").arg(_LOG_FOLDER->text(), _LOG_FOLDER->shortcut().toString()));

  _LOG_AGING = new (std::nothrow) QAction{QIcon(":img/AGING_LOGS"), "Aging logs", this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_AGING)
  _LOG_AGING->setCheckable(false);
  _LOG_AGING->setShortcutVisibleInContextMenu(true);
  _LOG_AGING->setToolTip(QString("<b>%1 (%2)</b><br/>Aging log file if file size >= 10MiB.").arg(_LOG_AGING->text(), _LOG_AGING->shortcut().toString()));

  _LOG_PRINT_LEVEL_DEBUG = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_DEBUG"), "Debug", this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_DEBUG)
  _LOG_PRINT_LEVEL_DEBUG->setCheckable(true);
  _LOG_PRINT_LEVEL_DEBUG->setChecked(true);
  _LOG_PRINT_LEVEL_DEBUG->setShortcutVisibleInContextMenu(true);
  _LOG_PRINT_LEVEL_DEBUG->setToolTip(QString("<b>%1 (%2)</b><br/>log level below debug will get ignored.").arg(_LOG_PRINT_LEVEL_DEBUG->text(), _LOG_PRINT_LEVEL_DEBUG->shortcut().toString()));

  _LOG_PRINT_LEVEL_WARNING = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_WARNING"), "Warning", this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_WARNING)
  _LOG_PRINT_LEVEL_WARNING->setCheckable(true);
  _LOG_PRINT_LEVEL_WARNING->setShortcutVisibleInContextMenu(true);
  _LOG_PRINT_LEVEL_WARNING->setToolTip(QString("<b>%1 (%2)</b><br/>log level below warning will get ignored.").arg(_LOG_PRINT_LEVEL_WARNING->text(), _LOG_PRINT_LEVEL_WARNING->shortcut().toString()));

  _LOG_PRINT_LEVEL_AG = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_AG)
  _LOG_PRINT_LEVEL_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
  _LOG_PRINT_LEVEL_AG->addAction(_LOG_PRINT_LEVEL_DEBUG);
  _LOG_PRINT_LEVEL_AG->addAction(_LOG_PRINT_LEVEL_WARNING);

  _AUTO_FLUSH_IGNORE_LEVEL = new (std::nothrow) QAction{"Auto FFlush ignore level", this};
  CHECK_NULLPTR_RETURN_VOID(_AUTO_FLUSH_IGNORE_LEVEL)
  _AUTO_FLUSH_IGNORE_LEVEL->setToolTip(QString("<b>%1 (%2)</b><br/>Auto flush log to local files no matter what log level it is.").arg(_AUTO_FLUSH_IGNORE_LEVEL->text(), _AUTO_FLUSH_IGNORE_LEVEL->shortcut().toString()));
  _AUTO_FLUSH_IGNORE_LEVEL->setCheckable(true);
  _AUTO_FLUSH_IGNORE_LEVEL->setChecked(Configuration().value(MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.name, MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.v).toBool());
  _AUTO_FLUSH_IGNORE_LEVEL->setShortcutVisibleInContextMenu(true);

  _DROPDOWN_LIST << _LOG_FILE << _LOG_FOLDER << nullptr << _LOG_AGING << nullptr << _LOG_PRINT_LEVEL_DEBUG << _LOG_PRINT_LEVEL_WARNING << nullptr << _AUTO_FLUSH_IGNORE_LEVEL;
}

QToolButton* LogActions::GetLogPreviewerToolButton(QWidget* parent) {
  QToolButton* logPreviewerTb = new (std::nothrow) QToolButton{parent};
  CHECK_NULLPTR_RETURN_NULLPTR(logPreviewerTb)
  logPreviewerTb->setIcon(QIcon(":img/LOG_FILES_PREVIEW"));
  logPreviewerTb->setCheckable(true);
  return logPreviewerTb;
}

LogActions& g_LogActions() {
  static LogActions ins;
  return ins;
}
