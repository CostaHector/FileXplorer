#ifndef IMGVIDOTHINFOLDERPREVIEWER_H
#define IMGVIDOTHINFOLDERPREVIEWER_H

#include "ItemView.h"
#include "FloatingModels.h"
#include "ReorderableToolBar.h"
#include "WidgetReorderHelper.h"
#include <QVBoxLayout>

class ImgVidOthInFolderPreviewer: public QWidget {
public:
  explicit ImgVidOthInFolderPreviewer(const QString& memoryName, QWidget* parent = nullptr);
  ~ImgVidOthInFolderPreviewer();
  void operator()(const QString& pth); // file system view
  void operator()(const QString& name, const QString& jsonAbsFilePath, const QStringList& imgPthLst, const QStringList& vidsLst); // scene view

  void UpdateImgs(const QString& name, const QStringList& imgPthLst);
  void UpdateVids(const QStringList& vidsLst);
  void UpdateOthers(const QStringList& dataLst);

  inline bool NeedUpdateImgs() const { return m_bImgVisible; }
  inline bool NeedUpdateVids() const { return m_bVidVisible; }
  inline bool NeedUpdateOthers() const { return m_bOthVisible; }

  void SaveState();
  void subscribe();

  void AdjustButtonPosition() {
    if (mTypeToDisplayTB == nullptr) {return;}
    static constexpr int marginX = 16, marginY = 32;
    mTypeToDisplayTB->move(width() - mTypeToDisplayTB->width() - marginX, height() - mTypeToDisplayTB->height() - marginY);
    mTypeToDisplayTB->raise();
  }
protected:
  void resizeEvent(QResizeEvent *event) override {
    QWidget::resizeEvent(event);
    AdjustButtonPosition();
  }

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

  ImgsModel* mImgModel{nullptr};
  VidsModel* mVidsModel{nullptr};
  OthersModel* mOthModel{nullptr};
  ItemView *mImgTv{nullptr}, *mVidTv{nullptr}, *mOthTv{nullptr};
  QAction *_IMG_ACT{nullptr}, *_VID_ACT{nullptr}, *_OTH_ACT{nullptr};
  bool m_bImgVisible{true}, m_bVidVisible{true}, m_bOthVisible{true};
  ReorderableToolBar* mTypeToDisplayTB{nullptr};

  QSplitter* mImgVidOtherSplitter{nullptr};
  QVBoxLayout* mLo{nullptr};

  const QString mMemoryName;
  QVector<int> mMediaSequence{
      (int)PREVIEW_ITEM_TYPE::IMG,  //
      (int)PREVIEW_ITEM_TYPE::VID,  //
      (int)PREVIEW_ITEM_TYPE::OTH   //
  };
};

#endif
