#ifndef PREVIEWBROWSER_H
#define PREVIEWBROWSER_H
#include "ClickableTextBrowser.h"

class PreviewBrowser : public ClickableTextBrowser {
 public:
  explicit PreviewBrowser(QWidget* parent = nullptr);
  auto operator()(const QString& path) -> bool;
  void subscribe();

  void setDockerWindowTitle(int vidCnt) {
    if (m_parentDocker == nullptr) {
      return;
    }
    m_parentDocker->setWindowTitle(QString::number(vidCnt) + '|' + QString::number(m_imgsLst.size()));
  }

  QStringList InitImgsList(const QString& dirPath) const;
  bool hasNextImgs() const;
  QString nextImgsHTMLSrc();
  auto ShowRemainImages(const int val) -> bool;

  static constexpr int SHOW_IMGS_CNT_LIST[] = {0, 3, 10, 50, INT_MAX};  // never remove last element "INT_MAX"
  static constexpr int N_SHOW_IMGS_CNT_LIST = sizeof(SHOW_IMGS_CNT_LIST) / sizeof(SHOW_IMGS_CNT_LIST[0]);
  int m_curImgCntIndex = 0;

  QString dirPath;
  QStringList m_imgsLst;
  QWidget* m_parentDocker;
};

#endif  // PREVIEWBROWSER_H
