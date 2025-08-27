#include "ItemView.h"
#include "NotificatorMacro.h"
#include <QActionGroup>
#include <QDesktopServices>
#include <QUrl>

ItemView::ItemView(const QString& itemViewName, QWidget* parent)//
    : CustomListView{itemViewName, parent} {
  setViewMode(QListView::ViewMode::ListMode);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setUniformItemSizes(false);

  setResizeMode(QListView::ResizeMode::Adjust);
  setMovement(QListView::Movement::Free);
  setWrapping(true);

  setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

  _PLAY_ITEM = new (std::nothrow) QAction{"Play", this};
  mItemMenu = new (std::nothrow) QMenu{"Item Menu", this};
  mItemMenu->addAction(_PLAY_ITEM);
  BindMenu(mItemMenu);

  subscribe();
}

void ItemView::subscribe() {
  connect(this, &QListView::doubleClicked, this, &ItemView::onCellDoubleClicked);
  connect(_PLAY_ITEM, &QAction::triggered, this, [this]() { onCellDoubleClicked(currentIndex()); });
}

void ItemView::onCellDoubleClicked(const QModelIndex& clickedIndex) const {
  if (mModels == nullptr) {
    return;
  }
  if (!clickedIndex.isValid()) {
    return;
  }
  const QString& path = mModels->filePath(clickedIndex);
  const bool ret = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
  if (ret) {
    LOG_GOOD_NP("[Ok] Open", path);
  } else {
    LOG_BAD_NP("[Failed] Open", path);
  }
}
