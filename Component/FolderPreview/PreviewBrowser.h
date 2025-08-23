#ifndef PREVIEWBROWSER_H
#define PREVIEWBROWSER_H
#include "ClickableTextBrowser.h"

class PreviewBrowser : public ClickableTextBrowser {
 public:
  explicit PreviewBrowser(QWidget* parent = nullptr);
  bool operator()(const QString& path);
  void subscribe();

  void setDockerWindowTitle(int vidCnt) {
    if (m_parentDocker == nullptr) {
      return;
    }
    m_parentDocker->setWindowTitle(QString{"%1|%2"}.arg(vidCnt).arg(m_imgsLst.size()));
  }

  QStringList InitImgsList(const QString& dirPath) const;
  bool hasNextImgs() const;
  QString nextImgsHTMLSrc();
  bool ShowRemainImages(const int val);

  static constexpr int SHOW_IMGS_CNT_LIST[] = {0, 3, 10, 50, INT_MAX};  // never remove last element "INT_MAX"
  static constexpr int N_SHOW_IMGS_CNT_LIST = sizeof(SHOW_IMGS_CNT_LIST) / sizeof(SHOW_IMGS_CNT_LIST[0]);
  int m_curImgCntIndex = 0;

  QString dirPath;
  QStringList m_imgsLst;
  QWidget* m_parentDocker{nullptr};
};

#endif  // PREVIEWBROWSER_H
