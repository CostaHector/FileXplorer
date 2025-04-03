#include "ItemView.h"
#include "PublicVariable.h"
#include <QDesktopServices>
#include "Component/NotificatorFrame.h"

ItemView::ItemView(const QString& itemViewName, QWidget* parent) : CustomListView{itemViewName, parent} {
  setViewMode(QListView::ViewMode::ListMode);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setUniformItemSizes(false);

  setResizeMode(QListView::ResizeMode::Adjust);
  setMovement(QListView::Movement::Free);
  setWrapping(true);

  setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

  _PLAY_ITEM = new (std::nothrow) QAction{"Play", this};
  _ORIENTATION_LEFT_TO_RIGHT = new (std::nothrow) QAction{"left to right", this};
  _ORIENTATION_TOP_TO_BOTTOM = new (std::nothrow) QAction{"top to bottom", this};
  _ORIENTATION_GRP = new (std::nothrow) QActionGroup{this};
  _ORIENTATION_GRP->addAction(_ORIENTATION_LEFT_TO_RIGHT);
  _ORIENTATION_GRP->addAction(_ORIENTATION_TOP_TO_BOTTOM);
  _ORIENTATION_LEFT_TO_RIGHT->setCheckable(true);
  _ORIENTATION_TOP_TO_BOTTOM->setCheckable(true);
  _ORIENTATION_GRP->setExclusionPolicy(QActionGroup::ExclusionPolicy::Exclusive);

  if (PreferenceSettings().value(m_name + "_ORIENTATION_LEFT_TO_RIGHT", true).toBool()) {
    _ORIENTATION_LEFT_TO_RIGHT->setChecked(true);
    onOrientationChange(_ORIENTATION_LEFT_TO_RIGHT);
  } else {
    _ORIENTATION_TOP_TO_BOTTOM->setChecked(true);
    onOrientationChange(_ORIENTATION_TOP_TO_BOTTOM);
  }

  mItemMenu = new (std::nothrow) QMenu{"Item Menu", this};
  mItemMenu->addAction(_PLAY_ITEM);
  mItemMenu->addSeparator();
  mItemMenu->addActions(_ORIENTATION_GRP->actions());
  BindMenu(mItemMenu);

  subscribe();
}

void ItemView::subscribe() {
  connect(this, &QListView::doubleClicked, this, &ItemView::onCellDoubleClicked);
  connect(_PLAY_ITEM, &QAction::triggered, this, [this]() { onCellDoubleClicked(currentIndex()); });
  connect(_ORIENTATION_GRP, &QActionGroup::triggered, this, &ItemView::onOrientationChange);
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
  Notificator::information(QString("Try open\n[%1]:").arg(path), QString::number(ret));
}

void ItemView::onOrientationChange(const QAction* pOrientation) {
  if (pOrientation == _ORIENTATION_LEFT_TO_RIGHT) {
    setFlow(QListView::Flow::LeftToRight);
    PreferenceSettings().setValue(m_name + "_ORIENTATION_LEFT_TO_RIGHT", true);
  } else {
    setFlow(QListView::Flow::TopToBottom);
    PreferenceSettings().setValue(m_name + "_ORIENTATION_LEFT_TO_RIGHT", false);
  }
}
