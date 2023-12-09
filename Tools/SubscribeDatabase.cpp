#include "SubscribeDatabase.h"
#include "Actions/DataBaseActions.h"
#include "Actions/PerformersManagerActions.h"
#include "PublicVariable.h"

#include <QAbstractItemView>
#include <QMainWindow>
#include <QSqlError>
#include <QSqlQuery>

void SubscribeDatabase::subscribe() {
  connect(g_dbAct().DB_VIEW_CLOSE_SHOW, &QAction::triggered, this, &SubscribeDatabase::onShowOrCloseDatabase);
  connect(g_performersManagerActions().SHOW_PERFORMER_MANAGER, &QAction::triggered, this, &SubscribeDatabase::onShowOrHidePerformerManger);
  if (g_performersManagerActions().SHOW_PERFORMER_MANAGER->isChecked()) {
    onShowOrHidePerformerManger(true);
  }
}

auto SubscribeDatabase::onShowOrCloseDatabase(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_DATABASE.name, isVisible);
  switchStackWidget();
}

auto SubscribeDatabase::onShowOrHidePerformerManger(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_PERFORMERS_MANGER_DATABASE.name, isVisible);
  if (!performerManger) {
    performerManger = new PerformersManagerWidget(this->view);
  }
  performerManger->setVisible(isVisible);
}
