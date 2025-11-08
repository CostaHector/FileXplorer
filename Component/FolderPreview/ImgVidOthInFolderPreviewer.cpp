#include "ImgVidOthInFolderPreviewer.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "MemoryKey.h"
#include <QDir>

constexpr ImgVidOthInFolderPreviewer::MediaBtnHandlerFunc ImgVidOthInFolderPreviewer::MEDIA_HANDLERS_MAP[];

ImgVidOthInFolderPreviewer::ImgVidOthInFolderPreviewer(const QString& memoryName, QWidget* parent)
  : //
  QWidget{parent}
  , mMemoryName{memoryName} {
  mImgVidOtherSplitter = new (std::nothrow) QSplitter{this};
  CHECK_NULLPTR_RETURN_VOID(mImgVidOtherSplitter)
  mImgVidOtherSplitter->setOrientation(Qt::Orientation::Vertical);

  m_bImgVisible = Configuration().value(BrowserKey::FLOATING_IMAGE_VIEW_SHOW.name, BrowserKey::FLOATING_IMAGE_VIEW_SHOW.v).toBool();
  m_bVidVisible = Configuration().value(BrowserKey::FLOATING_VIDEO_VIEW_SHOW.name, BrowserKey::FLOATING_VIDEO_VIEW_SHOW.v).toBool();
  m_bOthVisible = Configuration().value(BrowserKey::FLOATING_OTHER_VIEW_SHOW.name, BrowserKey::FLOATING_OTHER_VIEW_SHOW.v).toBool();

  // init for mImgVidOtherSplitter
  const QString& seqStr = Configuration().value(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name, BrowserKey::FLOATING_MEDIA_TYPE_SEQ.v).toString();
  decltype(mMediaSequence) mediaSequenceMemory;
  if (IsValidMediaTypeSeq(seqStr, mediaSequenceMemory) && mediaSequenceMemory.size() == mMediaSequence.size()) {
    mMediaSequence.swap(mediaSequenceMemory);
  }
  const bool visibility[(int) PREVIEW_ITEM_TYPE::BUTT] = {m_bImgVisible, m_bVidVisible, m_bOthVisible};
  for (int mediaTypeInd : mMediaSequence) { // hide or show each widget in splitter
    (this->*MEDIA_HANDLERS_MAP[mediaTypeInd])(visibility[mediaTypeInd]);
  }
  mImgVidOtherSplitter->restoreState(Configuration().value("FLOATING_PREVIEW_STATE").toByteArray());

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
  QAction* MEDIA_TYPE_2_ACTS[(int) PREVIEW_ITEM_TYPE::BUTT] = {_IMG_ACT, _VID_ACT, _OTH_ACT};
  for (int mediaTypeInd : mMediaSequence) {
    mTypeToDisplayTB->addAction(MEDIA_TYPE_2_ACTS[mediaTypeInd]);
  }
  mTypeToDisplayTB->adjustSize();

  mLo = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(mLo)
  mLo->addWidget(mImgVidOtherSplitter);
  mLo->setSpacing(0);
  mLo->setContentsMargins(0, 0, 0, 0);

  subscribe();
  AdjustButtonPosition();
}

ImgVidOthInFolderPreviewer::~ImgVidOthInFolderPreviewer() {
  SaveState();
}

void ImgVidOthInFolderPreviewer::operator()(const QString& pth) { // file system view
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

void ImgVidOthInFolderPreviewer::operator()(const QString& name,
                                            const QString& jsonAbsFilePath,
                                            const QStringList& imgPthLst,
                                            const QStringList& vidsLst) { // scene view
  if (NeedUpdateImgs()) {
    UpdateImgs(name, imgPthLst);
  }
  if (NeedUpdateVids()) {
    UpdateVids(vidsLst);
  }
  if (NeedUpdateOthers()) {
    QStringList othersList;
    if (!jsonAbsFilePath.isEmpty()) {
      othersList << jsonAbsFilePath;
    }
    UpdateOthers(othersList);
  }
}

void ImgVidOthInFolderPreviewer::UpdateImgs(const QString& name, const QStringList& imgPthLst) {
  if (!NeedUpdateImgs()) {
    return;
  }

  CHECK_NULLPTR_RETURN_VOID(mImgModel)
  mImgModel->UpdateData(imgPthLst);
}

void ImgVidOthInFolderPreviewer::UpdateVids(const QStringList& vidsLst) {
  if (!NeedUpdateVids()) {
    return;
  }
  CHECK_NULLPTR_RETURN_VOID(mVidsModel)
  mVidsModel->UpdateData(vidsLst);
}

void ImgVidOthInFolderPreviewer::UpdateOthers(const QStringList& dataLst) { // no usage now
  if (!NeedUpdateOthers()) {
    return;
  }
  CHECK_NULLPTR_RETURN_VOID(mOthModel)
  mOthModel->UpdateData(dataLst);
}

bool ImgVidOthInFolderPreviewer::onReorder(int fromIndex, int destIndex) {
  CHECK_NULLPTR_RETURN_FALSE(mImgVidOtherSplitter);
  if (!MoveElementFrontOf(mMediaSequence, fromIndex, destIndex)) {
    LOG_W("failed, move widget at index[%d] in front of widget at[%d]", fromIndex, destIndex);
    return false;
  }
  const QString& newMediaTypeSeq = MediaTypeSeqStr(mMediaSequence);
  Configuration().setValue(BrowserKey::FLOATING_MEDIA_TYPE_SEQ.name, newMediaTypeSeq);
  LOG_D("New media type seq[%s]", qPrintable(newMediaTypeSeq));
  return MoveWidgetAtFromIndexInFrontOfDestIndex(fromIndex, destIndex, *mImgVidOtherSplitter);
}

void ImgVidOthInFolderPreviewer::SaveState() {
  Configuration().setValue("FLOATING_PREVIEW_STATE", mImgVidOtherSplitter->saveState());
}

void ImgVidOthInFolderPreviewer::subscribe() {
  connect(mTypeToDisplayTB, &ReorderableToolBar::widgetMoved, this, &ImgVidOthInFolderPreviewer::onReorder);
  connect(mTypeToDisplayTB->mCollectPathAgs, &QActionGroup::triggered, this, &ImgVidOthInFolderPreviewer::onImgVidOthActTriggered);
  connect(mImgVidOtherSplitter, &QSplitter::splitterMoved, this, &ImgVidOthInFolderPreviewer::SaveState);
}

void ImgVidOthInFolderPreviewer::onImgBtnClicked(bool checked) {
  m_bImgVisible = checked;
  if (mImgTv == nullptr) {
    mImgModel = new (std::nothrow) ImgsModel;
    CHECK_NULLPTR_RETURN_VOID(mImgModel)
    mImgTv = new (std::nothrow) ItemView{mMemoryName + "_IMAGE", this};
    CHECK_NULLPTR_RETURN_VOID(mImgTv)
    mImgTv->setViewMode(QListView::ViewMode::IconMode);
    mImgModel->onIconSizeChange(mImgTv->iconSize());
    mImgTv->SetCurrentModel(mImgModel);
    mImgTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    mImgVidOtherSplitter->addWidget(mImgTv);
    connect(mImgTv, &ItemView::iconSizeChanged, mImgModel, &ImgsModel::onIconSizeChange);
  }
  Configuration().setValue(BrowserKey::FLOATING_IMAGE_VIEW_SHOW.name, checked);
  if (mImgTv->isVisible() != checked) {
    mImgTv->setVisible(checked);
  }
}

void ImgVidOthInFolderPreviewer::onVidBtnClicked(bool checked) {
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
  Configuration().setValue(BrowserKey::FLOATING_VIDEO_VIEW_SHOW.name, checked);
  mVidTv->setVisible(checked);
}

void ImgVidOthInFolderPreviewer::onOthBtnClicked(bool checked) {
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
  Configuration().setValue(BrowserKey::FLOATING_OTHER_VIEW_SHOW.name, checked);
  mOthTv->setVisible(checked);
}

void ImgVidOthInFolderPreviewer::onImgVidOthActTriggered(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  const bool checked{pAct->isChecked()};
  if (pAct == _IMG_ACT) {
    onImgBtnClicked(checked);
  } else if (pAct == _VID_ACT) {
    onVidBtnClicked(checked);
  } else if (pAct == _OTH_ACT) {
    onOthBtnClicked(checked);
  } else {
    LOG_W("Action[%s] not supported", qPrintable(pAct->text()));
  }
}
