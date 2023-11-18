#ifndef FOLDERPREVIEWHTML_H
#define FOLDERPREVIEWHTML_H
#include <QDesktopServices>
#include <QDir>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTextCursor>
#include <QTextEdit>
#include "Actions/FileBasicOperationsActions.h"
class FolderPreviewHTML : public QTextBrowser {
 public:
  FolderPreviewHTML(QWidget* parent = nullptr)
      : m_scrollAtEndBefore(false), m_parent(parent), m_PLAY_ACTION(g_fileBasicOperationsActions().OPEN->actions()[0]) {
    setReadOnly(true);
    setOpenLinks(false);
    setOpenExternalLinks(true);

    connect(this->verticalScrollBar(), &QScrollBar::valueChanged, this, &FolderPreviewHTML::ShowAllImages);
    connect(this, &QTextBrowser::anchorClicked, this, &FolderPreviewHTML::onAnchorClicked);
  };

  bool onAnchorClicked(const QUrl& url) {
    if (not url.isLocalFile()) {
      return false;
    }
    QFileInfo fi(url.toLocalFile());
    if (TYPE_FILTER::VIDEO_TYPE_SET.contains("*." + fi.suffix()) or fi.isDir()) {
      if (m_PLAY_ACTION) {
        emit m_PLAY_ACTION->triggered(false);
      }
      return true;
    }
    QDesktopServices::openUrl(url);
    return true;
  }

  auto operator()(const QString& path) -> bool;
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
