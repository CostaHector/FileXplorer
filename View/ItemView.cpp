#include "ItemView.h"
#include "PublicMacro.h"
#include "NotificatorMacro.h"
#include <QActionGroup>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>

ItemView::ItemView(const QString& itemViewName, QWidget* parent)  //
    : CustomListView{itemViewName, parent} {
  setViewMode(QListView::ViewMode::ListMode);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setUniformItemSizes(false);

  setResizeMode(QListView::ResizeMode::Adjust);
  setMovement(QListView::Movement::Free);
  setWrapping(true);

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

bool ItemView::onCellDoubleClicked(const QModelIndex& clickedIndex) const {
  CHECK_NULLPTR_RETURN_FALSE(mModels);
  if (!clickedIndex.isValid()) {
    return false;
  }
  const QString& path = mModels->filePath(clickedIndex);
  if (!QFile::exists(path)) {
    return false;
  }
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  const bool ret = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
  LOG_OE_P(ret, "Double click open", "path:%s", qPrintable(path));
  return ret;
}
