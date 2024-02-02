#include "SubscribeDatabase.h"
#include "Actions/DataBaseActions.h"
#include "Actions/PerformersManagerActions.h"
#include "Actions/TorrentsManagerActions.h"

#include "Component/PerformersWidget.h"
#include "Component/TorrentsManagerWidget.h"
#include "PublicVariable.h"

#include <QAbstractItemView>
#include <QMainWindow>
#include <QSqlError>
#include <QSqlQuery>

SubscribeDatabase::SubscribeDatabase(QTableView* view_, T_SwitchStackWidget switchStackWidget_)
    : QObject(), view(view_), switchStackWidget(switchStackWidget_), performerManager(nullptr), torrentsManager(nullptr) {
  this->subscribe();
}

void SubscribeDatabase::subscribe() {
  connect(g_dbAct().DB_VIEW_CLOSE_SHOW, &QAction::triggered, this, &SubscribeDatabase::onShowOrCloseDatabase);
  connect(g_performersManagerActions().SHOW_PERFORMER_MANAGER, &QAction::triggered, this, &SubscribeDatabase::onShowOrHidePerformerManger);
  if (g_performersManagerActions().SHOW_PERFORMER_MANAGER->isChecked()) {
    onShowOrHidePerformerManger(true);
  }
  connect(g_torrentsManagerActions().SHOW_TORRENTS_MANAGER, &QAction::triggered, this, &SubscribeDatabase::onShowOrHideTorrentsManager);
  if (g_torrentsManagerActions().SHOW_TORRENTS_MANAGER->isChecked()) {
    onShowOrHideTorrentsManager(true);
  }
}

auto SubscribeDatabase::onShowOrCloseDatabase(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_DATABASE.name, isVisible);
  switchStackWidget();
}

auto SubscribeDatabase::onShowOrHidePerformerManger(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_PERFORMERS_MANAGER_DATABASE.name, isVisible);
  if (!performerManager) {
    performerManager = new PerformersWidget(this->view);
  }
  performerManager->setVisible(isVisible);
}

auto SubscribeDatabase::onShowOrHideTorrentsManager(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_TORRENTS_MANAGER_DATABASE.name, isVisible);
  if (!torrentsManager) {
    torrentsManager = new TorrentsManagerWidget(this->view);
  }
  torrentsManager->setVisible(isVisible);
}
