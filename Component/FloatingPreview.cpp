#include "FloatingPreview.h"
#include "PublicVariable.h"
#include "NotificatorFrame.h"
#include <QDir>
#include <QHeaderView>
#include <QDesktopServices>
#include <QMenu>
#include <QScrollBar>
#include <QLayout>

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

// -----------------

FloatingPreview::FloatingPreview(QWidget* parent) : QSplitter{parent} {
  setOrientation(Qt::Orientation::Vertical);

  mImgBtn = new QAction(QIcon(":img/IMAGE"), "Images", this);
  mVidsBtn = new QAction(QIcon(":img/VIDEO"), "Videos", this);
  mOthersBtn = new QAction(QIcon(":img/FILE"), "Others", this);
  mImgBtn->setCheckable(true);
  mVidsBtn->setCheckable(true);
  mOthersBtn->setCheckable(true);

  mTypeToDisplayTB = new QToolBar{"Type To Display", this};
  mTypeToDisplayTB->addAction(mImgBtn);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(mVidsBtn);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(mOthersBtn);
  mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mTypeToDisplayTB->setMaximumHeight(40);
  addWidget(mTypeToDisplayTB);

  mImgBtn->setChecked(PreferenceSettings().value("FLOATING_IMAGE_VIEW_SHOW", true).toBool());
  mVidsBtn->setChecked(PreferenceSettings().value("FLOATING_VIDEO_VIEW_SHOW", false).toBool());
  mOthersBtn->setChecked(PreferenceSettings().value("FLOATING_OTHER_VIEW_SHOW", false).toBool());

  onImgBtnClicked(mImgBtn->isChecked());
  onVidBtnClicked(mVidsBtn->isChecked());
  onOthBtnClicked(mOthersBtn->isChecked());

  subscribe();

  ReadSettings();
}

void FloatingPreview::ReadSettings() {
  if (PreferenceSettings().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FloatingPreview::SaveSettings() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

void FloatingPreview::operator()(const QString& pth) {  // file system
  if (!NeedUpdate(pth)) {
    return;
  }
  mLastName = pth;
  setWindowTitle(mLastName);
  QDir dir(pth, "", QDir::SortFlag::Name, QDir::Filter::Files);
  if (!dir.exists()) {
    if (NeedUpdateImgs()) {
      mImgModel->UpdateData({});
    }
    if (NeedUpdateVids()) {
      mVidsModel->UpdateData({});
    }
    if (NeedUpdateOthers()) {
      mOthModel->UpdateData({});
    }
    return;
  }
  if (NeedUpdateImgs()) {
    mImgModel->setDirPath(pth);
  }
  if (NeedUpdateVids()) {
    dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    QStringList vids;
    for (const QString& name : dir.entryList()) {
      vids.append(pth + '/' + name);
    }
    mVidsModel->UpdateData(vids);
  }
  if (NeedUpdateOthers()) {
    dir.setNameFilters(TYPE_FILTER::TEXT_TYPE_SET);
    QStringList others;
    for (const QString& name : dir.entryList()) {
      others.append(pth + '/' + name);
    }
    mOthModel->UpdateData(others);
  }
}

void FloatingPreview::operator()(const QString& name, const QString& pth) {  // scene
  mLastName = name;
  setWindowTitle(mLastName);

  QDir dir{pth, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  mImgModel->UpdateData(dir.entryList());
}

void FloatingPreview::UpdateImgs(const QString& name, const QStringList& imgPthLst) {
  mLastName = name;
  setWindowTitle(mLastName);
  mImgModel->UpdateData(imgPthLst);
}

void FloatingPreview::UpdateVids(const QStringList& dataLst) {
  if (mVidsModel == nullptr) {
    return;
  }
  mVidsModel->UpdateData(dataLst);
}

void FloatingPreview::UpdateOthers(const QStringList& dataLst) {
  if (mOthModel == nullptr) {
    return;
  }
  mOthModel->UpdateData(dataLst);
}

void FloatingPreview::subscribe() {
  connect(mImgBtn, &QAction::triggered, this, &FloatingPreview::onImgBtnClicked);
  connect(mVidsBtn, &QAction::triggered, this, &FloatingPreview::onVidBtnClicked);
  connect(mOthersBtn, &QAction::triggered, this, &FloatingPreview::onOthBtnClicked);
}

void FloatingPreview::onImgBtnClicked(bool checked) {
  if (mImgTv == nullptr) {
    mImgModel = new (std::nothrow) ImgsModel;
    mImgTv = new (std::nothrow) ItemView{"FLOATING_IMAGE_VIEW", this};
    mImgTv->SetCurrentModel(mImgModel);
    mImgTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    insertWidget(count() - 1, mImgTv);
  }
  PreferenceSettings().setValue("FLOATING_IMAGE_VIEW_SHOW", checked);
  if (checked) {
    mImgBtn->setText("v Images");
    mImgTv->show();
  } else {
    mImgBtn->setText("> Images");
    mImgTv->hide();
  }
}

void FloatingPreview::onVidBtnClicked(bool checked) {
  if (mVidTv == nullptr) {
    mVidsModel = new (std::nothrow) VidsModel;
    mVidTv = new (std::nothrow) ItemView{"FLOATING_VIDEO_VIEW", this};
    mVidTv->SetCurrentModel(mVidsModel);
    mVidTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    insertWidget(count() - 1, mVidTv);
  }
  PreferenceSettings().setValue("FLOATING_VIDEO_VIEW_SHOW", checked);
  if (checked) {
    mVidsBtn->setText("v Videos");
    mVidTv->show();
  } else {
    mVidsBtn->setText("> Videos");
    mVidTv->hide();
  }
}

void FloatingPreview::onOthBtnClicked(bool checked) {
  if (mOthTv == nullptr) {
    mOthModel = new (std::nothrow) OthersModel;
    mOthTv = new (std::nothrow) ItemView{"FLOATING_OTHER_VIEW", this};
    mOthTv->SetCurrentModel(mOthModel);
    mOthTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    insertWidget(count() - 1, mOthTv);
  }
  PreferenceSettings().setValue("FLOATING_OTHER_VIEW_SHOW", checked);
  if (checked) {
    mOthersBtn->setText("v Others");
    mOthTv->show();
  } else {
    mOthersBtn->setText("> Others");
    mOthTv->hide();
  }
}
