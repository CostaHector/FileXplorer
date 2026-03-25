#ifndef IMGVIDOTHINFOLDERPREVIEWER_H
#define IMGVIDOTHINFOLDERPREVIEWER_H

#include "ItemView.h"
#include "FloatingModels.h"
#include "ReorderableToolBar.h"
#include "WidgetReorderHelper.h"
#include "VideoView.h"
#include "FullScreenableSplitter.h"

class ImgVidOthInFolderPreviewer: public FullScreenableSplitter {
public:
  explicit ImgVidOthInFolderPreviewer(const QString& memoryName, QWidget* parent = nullptr);
  virtual ~ImgVidOthInFolderPreviewer();

  void operator()(const QString& pth); // file system view
  void operator()(const QString& name, const QString& jsonAbsFilePath, const QStringList& imgPthLst, const QStringList& vidsLst); // scene view

  void onStopPlaying();
  bool UpdateImgs(const QString& name, const QStringList& imgPthLst);
  bool UpdateVids(const QString& rootPath, const QStringList& vidsLst);
  bool UpdateOthers(const QStringList& dataLst);

  inline bool NeedUpdateImgs() const { return m_bImgVisible; }
  inline bool NeedUpdateVids() const { return m_bVidVisible; }
  inline bool NeedUpdateOthers() const { return m_bOthVisible; }

  void subscribe();

private:
  void onImgBtnClicked(bool checked);
  void onVidBtnClicked(bool checked);
  void onOthBtnClicked(bool checked);
  void onTypesBtnClicked(bool checked = true);
  using MediaBtnHandlerFunc = void (ImgVidOthInFolderPreviewer::*)(bool);
  static constexpr MediaBtnHandlerFunc MEDIA_HANDLERS_MAP[(int)PREVIEW_ITEM_TYPE::BUTT]{
      &ImgVidOthInFolderPreviewer::onImgBtnClicked,  //
      &ImgVidOthInFolderPreviewer::onVidBtnClicked,  //
      &ImgVidOthInFolderPreviewer::onOthBtnClicked   //
  };
  void onImgVidOthActTriggered(const QAction* pAct);
  bool onReorder(int fromIndex, int destIndex);
  QString GetActionText(PREVIEW_ITEM_TYPE itemType, int cnt) const;
  QWidget* GetFullScreenableWidget() const override {
    return mVidTv;
  }


  ImgsModel* mImgModel{nullptr};
  OthersModel* mOthModel{nullptr};
  ItemView *mImgTv{nullptr}, *mOthTv{nullptr};
  VideoView *mVidTv{nullptr};
  QAction *_IMG_ACT{nullptr}, *_VID_ACT{nullptr}, *_OTH_ACT{nullptr};
  bool m_bImgVisible{true}, m_bVidVisible{true}, m_bOthVisible{true};
  ReorderableToolBar* mTypeToDisplayTB{nullptr};

  QVector<int> mMediaSequence{
      (int)PREVIEW_ITEM_TYPE::IMG,  //
      (int)PREVIEW_ITEM_TYPE::VID,  //
      (int)PREVIEW_ITEM_TYPE::OTH   //
  };
};

#endif
