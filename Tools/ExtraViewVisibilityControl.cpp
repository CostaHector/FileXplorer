#include "ExtraViewVisibilityControl.h"
#include "CastDBActions.h"
#include "TorrDBAction.h"

#include "CastDBView.h"
#include "TorrentsManagerWidget.h"

#include <QDebug>

ExtraViewVisibilityControl::ExtraViewVisibilityControl(QWidget* parent)
    : QObject(parent), _parent(parent), torrentsManager(nullptr), m_recycleBin{nullptr} {
  subscribe();
}

void ExtraViewVisibilityControl::subscribe() {
  connect(g_torrActions().SHOW_TORRENTS_MANAGER, &QAction::triggered, this, [this](const bool checked) -> void {
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
