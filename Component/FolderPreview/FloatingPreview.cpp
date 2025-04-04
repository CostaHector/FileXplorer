#include "FloatingPreview.h"
#include "PublicVariable.h"
#include "Tools/PathTool.h"
#include <QDir>
#include <QHeaderView>
#include <QMenu>
#include <QScrollBar>
#include <QLayout>

FloatingPreview::FloatingPreview(QWidget* parent) : QSplitter{parent} {
  setOrientation(Qt::Orientation::Vertical);

  _IMG_ENABLED = new QAction(QIcon(":img/IMAGE"), "Images", this);
  _VID_ENABLED = new QAction(QIcon(":img/VIDEO"), "Videos", this);
  _OTH_ENABLED = new QAction(QIcon(":img/FILE"), "Others", this);
  _IMG_ENABLED->setCheckable(true);
  _VID_ENABLED->setCheckable(true);
  _OTH_ENABLED->setCheckable(true);

  mTypeToDisplayTB = new QToolBar{"Type To Display", this};
  mTypeToDisplayTB->addAction(_IMG_ENABLED);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(_VID_ENABLED);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(_OTH_ENABLED);

  mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mTypeToDisplayTB->setMaximumHeight(35);
  addWidget(mTypeToDisplayTB);

  _IMG_ENABLED->setChecked(PreferenceSettings().value("FLOATING_IMAGE_VIEW_SHOW", true).toBool());
  _VID_ENABLED->setChecked(PreferenceSettings().value("FLOATING_VIDEO_VIEW_SHOW", false).toBool());
  _OTH_ENABLED->setChecked(PreferenceSettings().value("FLOATING_OTHER_VIEW_SHOW", false).toBool());

  onImgBtnClicked(_IMG_ENABLED->isChecked());
  onVidBtnClicked(_VID_ENABLED->isChecked());
  onOthBtnClicked(_OTH_ENABLED->isChecked());

  subscribe();

  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
}

void FloatingPreview::ReadSettings() {
  if (PreferenceSettings().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
    restoreState(PreferenceSettings().value("FLOATING_PREVIEW_STATE").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FloatingPreview::SaveSettings() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

void FloatingPreview::SaveState() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_STATE", saveState());
}

void FloatingPreview::operator()(const QString& pth) {  // file system
  if (!NeedUpdate(pth)) {
    return;
  }

  mLastName = pth;
  setWindowTitle(mLastName);

  if (!QDir(pth).exists()) {
    // not a folder
    if (NeedUpdateImgs() && !mImgModel->IsEmpty()) {
      mImgModel->Clear();
    }
    if (NeedUpdateVids() && !mVidsModel->IsEmpty()) {
      mVidsModel->Clear();
    }
    if (NeedUpdateOthers() && !mOthModel->IsEmpty()) {
      mOthModel->Clear();
    }
    if (_IMG_ENABLED->text() != "Images") {
      _IMG_ENABLED->setText("Images");
    }
    if (_VID_ENABLED->text() != "Videos") {
      _VID_ENABLED->setText("Videos");
    }
    if (_OTH_ENABLED->text() != "Others") {
      _OTH_ENABLED->setText("Others");
    }

    QString base, extension;
    std::tie(base, extension) = PATHTOOL::GetBaseNameExt(pth);
    if (TYPE_FILTER::IMAGE_TYPE_SET.contains('*' + extension)) {
      int cnt = 0;
      if (NeedUpdateImgs()) {
        mImgTv->setEnabled(true);
        cnt = mImgModel->UpdateData({pth});
      }
      _IMG_ENABLED->setText(QString("%1 Images").arg(cnt, 3, 10));
    }
    return;
  }

  if (NeedUpdateImgs()) {
    const int imgCnt = mImgModel->setDirPath(pth, TYPE_FILTER::IMAGE_TYPE_SET, false);
    _IMG_ENABLED->setText(QString("%1 Images").arg(imgCnt, 3, 10));
  }
  if (NeedUpdateVids()) {
    const int vidCnt = mVidsModel->setDirPath(pth, TYPE_FILTER::VIDEO_TYPE_SET, true);
    _VID_ENABLED->setText(QString("%1 Videos").arg(vidCnt, 3, 10));
  }
  if (NeedUpdateOthers()) {
    const int othCnt = mOthModel->setDirPath(pth, TYPE_FILTER::TEXT_TYPE_SET, true);
    _OTH_ENABLED->setText(QString("%1 Others").arg(othCnt, 3, 10));
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
  connect(_IMG_ENABLED, &QAction::triggered, this, &FloatingPreview::onImgBtnClicked);
  connect(_VID_ENABLED, &QAction::triggered, this, &FloatingPreview::onVidBtnClicked);
  connect(_OTH_ENABLED, &QAction::triggered, this, &FloatingPreview::onOthBtnClicked);
  connect(this, &QSplitter::splitterMoved, this, &FloatingPreview::SaveState);
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
  mImgTv->setVisible(checked);
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
  mVidTv->setVisible(checked);
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
  mOthTv->setVisible(checked);
}
