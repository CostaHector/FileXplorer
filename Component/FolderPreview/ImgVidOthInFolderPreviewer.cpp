#include "ImgVidOthInFolderPreviewer.h"
#include "PublicMacro.h"
#include "PublicVariable.h"
#include "PathTool.h"
#include "BrowserKey.h"
#include "Configuration.h"

constexpr ImgVidOthInFolderPreviewer::MediaBtnHandlerFunc ImgVidOthInFolderPreviewer::MEDIA_HANDLERS_MAP[];

ImgVidOthInFolderPreviewer::ImgVidOthInFolderPreviewer(const QString& memoryName, QWidget* parent)
    : FullScreenableSplitter{memoryName, Qt::Orientation::Vertical, parent} {
  m_bImgVisible = getConfig(BrowserKey::FLOATING_IMAGE_VIEW_SHOW).toBool();
  m_bVidVisible = getConfig(BrowserKey::FLOATING_VIDEO_VIEW_SHOW).toBool();
  m_bOthVisible = getConfig(BrowserKey::FLOATING_OTHER_VIEW_SHOW).toBool();

  const QString& seqStr = getConfig(BrowserKey::FLOATING_MEDIA_TYPE_SEQ).toString();
  decltype(mMediaSequence) mediaSequenceMemory;
  if (IsValidMediaTypeSeq(seqStr, mediaSequenceMemory) && mediaSequenceMemory.size() == mMediaSequence.size()) {
    mMediaSequence.swap(mediaSequenceMemory);
  }
  const bool visibility[(int)PREVIEW_ITEM_TYPE::BUTT] = {m_bImgVisible, m_bVidVisible, m_bOthVisible};
  for (int mediaTypeInd : mMediaSequence) {  // hide or show each widget in splitter
    (this->*MEDIA_HANDLERS_MAP[mediaTypeInd])(visibility[mediaTypeInd]);
  }

  // init for actions
  _IMG_ACT = new (std::nothrow) QAction{QIcon{":img/IMAGE"}, GetActionText(PREVIEW_ITEM_TYPE::IMG, 0), this};
  CHECK_NULLPTR_RETURN_VOID(_IMG_ACT)
  _IMG_ACT->setToolTip("Image(s) file count");
  _IMG_ACT->setCheckable(true);
  _IMG_ACT->setChecked(m_bImgVisible);

  _VID_ACT = new (std::nothrow) QAction{QIcon{":img/VIDEO"}, GetActionText(PREVIEW_ITEM_TYPE::VID, 0), this};
  CHECK_NULLPTR_RETURN_VOID(_VID_ACT)
  _VID_ACT->setToolTip("Video(s) file count");
  _VID_ACT->setCheckable(true);
  _VID_ACT->setChecked(m_bVidVisible);

  _OTH_ACT = new (std::nothrow) QAction{QIcon{":img/FILE"}, GetActionText(PREVIEW_ITEM_TYPE::OTH, 0), this};
  CHECK_NULLPTR_RETURN_VOID(_OTH_ACT)
  _OTH_ACT->setToolTip("Other(s) file count");
  _OTH_ACT->setCheckable(true);
  _OTH_ACT->setChecked(m_bOthVisible);

  // init for mTypeToDisplayTB
  mTypeToDisplayTB = new (std::nothrow) ReorderableToolBar{"Type To Display", this};
  CHECK_NULLPTR_RETURN_VOID(mTypeToDisplayTB)
  mTypeToDisplayTB->setOrientation(Qt::Orientation::Horizontal);
  mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  QAction* MEDIA_TYPE_2_ACTS[(int)PREVIEW_ITEM_TYPE::BUTT] = {_IMG_ACT, _VID_ACT, _OTH_ACT};
  for (int mediaTypeInd : mMediaSequence) {
    mTypeToDisplayTB->addAction(MEDIA_TYPE_2_ACTS[mediaTypeInd]);
  }
  mTypeToDisplayTB->setContentsMargins(0, 0, 0, 0);
  mTypeToDisplayTB->setMaximumHeight(24);
  mTypeToDisplayTB->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Minimum);
  addWidget(mTypeToDisplayTB);

  setContentsMargins(0, 0, 0, 0);
  restoreState(Configuration().value(GetMemoryName() + "/STATE").toByteArray());
  subscribe();
}

ImgVidOthInFolderPreviewer::~ImgVidOthInFolderPreviewer() {  //
  saveStateInDerivedDestructor();
}

void ImgVidOthInFolderPreviewer::operator()(const QString& pth) {  // file system view
  if (NeedUpdateImgs()) {
    const int imgCnt = mImgModel->setDirPath(pth, TYPE_FILTER::IMAGE_TYPE_SET, false);
    _IMG_ACT->setText(GetActionText(PREVIEW_ITEM_TYPE::IMG, imgCnt));
  }
  if (NeedUpdateVids()) {
    const int vidCnt = mVidTv->PlayAPath(pth, false);
    _VID_ACT->setText(GetActionText(PREVIEW_ITEM_TYPE::VID, vidCnt));
  }
  if (NeedUpdateOthers()) {
    const int othCnt = mOthModel->setDirPath(pth, TYPE_FILTER::TEXT_TYPE_SET, true);
    _OTH_ACT->setText(GetActionText(PREVIEW_ITEM_TYPE::OTH, othCnt));
  }
}

void ImgVidOthInFolderPreviewer::operator()(const QString& name,
                                            const QString& jsonAbsFilePath,
                                            const QStringList& imgPthLst,
                                            const QStringList& vidsLst) {  // scene view, json view
  if (NeedUpdateImgs()) {
    UpdateImgs(name, imgPthLst);
  }
  const QString& rootPath = PathTool::absolutePath(jsonAbsFilePath);
  if (NeedUpdateVids()) {
    UpdateVids(rootPath, vidsLst);
  }
  if (NeedUpdateOthers()) {
    QStringList othersList;
    if (!jsonAbsFilePath.isEmpty()) {
      othersList << jsonAbsFilePath;
    }
    UpdateOthers(othersList);
  }
}

void ImgVidOthInFolderPreviewer::onStopPlaying() {
  CHECK_NULLPTR_RETURN_VOID(mVidTv);
  mVidTv->onStopPlaying();
}

bool ImgVidOthInFolderPreviewer::UpdateImgs(const QString& name, const QStringList& imgPthLst) {
  if (!NeedUpdateImgs()) {
    return false;
  }

  CHECK_NULLPTR_RETURN_FALSE(mImgModel)
  mImgModel->UpdateData(imgPthLst);
  return true;
}

bool ImgVidOthInFolderPreviewer::UpdateVids(const QString& rootPath, const QStringList& vidsLst) {
  if (!NeedUpdateVids()) {
    return false;
  }
  CHECK_NULLPTR_RETURN_FALSE(mVidTv)
  mVidTv->onStopPlaying();
  mVidTv->PlayVideos(rootPath, vidsLst, false);
  return true;
}

bool ImgVidOthInFolderPreviewer::UpdateOthers(const QStringList& dataLst) {  // no usage now
  if (!NeedUpdateOthers()) {
    return false;
  }
  CHECK_NULLPTR_RETURN_FALSE(mOthModel)
  mOthModel->UpdateData(dataLst);
  return true;
}

bool ImgVidOthInFolderPreviewer::onReorder(int fromIndex, int destIndex) {
  if (!MoveElementFrontOf(mMediaSequence, fromIndex, destIndex)) {
    LOG_W("failed, move widget at index[%d] in front of widget at[%d]", fromIndex, destIndex);
    return false;
  }
  const QString& newMediaTypeSeq = MediaTypeSeqStr(mMediaSequence);
  setConfig(BrowserKey::FLOATING_MEDIA_TYPE_SEQ, newMediaTypeSeq);
  LOG_D("New media type seq[%s]", qPrintable(newMediaTypeSeq));
  return MoveWidgetAtFromIndexInFrontOfDestIndex(fromIndex, destIndex, *this);
}

QString ImgVidOthInFolderPreviewer::GetActionText(PREVIEW_ITEM_TYPE itemType, int cnt) const {
  switch (itemType) {
    case PREVIEW_ITEM_TYPE::IMG:
      return QString::asprintf("Images: %d", cnt);
    case PREVIEW_ITEM_TYPE::VID:
      return QString::asprintf("Videos: %d", cnt);
    case PREVIEW_ITEM_TYPE::OTH:
      return QString::asprintf("Others: %d", cnt);
    case PREVIEW_ITEM_TYPE::BUTT:
    default:
      return QString::asprintf("Unknown[%d]: %d", (int)itemType, cnt);
  }
}

void ImgVidOthInFolderPreviewer::subscribe() {
  connect(mTypeToDisplayTB, &ReorderableToolBar::widgetMoved, this, &ImgVidOthInFolderPreviewer::onReorder);
  connect(mTypeToDisplayTB->mCollectPathAgs, &QActionGroup::triggered, this, &ImgVidOthInFolderPreviewer::onImgVidOthActTriggered);
  mVidTv->registerFullScreenToggleCallback(std::bind(&ImgVidOthInFolderPreviewer::onReqFullscreenModeChange, this, std::placeholders::_1));
}

void ImgVidOthInFolderPreviewer::onImgBtnClicked(bool checked) {
  m_bImgVisible = checked;
  if (mImgTv == nullptr) {
    mImgModel = new (std::nothrow) ImgsModel{GetMemoryName() + "/IMAGE", this};
    CHECK_NULLPTR_RETURN_VOID(mImgModel)
    mImgTv = new (std::nothrow) ItemView{GetMemoryName() + "/IMAGE", this};
    CHECK_NULLPTR_RETURN_VOID(mImgTv)
    mImgTv->InitListView();
    mImgTv->SetCurrentModel(mImgModel);
    mImgTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    addWidget(mImgTv);

    connect(mImgTv, &QListView::iconSizeChanged, mImgModel, &QAbstractListModelPub::onIconSizeChange);
  }
  setConfig(BrowserKey::FLOATING_IMAGE_VIEW_SHOW, checked);

  if (mImgTv->isHidden() == checked) {
    mImgTv->setVisible(checked);
  }
}

void ImgVidOthInFolderPreviewer::onVidBtnClicked(bool checked) {
  m_bVidVisible = checked;
  if (mVidTv == nullptr) {
    mVidTv = new (std::nothrow) VideoView{false, this};
    CHECK_NULLPTR_RETURN_VOID(mVidTv)
    mVidTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    addWidget(mVidTv);
  }
  setConfig(BrowserKey::FLOATING_VIDEO_VIEW_SHOW, checked);
  if (mVidTv->isHidden() == checked) {
    mVidTv->setVisible(checked);
  }
}

void ImgVidOthInFolderPreviewer::onOthBtnClicked(bool checked) {
  m_bOthVisible = checked;
  if (mOthTv == nullptr) {
    mOthModel = new (std::nothrow) OthersModel{GetMemoryName() + "/OTHER", this};
    CHECK_NULLPTR_RETURN_VOID(mOthModel)
    mOthTv = new (std::nothrow) ItemView{GetMemoryName() + "/OTHER", this};
    CHECK_NULLPTR_RETURN_VOID(mOthTv);
    mOthTv->InitListView();
    mOthTv->SetCurrentModel(mOthModel);
    mOthTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    addWidget(mOthTv);
  }
  setConfig(BrowserKey::FLOATING_OTHER_VIEW_SHOW, checked);
  if (mOthTv->isHidden() == checked) {
    mOthTv->setVisible(checked);
  }
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
