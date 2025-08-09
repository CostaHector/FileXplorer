#ifndef FLOATINGPREVIEW_H
#define FLOATINGPREVIEW_H

#include "ImgVidOthWid.h"
#include "ClickableTextBrowser.h"
#include <QStackedWidget>
#include <QSqlRecord>

class FloatingPreview : public QStackedWidget {
public:
  enum class PANE_TYPE : int { BEGIN = 0, DETAIL = BEGIN, IMG_VID_OTH, BUTT };
  FloatingPreview(const QString& memoryName, QWidget* parent = nullptr);

  void ReadSettings();
  void SaveSettings();

  void operator()(const QSqlRecord& record, const QString& imgHost, const int imgHeight);  // cast view
  void operator()(const QString& pth);                                                     // file system view
  void operator()(const QString& name, const QString& pth);                                // scene view

  inline void BeforeDisplayAFileDetail() {
    if (m_curIndex != PANE_TYPE::DETAIL) {
      setCurrentIndex((int)PANE_TYPE::DETAIL);
    }
  }
  inline void BeforeDisplayAFolder() {
    if (m_curIndex != PANE_TYPE::IMG_VID_OTH) {
      setCurrentIndex((int)PANE_TYPE::IMG_VID_OTH);
    }
  }
  void setCurrentIndex(int index) {
    if (index < (int)PANE_TYPE::BEGIN || index >= (int)PANE_TYPE::BUTT) {
      qWarning("Current index[%d] out of bound[%d, %d)", index, (int)PANE_TYPE::BEGIN, (int)PANE_TYPE::BUTT);
      return;
    }
    m_curIndex = static_cast<PANE_TYPE>(index);
    QStackedWidget::setCurrentIndex(index);
  }
  inline bool NeedUpdate(const QString& lastName) const { return !lastName.isEmpty() && (mLastName.isEmpty() || mLastName != lastName); }

  // quick access
  void UpdateImgs(const QString& name, const QStringList& imgPthLst) {
    mLastName = name;
    setWindowTitle(mLastName);
    mImgVidOtherPane->UpdateImgs(name, imgPthLst);
  }
  void UpdateVids(const QStringList& dataLst) {
    mImgVidOtherPane->UpdateVids(dataLst);
  }
  void UpdateOthers(const QStringList& dataLst) {
    mImgVidOtherPane->UpdateOthers(dataLst);
  }

private:
  QString mLastName;
  PANE_TYPE m_curIndex{PANE_TYPE::DETAIL};

  ClickableTextBrowser* mDetailsPane{nullptr};
  ImgVidOthWid* mImgVidOtherPane{nullptr};
};

#endif  // FLOATINGPREVIEW_H
