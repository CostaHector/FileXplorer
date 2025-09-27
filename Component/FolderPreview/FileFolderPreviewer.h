#ifndef FILEFOLDERPREVIEWER_H
#define FILEFOLDERPREVIEWER_H

#include "ImgVidOthInFolderPreviewer.h"
#include "ClickableTextBrowser.h"
#include <QStackedWidget>
#include <QSqlRecord>

class FileFolderPreviewer : public QStackedWidget {
public:
  enum class PANE_TYPE : int { BEGIN = 0, DETAIL = BEGIN, IMG_VID_OTH, BUTT };
  explicit FileFolderPreviewer(const QString& memoryName, QWidget* parent = nullptr);
  ~FileFolderPreviewer();

  void ReadSettings();
  void SaveSettings();

  void operator()(const QSqlRecord& record, const QString& imgHost);  // cast view
  void operator()(const QString& pth);                                // file system view
  void operator()(const QString& name, const QStringList& imgPthLst, const QStringList& vidsLst);           // scene view

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
      LOG_W("Current index[%d] out of bound[%d, %d)", index, (int)PANE_TYPE::BEGIN, (int)PANE_TYPE::BUTT);
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
  ImgVidOthInFolderPreviewer* mImgVidOtherPane{nullptr};
};

#endif  // FILEFOLDERPREVIEWER_H
