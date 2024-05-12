#ifndef PREVIEWLISTS_H
#define PREVIEWLISTS_H

#include <QString>
#include <QWidget>
#include "FolderPreviewComponent/FolderListView.h"

class PreviewLists : public QWidget {
  Q_OBJECT
 public:
  explicit PreviewLists(QWidget* parent = nullptr);
  bool operator()(const QString& path);
  void subscribe() {}

  void setDockerWindowTitle(int vidCnt, int imgCnt) {
    if (m_parentDocker == nullptr) {
      return;
    }
    m_parentDocker->setWindowTitle(QString::number(vidCnt) + '|' + QString::number(imgCnt));
  }

  void contextMenuEvent(QContextMenuEvent* event) override;
 signals:
  void showANewPath(const QString&);

 protected:
  QWidget* m_parentDocker;
  FolderListView* m_vidsPreview;
  FolderListView* m_imgsPreview;
  FolderListView* m_othersPreview;

  QMenu* m_folderPreviewMenu;
};

#endif  // PREVIEWLISTS_H
