#ifndef FLOATINGPREVIEW_H
#define FLOATINGPREVIEW_H

#include "Model/FloatingModels.h"
#include "View/ItemView.h"
#include "Tools/SplitterInsertIndexHelper.h"
#include "ClickableTextBrowser.h"
#include <QSplitter>
#include <QPushButton>
#include <QAction>
#include <QToolBar>
#include <QSqlRecord>

class FloatingPreview : public QSplitter {
 public:
  enum PANE_TYPE { DETAIL, TYPE_DISPLAY_CONTROL, IMAGE, VIDEO, OTHER, BUTT };

  FloatingPreview(QWidget* parent = nullptr);
  void ReadSettings();
  void SaveSettings();
  void SaveState();

  void operator()(const QSqlRecord& record, const QString& imgHost, const int imgHeight);  // cast
  void operator()(const QString& pth);                                                     // file system
  void operator()(const QString& name, const QString& pth);                                // scene
  void UpdateImgs(const QString& name, const QStringList& imgPthLst);                      // scene
  void UpdateVids(const QStringList& dataLst);
  void UpdateOthers(const QStringList& dataLst);

  bool NeedUpdate(const QString& lastName) const { return !lastName.isEmpty() && (mLastName.isEmpty() || mLastName != lastName); }
  bool NeedUpdateDetail() const { return mDetailsPane != nullptr; }
  bool NeedUpdateImgs() const { return mImgTv != nullptr; }
  bool NeedUpdateVids() const { return mVidTv != nullptr; }
  bool NeedUpdateOthers() const { return mOthTv != nullptr; }
  void BeforeDisplayAFileDetail() {
    if (NeedUpdateDetail() && !mDetailsPane->isVisible()) {
      mDetailsPane->setVisible(true);
    }
    if (mTypeToDisplayTB != nullptr && mTypeToDisplayTB->isVisible()) {
      mTypeToDisplayTB->setVisible(false);
    }
    if (_IMG_ENABLED->isChecked() && NeedUpdateImgs() && mImgTv->isVisible()) {
      mImgTv->setVisible(false);
    }
    if (_VID_ENABLED->isChecked() && NeedUpdateVids() && mVidTv->isVisible()) {
      mVidTv->setVisible(false);
    }
    if (_OTH_ENABLED->isChecked() && NeedUpdateOthers() && mOthTv->isVisible()) {
      mOthTv->setVisible(false);
    }
  }
  void BeforeDisplayAFolder() {
    if (NeedUpdateDetail() && mDetailsPane->isVisible()) {
      mDetailsPane->setVisible(false);
    }
    if (mTypeToDisplayTB != nullptr && !mTypeToDisplayTB->isVisible()) {
      mTypeToDisplayTB->setVisible(true);
    }
    if (_IMG_ENABLED->isChecked() && NeedUpdateImgs() && !mImgTv->isVisible()) {
      mImgTv->setVisible(true);
    }
    if (_VID_ENABLED->isChecked() && NeedUpdateVids() && !mVidTv->isVisible()) {
      mVidTv->setVisible(true);
    }
    if (_OTH_ENABLED->isChecked() && NeedUpdateOthers() && !mOthTv->isVisible()) {
      mOthTv->setVisible(true);
    }
  }

  void subscribe();

 private:
  void onImgBtnClicked(bool checked);
  void onVidBtnClicked(bool checked);
  void onOthBtnClicked(bool checked);

  QAction *_IMG_ENABLED{nullptr}, *_VID_ENABLED{nullptr}, *_OTH_ENABLED{nullptr};
  QToolBar* mTypeToDisplayTB{nullptr};

  ClickableTextBrowser* mDetailsPane{nullptr};
  ImgsModel* mImgModel{nullptr};
  VidsModel* mVidsModel{nullptr};
  OthersModel* mOthModel{nullptr};
  ItemView *mImgTv{nullptr}, *mVidTv{nullptr}, *mOthTv{nullptr};

  QString mLastName;

  SplitterInsertIndexHelper m_insertIndex{BUTT};
};

#endif  // FLOATINGPREVIEW_H
