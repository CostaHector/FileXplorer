#include "ArrangeActions.h"
#include "NotificatorMacro.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "CastAkasManager.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFile>

void onEditStudios() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(STANDARD_STUDIO_NAME);
  if (!QFile::exists(fileAbsPath)) {
    LOG_WARN_P("Cannot edit", "File[%s] not found", qPrintable(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  LOG_INFO_NP("Modify work after reload", "Reload now?");
}

void onEditPerformers() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(PERFORMERS_TABLE);
  if (!QFile::exists(fileAbsPath)) {
    LOG_WARN_NP("[File Inexists] Cannot edit", fileAbsPath);
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  LOG_INFO_NP("Remember to reload", "don't forget it");
}

void onEditAkaPerformer() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(AKA_PERFORMERS);
  if (!QFile::exists(fileAbsPath)) {
    LOG_WARN_P("Cannot edit", "File[%s] not found", qPrintable(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  LOG_INFO_NP("Work after reopen", "changes not work now");
}

ArrangeActions::ArrangeActions(QObject* parent) : QObject{parent} {
  _EDIT_STUDIOS = new (std::nothrow) QAction(QIcon(":img/STUDIOS_LIST_FILE"), "Edit studios list file", this);
  _RELOAD_STUDIOS = new (std::nothrow) QAction("Reload studios", this);

  _EDIT_PERFS = new (std::nothrow) QAction(QIcon(":img/CAST_LIST_FILE"), "Edit performers list file", this);
  _RELOAD_PERFS = new (std::nothrow) QAction("Reload performers", this);

  _EDIT_PERF_AKA = new (std::nothrow) QAction(QIcon(":img/CAST_AKA_LIST_FILE"), "Edit performers AKA list file", this);
  _RELOAD_PERF_AKA = new (std::nothrow) QAction("Reload performers AKA", this);

  _RENAME_RULE_STAT = new (std::nothrow) QAction(QIcon(":img/SHOW_CAST_STUDIO_STATISTIC"), "Rename rule statistics", this);
}

void onShowRenameRuleStatistics() {
  QString statictsContent;
  statictsContent += "Studios number\t";
  statictsContent += QString::number(StudiosManager::getInst().count());
  statictsContent += '\n';
  statictsContent += "Cast number\t";
  statictsContent += QString::number(CastManager::getInst().count());
  statictsContent += '\n';
  statictsContent += "AKA number\t";
  statictsContent += QString::number(CastAkasManager::getInst().count());
  LOG_INFO_NP("Rename rule statistics", statictsContent);
}

void ArrangeActions::subscribe() {
  connect(_EDIT_STUDIOS, &QAction::triggered, this, &onEditStudios);
  connect(_RELOAD_STUDIOS, &QAction::triggered, this, []() {
    StudiosManager& psm = StudiosManager::getInst();
    int itemsCntChanged = psm.ForceReloadImpl();
    LOG_OK_P("Reload studios", "delta %d items", itemsCntChanged);
  });

  connect(_EDIT_PERFS, &QAction::triggered, this, &onEditPerformers);
  connect(_RELOAD_PERFS, &QAction::triggered, this, []() {
    CastManager& pm = CastManager::getInst();
    int itemsCntChanged = pm.ForceReloadImpl();
    LOG_OK_P("Reload performers", "delta %d item(s)", itemsCntChanged);
  });

  connect(_EDIT_PERF_AKA, &QAction::triggered, this, &onEditAkaPerformer);
  connect(_RELOAD_PERF_AKA, &QAction::triggered, this, []() {
    static auto& dbTM = CastAkasManager::getInst();
    int itemsCntChanged = dbTM.ForceReloadImpl();
    LOG_OK_P("Reload performers AKA", "delta %d item(s)", itemsCntChanged);
  });

  connect(_RENAME_RULE_STAT, &QAction::triggered, this, &onShowRenameRuleStatistics);
}

ArrangeActions& g_ArrangeActions() {
  static ArrangeActions ins;
  return ins;
}
