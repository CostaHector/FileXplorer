#ifndef PERFORMERSPREVIEWTEXTBROWSER_H
#define PERFORMERSPREVIEWTEXTBROWSER_H

#include "FolderPreview/ClickableTextBrowser.h"
#include <QSqlRecord>

class PerformersPreviewTextBrowser : public ClickableTextBrowser {
 public:
  explicit PerformersPreviewTextBrowser(QWidget* parent = nullptr);
  auto operator()(const QSqlRecord& record, const QString& m_imageHostPath, const int m_performerImageHeight = 200) -> bool;
  void subscribe();
  QSize sizeHint() const override;
  auto keyPressEvent(QKeyEvent* e) -> void override;

  QStringList InitImgsList(const QString& dirPath) const;
  bool hasNextImgs() const;
  QString nextImgsHTMLSrc();
  auto ShowRemainImages(const int val) -> bool;
  auto onVerticalScrollBarAction(int action) -> bool;

  static constexpr int SHOW_IMGS_CNT_LIST[] = {0, 1, 10, 50, INT_MAX};  // never remove last element "INT_MAX"
  static constexpr int N_SHOW_IMGS_CNT_LIST = sizeof(SHOW_IMGS_CNT_LIST) / sizeof(SHOW_IMGS_CNT_LIST[0]);
  int m_curImgCntIndex = 0;

  QString dirPath;
  QStringList m_imgsLst;

  static QString PERFORMER_HTML_TEMPLATE;
  static const QRegularExpression IMG_VID_SEP_COMP;
};

#endif  // PERFORMERSPREVIEWTEXTBROWSER_H
