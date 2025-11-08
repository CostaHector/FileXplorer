#include "FileRenameRulerActions.h"
#include "NotificatorMacro.h"
#include "CastManager.h"
#include "StudiosManager.h"
#include "CastAkasManager.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
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
  _NAME_RULER = new (std::nothrow) QAction(QIcon(":img/NAME_RULER"), "Name Ruler");
  _NAME_RULER->setToolTip(
      "<b>Standardized Files/Folders Name under current view path</b><br/>"
      "Given: [A..mp4, A (1).jpg, A -- 2.json]<br/>"
      "Result: [A.mp4, A - 1.jpg, A - 2.json]");
  NAME_RULES_ACTIONS_LIST += _NAME_RULER;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _EDIT_STUDIOS = new (std::nothrow) QAction(QIcon(":img/STUDIOS_LIST_FILE"), "Edit studios list file", this);
  NAME_RULES_ACTIONS_LIST += _EDIT_STUDIOS;
  _RELOAD_STUDIOS = new (std::nothrow) QAction("Reload studios", this);
  NAME_RULES_ACTIONS_LIST += _RELOAD_STUDIOS;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _EDIT_PERFS = new (std::nothrow) QAction(QIcon(":img/CAST_LIST_FILE"), "Edit performers list file", this);
  NAME_RULES_ACTIONS_LIST += _EDIT_PERFS;
  _RELOAD_PERFS = new (std::nothrow) QAction("Reload performers", this);
  NAME_RULES_ACTIONS_LIST += _RELOAD_PERFS;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _EDIT_PERF_AKA = new (std::nothrow) QAction(QIcon(":img/CAST_AKA_LIST_FILE"), "Edit performers AKA list file", this);
  NAME_RULES_ACTIONS_LIST += _EDIT_PERF_AKA;
  _RELOAD_PERF_AKA = new (std::nothrow) QAction("Reload performers AKA", this);
  NAME_RULES_ACTIONS_LIST += _RELOAD_PERF_AKA;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _RENAME_RULE_STAT = new (std::nothrow) QAction(QIcon(":img/SHOW_CAST_STUDIO_STATISTIC"), "Rename rule statistics", this);
  NAME_RULES_ACTIONS_LIST += _RENAME_RULE_STAT;

  subscribe();
}

void onShowRenameRuleStatistics() {
  QString statictsContent;
  statictsContent += "\nStudios number\t";
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
  connect(_EDIT_STUDIOS, &QAction::triggered,  this, [this]() { onEditLocalFile(PathTool::GetPathByApplicationDirPath(STANDARD_STUDIO_NAME)); });
  connect(_EDIT_PERFS, &QAction::triggered,    this, [this]() { onEditLocalFile(PathTool::GetPathByApplicationDirPath(PERFORMERS_TABLE)); });
  connect(_EDIT_PERF_AKA, &QAction::triggered, this, [this]() { onEditLocalFile(PathTool::GetPathByApplicationDirPath(AKA_PERFORMERS)); });

  connect(_RELOAD_STUDIOS, &QAction::triggered, this, [this]() {
    StudiosManager& psm = StudiosManager::getInst();
    mLastTimeCntDelta = psm.ForceReloadImpl();
    LOG_OK_P("Reload studios", "delta %d items", mLastTimeCntDelta);
  });
  connect(_RELOAD_PERFS, &QAction::triggered, this, [this]() {
    CastManager& pm = CastManager::getInst();
    mLastTimeCntDelta = pm.ForceReloadImpl();
    LOG_OK_P("Reload performers", "delta %d item(s)", mLastTimeCntDelta);
  });
  connect(_RELOAD_PERF_AKA, &QAction::triggered, this, [this]() {
    static auto& dbTM = CastAkasManager::getInst();
    mLastTimeCntDelta = dbTM.ForceReloadImpl();
    LOG_OK_P("Reload performers AKA", "delta %d item(s)", mLastTimeCntDelta);
  });

  connect(_RENAME_RULE_STAT, &QAction::triggered, this, &onShowRenameRuleStatistics);
}

FileRenameRulerActions& g_NameRulerActions() {
  static FileRenameRulerActions ins;
  return ins;
}
