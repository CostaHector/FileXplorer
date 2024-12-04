#include "LogActions.h"
#include "PublicVariable.h"

LogActions::LogActions(QObject* parent) : QObject{parent} {
  _LOG_FILE = new (std::nothrow) QAction{QIcon(":img/RUNNING_LOGS"), "Logs"};
  if (_LOG_FILE == nullptr) {
    qCritical("_LOG_FILE is nullptr");
    return;
  }
  _LOG_FOLDER = new (std::nothrow) QAction{"Logs folder"};
  if (_LOG_FOLDER == nullptr) {
    qCritical("_LOG_FOLDER is nullptr");
    return;
  }
  _LOG_AGING = new (std::nothrow) QAction{QIcon(":img/AGING_LOGS"), "Aging logs"};
  if (_LOG_AGING == nullptr) {
    qCritical("_LOG_AGING is nullptr");
    return;
  }
  _LOG_LEVEL_DEBUG = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_DEBUG"), "Debug"};
  if (_LOG_LEVEL_DEBUG == nullptr) {
    qCritical("_LOG_LEVEL_DEBUG is nullptr");
    return;
  }
  _LOG_LEVEL_ERROR = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_WARNING"), "Error"};
  if (_LOG_LEVEL_ERROR == nullptr) {
    qCritical("_LOG_LEVEL_ERROR is nullptr");
    return;
  }

  _FLUSH_INSTANTLY = new (std::nothrow) QAction{"Flush instantly"};
  if (_FLUSH_INSTANTLY == nullptr) {
    qCritical("_FLUSH_INSTANTLY is nullptr");
    return;
  }

  _LOG_LEVEL_AG = new (std::nothrow) QActionGroup{this};
  if (_LOG_LEVEL_AG == nullptr) {
    qCritical("_LOG_LEVEL_AG is nullptr");
    return;
  }

  _LOG_FILE->setCheckable(false);
  _LOG_FILE->setShortcutVisibleInContextMenu(true);
  _LOG_FILE->setToolTip(QString("<b>%1 (%2)</b><br/>Open log file.").arg(_LOG_FILE->text(), _LOG_FILE->shortcut().toString()));

  _LOG_FOLDER->setCheckable(false);
  _LOG_FOLDER->setShortcutVisibleInContextMenu(true);
  _LOG_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Open log folder.").arg(_LOG_FOLDER->text(), _LOG_FOLDER->shortcut().toString()));

  _LOG_AGING->setCheckable(false);
  _LOG_AGING->setShortcutVisibleInContextMenu(true);
  _LOG_AGING->setToolTip(QString("<b>%1 (%2)</b><br/>Aging log file if file size >= 10MiB.").arg(_LOG_AGING->text(), _LOG_AGING->shortcut().toString()));

  _LOG_LEVEL_DEBUG->setCheckable(true);
  _LOG_LEVEL_DEBUG->setShortcutVisibleInContextMenu(true);
  _LOG_LEVEL_DEBUG->setToolTip(QString("<b>%1 (%2)</b><br/>Set log level to debug.").arg(_LOG_LEVEL_DEBUG->text(), _LOG_LEVEL_DEBUG->shortcut().toString()));

  _LOG_LEVEL_ERROR->setCheckable(true);
  _LOG_LEVEL_ERROR->setShortcutVisibleInContextMenu(true);
  _LOG_LEVEL_ERROR->setToolTip(QString("<b>%1 (%2)</b><br/>Set log level to warning.").arg(_LOG_LEVEL_ERROR->text(), _LOG_LEVEL_ERROR->shortcut().toString()));

  if (PreferenceSettings().value(MemoryKey::LOG_DEVEL_DEBUG.name, MemoryKey::LOG_DEVEL_DEBUG.v).toBool()) {
    _LOG_LEVEL_DEBUG->setChecked(true);
  } else {
    _LOG_LEVEL_ERROR->setChecked(true);
  }

  _LOG_LEVEL_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
  _LOG_LEVEL_AG->addAction(_LOG_LEVEL_DEBUG);
  _LOG_LEVEL_AG->addAction(_LOG_LEVEL_ERROR);

  _FLUSH_INSTANTLY->setCheckable(true);
  _FLUSH_INSTANTLY->setChecked(false);
  _FLUSH_INSTANTLY->setShortcutVisibleInContextMenu(true);
  _FLUSH_INSTANTLY->setToolTip(QString("<b>%1 (%2)</b><br/>Flush log stashed in buffers into log file instantly.").arg(_FLUSH_INSTANTLY->text(), _FLUSH_INSTANTLY->shortcut().toString()));

  _DROPDOWN_LIST << _LOG_FILE << _LOG_FOLDER << nullptr << _LOG_AGING << nullptr << _LOG_LEVEL_DEBUG << _LOG_LEVEL_ERROR << nullptr << _FLUSH_INSTANTLY;
}

LogActions& g_LogActions() {
  static LogActions ins;
  return ins;
}
