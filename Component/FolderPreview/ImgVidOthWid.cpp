#include "ImgVidOthWid.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include <QDir>

constexpr ImgVidOthWid::MediaBtnHandlerFunc ImgVidOthWid::MEDIA_HANDLERS_MAP[];

ImgVidOthWid::ImgVidOthWid(const QString& memoryName, QWidget* parent)://
  QWidget{parent}, mMemoryName{memoryName} {
  mImgVidOtherSplitter = new (std::nothrow) QSplitter{this};
  CHECK_NULLPTR_RETURN_VOID(mImgVidOtherSplitter)
  mImgVidOtherSplitter->setOrientation(Qt::Orientation::Vertical);

  m_bImgVisible = PreferenceSettings().value("FLOATING_IMAGE_VIEW_SHOW", true).toBool();
  m_bVidVisible = PreferenceSettings().value("FLOATING_VIDEO_VIEW_SHOW", false).toBool();
  m_bOthVisible = PreferenceSettings().value("FLOATING_OTHER_VIEW_SHOW", false).toBool();

  // init for mImgVidOtherSplitter
  const QString& defaultMediaTypeSeq = MediaTypeSeqStr(mMediaSequence);
  const QString& seqStr = PreferenceSettings().value("FLOATING_MEDIA_TYPE_SEQ", defaultMediaTypeSeq).toString();
  decltype(mMediaSequence) mediaSequenceMemory;
  if (IsValidMediaTypeSeq(seqStr, mediaSequenceMemory) && mediaSequenceMemory.size() == mMediaSequence.size()) {
    mMediaSequence.swap(mediaSequenceMemory);
  }
  const bool visibility[(int)PREVIEW_ITEM_TYPE::BUTT] = {m_bImgVisible, m_bVidVisible, m_bOthVisible};
  for (int mediaTypeInd : mMediaSequence) { // hide or show each widget in splitter
    (this->*MEDIA_HANDLERS_MAP[mediaTypeInd])(visibility[mediaTypeInd]);
  }
  mImgVidOtherSplitter->restoreState(PreferenceSettings().value("FLOATING_PREVIEW_STATE").toByteArray());

  // init for actions
  _IMG_ACT = new (std::nothrow) QAction{QIcon{":img/IMAGE"}, "0", this};
  CHECK_NULLPTR_RETURN_VOID(_IMG_ACT)
  _IMG_ACT->setToolTip("Image(s) file count");
  _IMG_ACT->setCheckable(true);
  _IMG_ACT->setChecked(m_bImgVisible);

  _VID_ACT = new (std::nothrow) QAction{QIcon{":img/VIDEO"}, "0", this};
  CHECK_NULLPTR_RETURN_VOID(_VID_ACT)
  _VID_ACT->setToolTip("Video(s) file count");
  _VID_ACT->setCheckable(true);
  _VID_ACT->setChecked(m_bVidVisible);

  _OTH_ACT = new (std::nothrow) QAction{QIcon{":img/FILE"}, "0", this};
  CHECK_NULLPTR_RETURN_VOID(_OTH_ACT)
  _OTH_ACT->setToolTip("Other(s) file count");
  _OTH_ACT->setCheckable(true);
  _OTH_ACT->setChecked(m_bOthVisible);

  // init for mTypeToDisplayTB
  mTypeToDisplayTB = new (std::nothrow) ReorderableToolBar{"Type To Display", this};
  CHECK_NULLPTR_RETURN_VOID(mTypeToDisplayTB)
  mTypeToDisplayTB->setOrientation(Qt::Orientation::Vertical);
  mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  static QAction* MEDIA_TYPE_2_ACTS[(int)PREVIEW_ITEM_TYPE::BUTT] = {_IMG_ACT, _VID_ACT, _OTH_ACT};
  for (int mediaTypeInd : mMediaSequence) {
    mTypeToDisplayTB->addAction(MEDIA_TYPE_2_ACTS[mediaTypeInd]);
  }
  mTypeToDisplayTB->adjustSize();

  mLo = new (std::nothrow)QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(mLo)
  mLo->addWidget(mImgVidOtherSplitter);
  mLo->setSpacing(0);
  mLo->setContentsMargins(0, 0, 0, 0);

  subscribe();
  AdjustButtonPosition();
}

void ImgVidOthWid::operator()(const QString& pth) {  // file system view
  if (NeedUpdateImgs()) {
    const int imgCnt = mImgModel->setDirPath(pth, TYPE_FILTER::IMAGE_TYPE_SET, false);
    _IMG_ACT->setText(QString::number(imgCnt));
  }
  if (NeedUpdateVids()) {
    const int vidCnt = mVidsModel->setDirPath(pth, TYPE_FILTER::VIDEO_TYPE_SET, true);
    _VID_ACT->setText(QString::number(vidCnt));
  }
  if (NeedUpdateOthers()) {
    const int othCnt = mOthModel->setDirPath(pth, TYPE_FILTER::TEXT_TYPE_SET, true);
    _OTH_ACT->setText(QString::number(othCnt));
  }
}

void ImgVidOthWid::operator()(const QString& /*name*/, const QString& pth) {  // scene view
  CHECK_NULLPTR_RETURN_VOID(mImgModel)
  QDir dir{pth, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  mImgModel->UpdateData(dir.entryList());
}

void ImgVidOthWid::UpdateImgs(const QString& name, const QStringList& imgPthLst) {
  if (!NeedUpdateImgs()) {
    return;
  }

  CHECK_NULLPTR_RETURN_VOID(mImgModel)
  mImgModel->UpdateData(imgPthLst);
}

void ImgVidOthWid::UpdateVids(const QStringList& dataLst) {
  if (!NeedUpdateVids()) {
    return;
  }
  CHECK_NULLPTR_RETURN_VOID(mVidsModel)
  mVidsModel->UpdateData(dataLst);
}

void ImgVidOthWid::UpdateOthers(const QStringList& dataLst) {
  if (!NeedUpdateOthers()) {
    return;
  }
  CHECK_NULLPTR_RETURN_VOID(mOthModel)
  mOthModel->UpdateData(dataLst);
}

bool ImgVidOthWid::onReorder(int fromIndex, int destIndex) {
  CHECK_NULLPTR_RETURN_FALSE(mImgVidOtherSplitter);
  if (!MoveElementFrontOf(mMediaSequence, fromIndex, destIndex)) {
    qWarning("failed, move widget at index[%d] in front of widget at[%d]", fromIndex, destIndex);
    return false;
  }
  const QString& newMediaTypeSeq = MediaTypeSeqStr(mMediaSequence);
  PreferenceSettings().setValue("FLOATING_MEDIA_TYPE_SEQ", newMediaTypeSeq);
  qDebug("New media type seq[%s]", qPrintable(newMediaTypeSeq));
  return MoveWidgetAtFromIndexInFrontOfDestIndex(fromIndex, destIndex, *mImgVidOtherSplitter);
}

void ImgVidOthWid::SaveState() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_STATE", mImgVidOtherSplitter->saveState());
}

void ImgVidOthWid::subscribe() {
  connect(mTypeToDisplayTB, &ReorderableToolBar::widgetMoved, this, &ImgVidOthWid::onReorder);
  connect(mTypeToDisplayTB->mCollectPathAgs, &QActionGroup::triggered, this, &ImgVidOthWid::onImgVidOthActTriggered);
  connect(mImgVidOtherSplitter, &QSplitter::splitterMoved, this, &ImgVidOthWid::SaveState);
}

void ImgVidOthWid::onImgBtnClicked(bool checked) {
  m_bImgVisible = checked;
  if (mImgTv == nullptr) {
    mImgModel = new (std::nothrow) ImgsModel;
    CHECK_NULLPTR_RETURN_VOID(mImgModel)
    mImgTv = new (std::nothrow) ItemView{mMemoryName + "_IMAGE", this};
    CHECK_NULLPTR_RETURN_VOID(mImgTv)
    mImgTv->SetCurrentModel(mImgModel);
    mImgTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    mImgVidOtherSplitter->addWidget(mImgTv);
  }
  PreferenceSettings().setValue("FLOATING_IMAGE_VIEW_SHOW", checked);
  if (mImgTv->isVisible() != checked) {
    mImgTv->setVisible(checked);
  }
}

void ImgVidOthWid::onVidBtnClicked(bool checked) {
  m_bVidVisible = checked;
  if (mVidTv == nullptr) {
    mVidsModel = new (std::nothrow) VidsModel;
    CHECK_NULLPTR_RETURN_VOID(mVidsModel)
    mVidTv = new (std::nothrow) ItemView{mMemoryName + "_VIDEO", this};
    CHECK_NULLPTR_RETURN_VOID(mVidTv)
    mVidTv->SetCurrentModel(mVidsModel);
    mVidTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    mImgVidOtherSplitter->addWidget(mVidTv);
  }
  PreferenceSettings().setValue("FLOATING_VIDEO_VIEW_SHOW", checked);
  mVidTv->setVisible(checked);
}

void ImgVidOthWid::onOthBtnClicked(bool checked) {
  m_bOthVisible = checked;
  if (mOthTv == nullptr) {
    mOthModel = new (std::nothrow) OthersModel;
    CHECK_NULLPTR_RETURN_VOID(mOthModel)
    mOthTv = new (std::nothrow) ItemView{mMemoryName + "_OTHER", this};
    CHECK_NULLPTR_RETURN_VOID(mOthTv)
    mOthTv->SetCurrentModel(mOthModel);
    mOthTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    mImgVidOtherSplitter->addWidget(mOthTv);
  }
  PreferenceSettings().setValue("FLOATING_OTHER_VIEW_SHOW", checked);
  mOthTv->setVisible(checked);
}

void ImgVidOthWid::onImgVidOthActTriggered(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  const bool checked{pAct->isChecked()};
  if (pAct == _IMG_ACT) {
    onImgBtnClicked(checked);
  } else if (pAct == _VID_ACT) {
    onVidBtnClicked(checked);
  } else if (pAct == _OTH_ACT) {
    onOthBtnClicked(checked);
  } else {
    qWarning("Action[%s] not supported", qPrintable(pAct->text()));
  }
}

// #define RUN_MAIN_FILE 1
// #ifdef RUN_MAIN_FILE
// #include <QApplication>
// int main(int argc, char* argv[]) {
//   QApplication a(argc, argv);
//   ImgVidOthWid w{"DockerList"};
//   w.show();
//   w("");
//   a.exec();
//   return 0;
// }
// #endif
