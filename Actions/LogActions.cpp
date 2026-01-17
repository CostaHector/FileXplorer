#include "LogActions.h"
#include "MemoryKey.h"
#include "MenuToolButton.h"
#include "PublicMacro.h"
#include "Logger.h"
#include <QHash>

LogActions::LogActions(QObject* parent) : QObject{parent} {
  _LOG_FILE = new (std::nothrow) QAction{QIcon{":img/LOG_FILES"}, tr("Open logs file"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_FILE)
  _LOG_FILE->setCheckable(false);
  _LOG_FILE->setShortcutVisibleInContextMenu(true);
  _LOG_FILE->setToolTip(QString("<b>%1 (%2)</b><br/>Call FFlush first then open log file in system default editor").arg(_LOG_FILE->text(), _LOG_FILE->shortcut().toString()));
  _DROPDOWN_LIST += _LOG_FILE;

  _LOG_FOLDER = new (std::nothrow) QAction{QIcon{":img/LOG_FOLDERS"}, tr("Open logs folder"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_FOLDER)
  _LOG_FOLDER->setCheckable(false);
  _LOG_FOLDER->setShortcutVisibleInContextMenu(true);
  _LOG_FOLDER->setToolTip(QString("<b>%1 (%2)</b><br/>Open folder logs where located in.").arg(_LOG_FOLDER->text(), _LOG_FOLDER->shortcut().toString()));
  _DROPDOWN_LIST += _LOG_FOLDER;

  _LOG_ROTATION = new (std::nothrow) QAction{QIcon(":img/LOG_ROTATION"), tr("Rotate log file"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_ROTATION)
  _LOG_ROTATION->setCheckable(false);
  _LOG_ROTATION->setShortcutVisibleInContextMenu(true);
  _LOG_ROTATION->setToolTip(QString("<b>%1 (%2)</b><br/>Rotate log files if they exceed 10 MB in size").arg(_LOG_ROTATION->text(), _LOG_ROTATION->shortcut().toString()));
  _DROPDOWN_LIST += _LOG_ROTATION;

  _LOG_PRINT_LEVEL_DEBUG = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_DEBUG"), tr("Debug"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_DEBUG)
  _LOG_PRINT_LEVEL_DEBUG->setCheckable(true);
  _LOG_PRINT_LEVEL_DEBUG->setChecked(true);
  _LOG_PRINT_LEVEL_DEBUG->setToolTip("Log messages below Debug level will be ignored.");

  _LOG_PRINT_LEVEL_INFO = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_INFO"), tr("Info"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_INFO)
  _LOG_PRINT_LEVEL_INFO->setCheckable(true);
  _LOG_PRINT_LEVEL_INFO->setToolTip("Log messages below Info level will be ignored.");

  _LOG_PRINT_LEVEL_WARNING = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_WARNING"), tr("Warning"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_WARNING)
  _LOG_PRINT_LEVEL_WARNING->setCheckable(true);
  _LOG_PRINT_LEVEL_WARNING->setToolTip("Log messages below Warning level will be ignored.");

  _LOG_PRINT_LEVEL_ERROR = new (std::nothrow) QAction{QIcon(":img/LOG_LEVEL_ERROR"), tr("Error"), this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_ERROR)
  _LOG_PRINT_LEVEL_ERROR->setCheckable(true);
  _LOG_PRINT_LEVEL_ERROR->setToolTip("Log messages below Error level will be ignored.");

  _LOG_PRINT_LEVEL_AG = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(_LOG_PRINT_LEVEL_AG)
  _LOG_PRINT_LEVEL_AG->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);
  _LOG_PRINT_LEVEL_AG->addAction(_LOG_PRINT_LEVEL_DEBUG);
  _LOG_PRINT_LEVEL_AG->addAction(_LOG_PRINT_LEVEL_INFO);
  _LOG_PRINT_LEVEL_AG->addAction(_LOG_PRINT_LEVEL_WARNING);
  _LOG_PRINT_LEVEL_AG->addAction(_LOG_PRINT_LEVEL_ERROR);
  _DROPDOWN_LIST += nullptr;
  _DROPDOWN_LIST += _LOG_PRINT_LEVEL_AG->actions();
  static const QHash<QAction*, LOG_LVL_E> LOG_PRINT_LVL_ACTION_2_ENUM//
      {
       {_LOG_PRINT_LEVEL_DEBUG, LOG_LVL_E::D},
       {_LOG_PRINT_LEVEL_INFO, LOG_LVL_E::I},
       {_LOG_PRINT_LEVEL_WARNING, LOG_LVL_E::W},
       {_LOG_PRINT_LEVEL_ERROR, LOG_LVL_E::E},
       };

  _AUTO_FLUSH_IGNORE_LEVEL = new (std::nothrow) QAction{tr("Auto FFlush ignore level"), this};
  CHECK_NULLPTR_RETURN_VOID(_AUTO_FLUSH_IGNORE_LEVEL)
  _AUTO_FLUSH_IGNORE_LEVEL->setToolTip(QString("<b>%1 (%2)</b><br/>Auto flush log to local files no matter what log level it is.").arg(_AUTO_FLUSH_IGNORE_LEVEL->text(), _AUTO_FLUSH_IGNORE_LEVEL->shortcut().toString()));
  _AUTO_FLUSH_IGNORE_LEVEL->setCheckable(true);
  _AUTO_FLUSH_IGNORE_LEVEL->setChecked(Configuration().value(MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.name, MemoryKey::ALL_LOG_LEVEL_AUTO_FFLUSH.v).toBool());
  _AUTO_FLUSH_IGNORE_LEVEL->setShortcutVisibleInContextMenu(true);
  _DROPDOWN_LIST += nullptr;
  _DROPDOWN_LIST += _AUTO_FLUSH_IGNORE_LEVEL;

  connect(_LOG_FILE, &QAction::triggered, &Logger::OpenLogFile);
  connect(_LOG_FOLDER, &QAction::triggered, &Logger::OpenLogFolder);
  connect(_LOG_ROTATION, &QAction::triggered, []() { Logger::AgingLogFiles(Logger::GetLogFileAbsPath()); });
  connect(_LOG_PRINT_LEVEL_AG, &QActionGroup::triggered, this, [](QAction* pAct) {
    auto it = LOG_PRINT_LVL_ACTION_2_ENUM.find(pAct);
    if (it != LOG_PRINT_LVL_ACTION_2_ENUM.cend()) {
      Logger::SetPrintLevel(it.value());
      return;
    }
    Logger::SetPrintLevel(LOG_LVL_E::D);
  });
  connect(_AUTO_FLUSH_IGNORE_LEVEL, &QAction::toggled, &Logger::SetAutoFlushAllLevel);
}

QToolButton *LogActions::GetLogToolButton(QWidget *parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  MenuToolButton* logToolButton = new (std::nothrow) MenuToolButton(
      _DROPDOWN_LIST,
      QToolButton::InstantPopup,
      Qt::ToolButtonStyle::ToolButtonTextUnderIcon,
      IMAGE_SIZE::TABS_ICON_IN_MENU_16,
      parent);
  logToolButton->SetCaption(QIcon{":img/LOG_SETTINGS"}, tr("Log settings"), "Open logs/Change Log print level/auto fflush level");
  return logToolButton;
}

LogActions& g_LogActions() {
  static LogActions ins;
  return ins;
}
