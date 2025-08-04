#include "ArrangeActions.h"
#include "Component/Notificator.h"
#include "Tools/CastManager.h"
#include "Tools/StudiosManager.h"
#include "Tools/PerformersAkaManager.h"
#include "public/PathTool.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include <QDesktopServices>
#include <QUrl>
#include <QFile>

void onEditStudios() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(STANDARD_STUDIO_NAME);
  if (!QFile::exists(fileAbsPath)) {
    qWarning("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Modify work after reload", "Reload now?");
}

void onEditPerformers() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(PERFORMERS_TABLE);
  if (!QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Remember to reload", "don't forget it");
}

void onEditAkaPerformer() {
  using namespace PathTool::FILE_REL_PATH;
  static const QString fileAbsPath = PathTool::GetPathByApplicationDirPath(AKA_PERFORMERS);
  if (!QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Work after reopen", "changes not work now");
}

ArrangeActions::ArrangeActions(QObject* parent) : QObject{parent} {
  _EDIT_STUDIOS = new (std::nothrow) QAction(QIcon(":img/PRODUCTION_STUDIOS_LIST_FILE"), "Edit studios list file", this);
  _RELOAD_STUDIOS = new (std::nothrow) QAction("Reload studios", this);

  _EDIT_PERFS = new (std::nothrow) QAction(QIcon(":img/CAST_SET"), "Edit performers list file", this);
  _RELOAD_PERFS = new (std::nothrow) QAction("Reload performers", this);

  _EDIT_PERF_AKA = new (std::nothrow) QAction(QIcon(":img/PERFORMER_AKA_LIST_FILE"), "Edit performers AKA list file", this);
  _RELOAD_PERF_AKA = new (std::nothrow) QAction("Reload performers AKA", this);

  _RENAME_RULE_STAT = new (std::nothrow) QAction(QIcon(":img/SHOW_PERFORMER_STUDIO_STATISTIC"), "Rename rule statistics", this);
}

void onShowRenameRuleStatistics() {
  QString statictsContent;
  statictsContent += "Studios number\t";
  statictsContent += QString::number(StudiosManager::getIns().count());
  statictsContent += '\n';
  statictsContent += "Cast number\t";
  statictsContent += QString::number(CastManager::getIns().count());
  statictsContent += '\n';
  statictsContent += "AKA number\t";
  statictsContent += QString::number(PerformersAkaManager::getIns().count());
  Notificator::information("Rename rule statistics", statictsContent);
}

void ArrangeActions::subscribe() {
  connect(_EDIT_STUDIOS, &QAction::triggered, this, &onEditStudios);
  connect(_RELOAD_STUDIOS, &QAction::triggered, this, []() {
    static auto& psm = StudiosManager::getIns();
    int itemsCntChanged = psm.ForceReloadStudio();
    Notificator::goodNews("Reload studios", QString("delta %1 items").arg(itemsCntChanged));
  });

  connect(_EDIT_PERFS, &QAction::triggered, this, &onEditPerformers);
  connect(_RELOAD_PERFS, &QAction::triggered, this, []() {
    static auto& pm = CastManager::getIns();
    int itemsCntChanged = pm.ForceReloadCast();
    Notificator::goodNews("Reload performers", QString("delta %1 item(s)").arg(itemsCntChanged));
  });

  connect(_EDIT_PERF_AKA, &QAction::triggered, this, &onEditAkaPerformer);
  connect(_RELOAD_PERF_AKA, &QAction::triggered, this, []() {
    static auto& dbTM = PerformersAkaManager::getIns();
    int itemsCntChanged = dbTM.ForceReloadAkaName();
    Notificator::goodNews("Reload performers AKA", QString("delta %1 item(s)").arg(itemsCntChanged));
  });

  connect(_RENAME_RULE_STAT, &QAction::triggered, this, &onShowRenameRuleStatistics);
}

ArrangeActions& g_ArrangeActions() {
  static ArrangeActions ins;
  return ins;
}
