#ifndef IMAGESINFOLDERBROWSER_H
#define IMAGESINFOLDERBROWSER_H
#include "ClickableTextBrowser.h"

class ImagesInFolderBrowser : public ClickableTextBrowser {
 public:
  explicit ImagesInFolderBrowser(QWidget* parent = nullptr);
  void wheelEvent(QWheelEvent *event) override;

  bool operator()(const QString& path);
  void subscribe();

  QStringList InitImgsList(const QString& dirPath) const;
  bool hasNextImgs() const;
  QString nextImgsHTMLSrc();
  bool ShowRemainImages(const int val);

private:
  static constexpr int SHOW_IMGS_CNT_LIST[] = {0, 3, 10, 20, 30, 40, 50, 70, 100, 255, INT_MAX};  // never remove last element "INT_MAX"
  static constexpr int N_SHOW_IMGS_CNT_LIST = sizeof(SHOW_IMGS_CNT_LIST) / sizeof(*SHOW_IMGS_CNT_LIST);
  int m_curImgCntIndex = 0;
  QString m_dirPath;
  QStringList m_imgsLst;
  QWidget* m_parentDocker{nullptr};
};

#endif  // IMAGESINFOLDERBROWSER_H
