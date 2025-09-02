#include "ExtraEvents.h"
#include "TorrDBAction.h"
#include "TorrentsManagerWidget.h"
#include "OnCheckedPopupOrHideAWidget.h"

ExtraEvents::ExtraEvents(QWidget* parent)
  : QObject{parent}, parentWidget{parent} {
}

void ExtraEvents::subscribe() {
  connect(g_torrActions().SHOW_TORRENTS_MANAGER, &QAction::toggled, this, &ExtraEvents::on_showTorrentsManager);
}

void ExtraEvents::on_showTorrentsManager(const bool checked) {
  mTorrentsManager = PopupHideWidget<TorrentsManagerWidget>(mTorrentsManager, checked, parentWidget);
}
