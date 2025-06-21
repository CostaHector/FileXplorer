#ifndef FLOATINGPREVIEW_H
#define FLOATINGPREVIEW_H

#include "Component/ReorderableToolBar.h"
#include "View/ItemView.h"
#include "Model/FloatingModels.h"
#include "ClickableTextBrowser.h"
#include "Tools/WidgetReorderHelper.h"
#include <QSplitter>
#include <QStackedWidget>
#include <QAction>
#include <QActionGroup>
#include <QToolButton>
#include <QToolBar>
#include <QSqlRecord>
#include <QVector>

class FloatingPreview : public QStackedWidget {
 public:
  enum class PANE_TYPE : int { BEGIN = 0, DETAIL = BEGIN, IMG_VID_OTH, BUTT };

  FloatingPreview(const QString& memoryName, QWidget* parent = nullptr);
  void showEvent(QShowEvent* event) override;

  void ReadSettings();
  void SaveSettings();
  void SaveState();

  void operator()(const QSqlRecord& record, const QString& imgHost, const int imgHeight);  // cast
  void operator()(const QString& pth);                                                     // file system
  void operator()(const QString& name, const QString& pth);                                // scene
  void UpdateImgs(const QString& name, const QStringList& imgPthLst);                      // scene
  void UpdateVids(const QStringList& dataLst);
  void UpdateOthers(const QStringList& dataLst);

  inline bool NeedUpdate(const QString& lastName) const { return !lastName.isEmpty() && (mLastName.isEmpty() || mLastName != lastName); }
  inline bool NeedUpdateDetail() const { return m_curIndex == PANE_TYPE::DETAIL; }
  inline bool NeedUpdateImgs() const { return m_curIndex == PANE_TYPE::IMG_VID_OTH && m_bImgVisible; }
  inline bool NeedUpdateVids() const { return m_curIndex == PANE_TYPE::IMG_VID_OTH && m_bVidVisible; }
  inline bool NeedUpdateOthers() const { return m_curIndex == PANE_TYPE::IMG_VID_OTH && m_bOthVisible; }
  inline void BeforeDisplayAFileDetail() { setCurrentIndex((int)PANE_TYPE::DETAIL); }
  inline void BeforeDisplayAFolder() { setCurrentIndex((int)PANE_TYPE::IMG_VID_OTH); }
  void setCurrentIndex(int index) {
    if (index < (int)PANE_TYPE::BEGIN || index >= (int)PANE_TYPE::BUTT) {
      qWarning("Current index[%d] out of bound[%d, %d)", index, (int)PANE_TYPE::BEGIN, (int)PANE_TYPE::BUTT);
      return;
    }
    m_curIndex = static_cast<PANE_TYPE>(index);
    QStackedWidget::setCurrentIndex(index);
  }
  void subscribe();

 private:
  void onImgBtnClicked(bool checked);
  void onVidBtnClicked(bool checked);
  void onOthBtnClicked(bool checked);
  void onTypesBtnClicked(bool checked = true);
  using MediaBtnHandlerFunc = void (FloatingPreview::*)(bool);
  static constexpr MediaBtnHandlerFunc MEDIA_HANDLERS_MAP[(int)PREVIEW_ITEM_TYPE::BUTT]{
      &FloatingPreview::onImgBtnClicked,  //
      &FloatingPreview::onVidBtnClicked,  //
      &FloatingPreview::onOthBtnClicked   //
  };
  void onImgVidOthActTriggered(const QAction* pAct);

  bool onReorder(int fromIndex, int destIndex);

  ClickableTextBrowser* mDetailsPane{nullptr};

  ImgsModel* mImgModel{nullptr};
  VidsModel* mVidsModel{nullptr};
  OthersModel* mOthModel{nullptr};
  ItemView *mImgTv{nullptr}, *mVidTv{nullptr}, *mOthTv{nullptr};
  QAction *_IMG_ACT{nullptr}, *_VID_ACT{nullptr}, *_OTH_ACT{nullptr};
  bool m_bImgVisible{true}, m_bVidVisible{true}, m_bOthVisible{true};
  ReorderableToolBar* mTypeToDisplayTB{nullptr};
  PANE_TYPE m_curIndex{PANE_TYPE::DETAIL};
  QSplitter* mImgVidOtherSplitter{nullptr};

  QString mLastName;
  const QString mMemoryName;
  QVector<int> mMediaSequence{
      (int)PREVIEW_ITEM_TYPE::IMG,  //
      (int)PREVIEW_ITEM_TYPE::VID,  //
      (int)PREVIEW_ITEM_TYPE::OTH   //
  };
};

#endif  // FLOATINGPREVIEW_H
