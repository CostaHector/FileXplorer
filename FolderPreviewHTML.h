#ifndef FOLDERPREVIEWHTML_H
#define FOLDERPREVIEWHTML_H
#include <QDesktopServices>
#include <QDir>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>

class FolderPreviewHTML : public QTextBrowser {
 public:
  explicit FolderPreviewHTML(QWidget* parent = nullptr);
  auto operator()(const QString& path) -> bool;
  void subscribe();
  QSize sizeHint() const override;

  bool onAnchorClicked(const QUrl& url);

  QStringList InitImgsList(const QString& dirPath) const;
  bool hasNextImgs() const;
  QString nextImgsHTMLSrc();
  auto ShowRemainImages(const int val) -> bool;

  static constexpr int SHOW_IMGS_CNT_LIST[] = {0, 3, 10, 50, INT_MAX}; // never remove last element "INT_MAX"
  static constexpr int N_SHOW_IMGS_CNT_LIST = sizeof(SHOW_IMGS_CNT_LIST) / sizeof(SHOW_IMGS_CNT_LIST[0]);
  int m_curImgCntIndex = 0;

  QString dirPath;
  QStringList m_imgsLst;
  QWidget* m_parent;
  QAction* m_PLAY_ACTION;
  static const QString HTML_H1_TEMPLATE;
  static const QString HTML_H1_WITH_VIDS_TEMPLATE;
  static const QString HTML_IMG_TEMPLATE;
  static constexpr int HTML_IMG_FIXED_WIDTH = 600;
};

#endif  // FOLDERPREVIEWHTML_H
