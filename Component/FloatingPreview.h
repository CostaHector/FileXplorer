#ifndef FLOATINGPREVIEW_H
#define FLOATINGPREVIEW_H

#include <QSplitter>
#include <QPushButton>
#include "View/CustomListView.h"
#include "Model/FloatingModels.h"
#include <QAction>
#include <QToolBar>

class ItemView : public CustomListView {
 public:
  explicit ItemView(const QString& itemViewName, QWidget* parent = nullptr);
  void SetCurrentModel(FloatingModels* mdl) {
    setModel(mdl);
    mModels = mdl;
  }
  void subscribe();
  void onCellDoubleClicked(const QModelIndex& clickedIndex) const;
  void onOrientationChange(const QAction* pOrientation);

 private:
  FloatingModels* mModels{nullptr};
  QAction* _PLAY_ITEM{nullptr};
  QAction *_ORIENTATION_LEFT_TO_RIGHT{nullptr}, *_ORIENTATION_TOP_TO_BOTTOM{nullptr};
  QActionGroup* _ORIENTATION_GRP{nullptr};
  QMenu* mItemMenu{nullptr};
};

class FloatingPreview : public QSplitter {
 public:
  FloatingPreview(QWidget* parent = nullptr);
  void ReadSettings();
  void SaveSettings();
  static QPushButton* CreateBtn(const QString& tag, QWidget* parent);

  void operator()(const QString& pth);                                 // file system
  void operator()(const QString& name, const QString& pth);            // scene
  void UpdateImgs(const QString& name, const QStringList& imgPthLst);  // scene
  void UpdateVids(const QStringList& dataLst);
  void UpdateOthers(const QStringList& dataLst);

  bool NeedUpdate(const QString& lastName) const { return mLastName.isEmpty() || mLastName != lastName; }
  bool NeedUpdateImgs() const { return mImgTv != nullptr; }
  bool NeedUpdateVids() const { return mVidTv != nullptr; }
  bool NeedUpdateOthers() const { return mOthTv != nullptr; }

  void subscribe();

 private:
  void onImgBtnClicked(bool checked);
  void onVidBtnClicked(bool checked);
  void onOthBtnClicked(bool checked);

  QAction* mImgBtn{nullptr}, *mVidsBtn{nullptr}, *mOthersBtn{nullptr};
  QToolBar* mTypeToDisplayTB{nullptr};
  ImgsModel* mImgModel{nullptr};
  VidsModel* mVidsModel{nullptr};
  OthersModel* mOthModel{nullptr};
  ItemView *mImgTv{nullptr}, *mVidTv{nullptr}, *mOthTv{nullptr};

  QString mLastName;
};

#endif  // FLOATINGPREVIEW_H
