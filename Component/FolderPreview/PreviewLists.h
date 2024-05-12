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
  void subscribe();

  void contextMenuEvent(QContextMenuEvent* event) override;

  void setDockerWindowTitle() {
    if (m_parentDocker == nullptr) {
      return;
    }
    m_parentDocker->setWindowTitle(QString("%1|%2").arg(mImgCnt).arg(mVidsCnt));
    clearCnt();
  }

 protected:
  void clearCnt() { mImgCnt = mVidsCnt = -1; }
  bool isCntOk() { return mImgCnt != -1 and mVidsCnt != -1; }
  QWidget* m_parentDocker;
  FolderListView* m_vidsPreview;
  FolderListView* m_imgsPreview;
  FolderListView* m_othersPreview;

  int mImgCnt = -1;
  int mVidsCnt = -1;

  QMenu* m_folderPreviewMenu;
};

#endif  // PREVIEWLISTS_H
