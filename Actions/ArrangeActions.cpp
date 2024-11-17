#include "ArrangeActions.h"

#include "PublicVariable.h"
#include "Component/NotificatorFrame.h"
#include "Tools/PerformersManager.h"
#include "Tools/ProductionStudioManager.h"
#include "Tools/PerformersAkaManager.h"
#include <QDesktopServices>

void onEditStudios() {
#ifdef _WIN32
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::WIN32_STANDARD_STUDIO_NAME.name).toString();
#else
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::LINUX_STANDARD_STUDIO_NAME.name).toString();
#endif
  if (!QFile::exists(fileAbsPath)) {
    qWarning("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Modify work after reload", "Reload now?");
}

void onEditPerformers() {
#ifdef _WIN32
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::WIN32_PERFORMERS_TABLE.name).toString();
#else
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::LINUX_PERFORMERS_TABLE.name).toString();
#endif
  if (not QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Remember to reload", "don't forget it");
}

void onEditAkaPerformer() {
#ifdef _WIN32
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::WIN32_AKA_PERFORMERS.name).toString();
#else
  QString fileAbsPath = PreferenceSettings().value(MemoryKey::LINUX_AKA_PERFORMERS.name).toString();
#endif
  if (not QFile::exists(fileAbsPath)) {
    qDebug("Cannot edit. File[%s] not found", qPrintable(fileAbsPath));
    Notificator::warning("Cannot edit", QString("File[%1] not found").arg(fileAbsPath));
    return;
  }
  QDesktopServices::openUrl(QUrl::fromLocalFile(fileAbsPath));
  Notificator::information("Work after reopen", "changes not work now");
}

ArrangeActions::ArrangeActions(QObject* parent) : QObject{parent} {
  _EDIT_STUDIOS = new QAction(QIcon(":img/PRODUCTION_STUDIOS"), "Edit Studios map", this);
  _RELOAD_STUDIOS = new QAction(QIcon(":img/RELOAD_FROM_DISK"), "Reload Studios", this);

  _EDIT_PERFS = new QAction(QIcon(":img/PERFORMERS"), "Edit Performers list", this);
  _RELOAD_PERFS = new QAction(QIcon(":img/RELOAD_FROM_DISK"), "Reload Performers", this);

  _EDIT_PERF_AKA = new QAction(QIcon(":img/EDIT_AKA_FILE"), "Edit AKA perf file", this);
  _RELOAD_PERF_AKA = new QAction(QIcon(":img/RELOAD_FROM_DISK"), "Reload AKA perf", this);

  _RENAME_RULE_STAT = new QAction(QIcon(":img/STATS"), "Rename rule statistics", this);
}

void onShowRenameRuleStatistics() {
  QString statictsContent;
  statictsContent += "Studios number\t";
  statictsContent += QString::number(ProductionStudioManager::getIns().count());
  statictsContent += '\n';
  statictsContent += "Performers number\t";
  statictsContent += QString::number(PerformersManager::getIns().count());
  statictsContent += '\n';
  statictsContent += "AKA number\t";
  statictsContent += QString::number(PerformersAkaManager::getIns().count());
  Notificator::information("Rename rule statistics", statictsContent);
}

void ArrangeActions::subscribe() {
  connect(_EDIT_STUDIOS, &QAction::triggered, this, &onEditStudios);
  connect(_RELOAD_STUDIOS, &QAction::triggered, this, []() {
    static auto& psm = ProductionStudioManager::getIns();
    int itemsCntChanged = psm.ForceReloadStdStudioName();
    Notificator::goodNews("Reload studios", QString("delta %1 items").arg(itemsCntChanged));
  });

  connect(_EDIT_PERFS, &QAction::triggered, this, &onEditPerformers);
  connect(_RELOAD_PERFS, &QAction::triggered, this, []() {
    static auto& pm = PerformersManager::getIns();
    int itemsCntChanged = pm.ForceReloadPerformers();
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
