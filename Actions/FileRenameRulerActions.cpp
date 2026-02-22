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
#include "FileTool.h"

FileRenameRulerActions::FileRenameRulerActions(QObject* parent) : QObject{parent} {
  _NAME_RULER = new (std::nothrow) QAction(QIcon(":img/NAME_RULER"), tr("Name Ruler"));
  _NAME_RULER->setToolTip(
      "<b>Standardized Files/Folders Name under current view path</b><br/>"
      "Given: [A..mp4, A (1).jpg, A -- 2.json]<br/>"
      "Result: [A.mp4, A - 1.jpg, A - 2.json]");
  NAME_RULES_ACTIONS_LIST += _NAME_RULER;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _EDIT_STUDIOS = new (std::nothrow) QAction(QIcon(":img/STUDIOS_LIST_FILE"), tr("Edit studios list file"), this);
  NAME_RULES_ACTIONS_LIST += _EDIT_STUDIOS;
  _RELOAD_STUDIOS = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload studios"), this);
  NAME_RULES_ACTIONS_LIST += _RELOAD_STUDIOS;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _EDIT_ACTORS = new (std::nothrow) QAction(QIcon(":img/CAST_LIST_FILE"), tr("Edit actors list file"), this);
  NAME_RULES_ACTIONS_LIST += _EDIT_ACTORS;
  _RELOAD_ACTORS = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload actors"), this);
  NAME_RULES_ACTIONS_LIST += _RELOAD_ACTORS;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _EDIT_ACTORS_ALIAS = new (std::nothrow) QAction(QIcon(":img/CAST_AKA_LIST_FILE"), tr("Edit actors alias list file"), this);
  NAME_RULES_ACTIONS_LIST += _EDIT_ACTORS_ALIAS;
  _RELOAD_ACTORS_ALIAS = new (std::nothrow) QAction(QIcon(":img/RELOAD_FROM_DISK"), tr("Reload actor alias"), this);
  NAME_RULES_ACTIONS_LIST += _RELOAD_ACTORS_ALIAS;
  NAME_RULES_ACTIONS_LIST += nullptr;

  _RENAME_RULE_STAT = new (std::nothrow) QAction(QIcon(":img/SHOW_CAST_STUDIO_STATISTIC"), tr("Rename rule statistics"), this);
  NAME_RULES_ACTIONS_LIST += _RENAME_RULE_STAT;

  subscribe();
}

bool FileRenameRulerActions::onEditLocalFile(const QString& rel2File) {
  const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(rel2File);
  const bool openResult = FileTool::OpenLocalFileUsingDesktopService(fileAbsPath);
  if (!openResult) {
    LOG_WARN_NP("Cannot Edit", fileAbsPath);
    return false;
  }
  LOG_INFO_NP("Remember to reload", "don't forget it");
  return true;
}

bool FileRenameRulerActions::onEditStudiosListFile() {
  using namespace PathTool::FILE_REL_PATH;
  return onEditLocalFile(GetVendorsTableFilePath());
}
bool FileRenameRulerActions::onEditActorsListFile() {
  using namespace PathTool::FILE_REL_PATH;
  return onEditLocalFile(GetActorsListFilePath());
}
bool FileRenameRulerActions::onEditActorsAliasListFile() {
  using namespace PathTool::FILE_REL_PATH;
  return onEditLocalFile(GetActorsAliasListFilePath());
}
int FileRenameRulerActions::onReloadStudiosListFile() {
  StudiosManager& psm = StudiosManager::getInst();
  int cntDelta = psm.ForceReloadImpl();
  LOG_OK_P("Reload studios", "delta %d items", cntDelta);
  return cntDelta;
}

int FileRenameRulerActions::onReloadActorsListFile() {
  CastManager& pm = CastManager::getInst();
  int cntDelta = pm.ForceReloadImpl();
  LOG_OK_P("Reload actors", "delta %d item(s)", cntDelta);
  return cntDelta;
}
int FileRenameRulerActions::onReloadActorsAliasListFile() {
  static auto& dbTM = CastAkasManager::getInst();
  int cntDelta = dbTM.ForceReloadImpl();
  LOG_OK_P("Reload actors alias", "delta %d item(s)", cntDelta);
  return cntDelta;
}

void FileRenameRulerActions::onShowRenameRuleStatistics() {
  QString statisticsContent;
  statisticsContent += "Studios number\t";
  statisticsContent += QString::number(StudiosManager::getInst().count());
  statisticsContent += '\n';
  statisticsContent += "Actors number\t";
  statisticsContent += QString::number(CastManager::getInst().count());
  statisticsContent += '\n';
  statisticsContent += "Actors Alias number\t";
  statisticsContent += QString::number(CastAkasManager::getInst().count());
  LOG_INFO_NP("Rename rule statistics", statisticsContent);
}

void FileRenameRulerActions::subscribe() {
  connect(_EDIT_STUDIOS, &QAction::triggered, this, &FileRenameRulerActions::onEditStudiosListFile);
  connect(_EDIT_ACTORS, &QAction::triggered, this, &FileRenameRulerActions::onEditActorsListFile);
  connect(_EDIT_ACTORS_ALIAS, &QAction::triggered, this, &FileRenameRulerActions::onEditActorsAliasListFile);

  connect(_RELOAD_STUDIOS, &QAction::triggered, this, &FileRenameRulerActions::onReloadStudiosListFile);
  connect(_RELOAD_ACTORS, &QAction::triggered, this, &FileRenameRulerActions::onReloadActorsListFile);
  connect(_RELOAD_ACTORS_ALIAS, &QAction::triggered, this, &FileRenameRulerActions::onReloadActorsAliasListFile);

  connect(_RENAME_RULE_STAT, &QAction::triggered, this, &onShowRenameRuleStatistics);
}

FileRenameRulerActions& g_NameRulerActions() {
  static FileRenameRulerActions ins;
  return ins;
}
