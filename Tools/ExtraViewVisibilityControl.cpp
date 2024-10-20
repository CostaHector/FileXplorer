#include "ExtraViewVisibilityControl.h"
#include "Actions/PerformersManagerActions.h"
#include "Actions/TorrentsManagerActions.h"

#include "Component/PerformersWidget.h"
#include "View/TorrentsManagerWidget.h"

#include <QDebug>

ExtraViewVisibilityControl::ExtraViewVisibilityControl(QWidget* parent)
    : QObject(parent), _parent(parent), performerManager(nullptr), torrentsManager(nullptr), m_recycleBin{nullptr} {
  subscribe();
}

void ExtraViewVisibilityControl::subscribe() {
  connect(g_performersManagerActions().SHOW_PERFORMER_MANAGER, &QAction::triggered, this, [this](const bool checked) -> void {
    if (not checked) {
      if (performerManager != nullptr)
        performerManager->hide();
      return;
    }
    if (performerManager == nullptr) {
      performerManager = new PerformersWidget(_parent);
    }
    performerManager->show();
    performerManager->activateWindow();
    performerManager->raise();
  });
  connect(g_torrentsManagerActions().SHOW_TORRENTS_MANAGER, &QAction::triggered, this, [this](const bool checked) -> void {
    if (not checked) {
      if (torrentsManager != nullptr)
        torrentsManager->hide();
      return;
    }
    if (torrentsManager == nullptr) {
      torrentsManager = new TorrentsManagerWidget(_parent);
    }
    torrentsManager->show();
    torrentsManager->activateWindow();
    torrentsManager->raise();
  });
}
