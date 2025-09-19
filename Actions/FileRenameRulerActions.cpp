#include "FileRenameRulerActions.h"
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
#include <tuple>
#include "PublicTool.h"

void FileRenameRulerActions::onEditLocalFile(const QString& rel2File) {
  const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(rel2File);
  mLastTimeEditFileInfo.first = FileTool::OpenLocalFileUsingDesktopService(fileAbsPath);
  mLastTimeEditFileInfo.second = fileAbsPath;
  if (!mLastTimeEditFileInfo.first) {
    LOG_WARN_NP("Cannot Edit", fileAbsPath);
    return;
  }
  LOG_INFO_NP("Remember to reload", "don't forget it");
}

FileRenameRulerActions::FileRenameRulerActions(QObject* parent) : QObject{parent} {
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

void FileRenameRulerActions::subscribe() {
  using namespace PathTool::FILE_REL_PATH;
  connect(_EDIT_STUDIOS, &QAction::triggered, this, []() {
    PathTool::GetPathByApplicationDirPath(STANDARD_STUDIO_NAME);
  });
  connect(_EDIT_PERFS, &QAction::triggered, this, []() {
    PathTool::GetPathByApplicationDirPath(PERFORMERS_TABLE);
  });
  connect(_EDIT_PERF_AKA, &QAction::triggered, this, []() {
    PathTool::GetPathByApplicationDirPath(AKA_PERFORMERS);
  });

  connect(_RELOAD_STUDIOS, &QAction::triggered, this, []() {
    StudiosManager& psm = StudiosManager::getInst();
    int itemsCntChanged = psm.ForceReloadImpl();
    LOG_OK_P("Reload studios", "delta %d items", itemsCntChanged);
  });
  connect(_RELOAD_PERFS, &QAction::triggered, this, []() {
    CastManager& pm = CastManager::getInst();
    int itemsCntChanged = pm.ForceReloadImpl();
    LOG_OK_P("Reload performers", "delta %d item(s)", itemsCntChanged);
  });
  connect(_RELOAD_PERF_AKA, &QAction::triggered, this, []() {
    static auto& dbTM = CastAkasManager::getInst();
    int itemsCntChanged = dbTM.ForceReloadImpl();
    LOG_OK_P("Reload performers AKA", "delta %d item(s)", itemsCntChanged);
  });

  connect(_RENAME_RULE_STAT, &QAction::triggered, this, &onShowRenameRuleStatistics);
}

FileRenameRulerActions& g_ArrangeActions() {
  static FileRenameRulerActions ins;
  return ins;
}
