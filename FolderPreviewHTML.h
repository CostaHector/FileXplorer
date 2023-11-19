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
  FolderPreviewHTML(QWidget* parent = nullptr);
  auto operator()(const QString& path) -> bool;
  void subscribe();
  QSize sizeHint() const override;

  bool onAnchorClicked(const QUrl& url);
  auto ShowAllImages(const int val) -> bool;
  QString InsertImgs(const QString& dirPath);

  int m_firstSightImgCnt = 3;
  QString dirPath;
  QStringList m_imgsLst;
  bool m_scrollAtEndBefore;
  QWidget* m_parent;
  QAction* m_PLAY_ACTION;
  static const QString HTML_IMG_TEMPLATE;
};

#endif  // FOLDERPREVIEWHTML_H
