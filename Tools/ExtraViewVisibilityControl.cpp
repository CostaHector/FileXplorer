#include "ExtraViewVisibilityControl.h"
#include "Actions/PerformersManagerActions.h"
#include "Actions/TorrentsManagerActions.h"

#include "Component/PerformersWidget.h"
#include "Component/TorrentsManagerWidget.h"
#include "PublicVariable.h"

#include <QAbstractItemView>
#include <QMainWindow>
#include <QSqlError>
#include <QSqlQuery>

#include "PublicVariable.h"

ExtraViewVisibilityControl::ExtraViewVisibilityControl(QWidget* parent)
    : QObject(parent), _parent(parent), performerManager(nullptr), torrentsManager(nullptr) {
  this->subscribe();
}

void ExtraViewVisibilityControl::subscribe() {
  connect(g_performersManagerActions().SHOW_PERFORMER_MANAGER, &QAction::triggered, this, &ExtraViewVisibilityControl::onShowOrHidePerformerManger);
  if (g_performersManagerActions().SHOW_PERFORMER_MANAGER->isChecked()) {
    onShowOrHidePerformerManger(true);
  }
  connect(g_torrentsManagerActions().SHOW_TORRENTS_MANAGER, &QAction::triggered, this, &ExtraViewVisibilityControl::onShowOrHideTorrentsManager);
  if (g_torrentsManagerActions().SHOW_TORRENTS_MANAGER->isChecked()) {
    onShowOrHideTorrentsManager(true);
  }
}

auto ExtraViewVisibilityControl::onShowOrHidePerformerManger(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_PERFORMERS_MANAGER_DATABASE.name, isVisible);
  if (!performerManager) {
    performerManager = new PerformersWidget(_parent);
  }
  performerManager->setVisible(isVisible);
}

auto ExtraViewVisibilityControl::onShowOrHideTorrentsManager(const bool isVisible) -> void {
  PreferenceSettings().setValue(MemoryKey::SHOW_TORRENTS_MANAGER_DATABASE.name, isVisible);
  if (!torrentsManager) {
    torrentsManager = new TorrentsManagerWidget(_parent);
  }
  torrentsManager->setVisible(isVisible);
}
