#ifndef SELECTIONPREVIEWER_H
#define SELECTIONPREVIEWER_H

#include <QHash>
#include <QStackedWidget>

#include "ImagesInFolderBrowser.h"
#include "ImagesInFolderSlider.h"
#include "FileFolderPreviewer.h"

class FolderPreviewSwitcher;
class ToolBarAndViewSwitcher;

class SelectionPreviewer : public QStackedWidget {
 public:
  friend class FolderPreviewSwitcher;
  friend class ToolBarAndViewSwitcher;
  explicit SelectionPreviewer(QWidget* parent = nullptr);

  int AddView(const QString& viewType, QWidget* w) {  //
    return m_name2PreviewIndex[viewType] = addWidget(w);
  }

  void operator()(const QString& path) {
    m_curPath = path;
    if (isTimerDisabled()) {
      UpdatePreview();
    }
    m_nextFolderTimer->stop();
    m_nextFolderTimer->start();
  }

  inline bool isTimerDisabled() const { return SelectionPreviewer::NEXT_FOLDER_TIME_INTERVAL <= 0; }

  void UpdatePreview();

  QString GetCurPath() const { return m_curPath; }
  QSize sizeHint() const override;

 private:
  QHash<QString, int> m_name2PreviewIndex;
  QWidget* m_parentDocker;

  QString m_curPath;
  ImagesInFolderBrowser* m_imgInFolderBrowser{nullptr};
  ImagesInFolderSlider* m_imgInFolderLabels{nullptr};
  FileFolderPreviewer* m_fileFolderPreviewStackedWid{nullptr};

  QTimer* m_nextFolderTimer = new QTimer{this};

  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 0.1 * 1000;
  // ms, when NEXT_FOLDER_TIME_INTERVAL <= 0. update preview imgs imediately(may cause lag).
};

#endif  // SELECTIONPREVIEWER_H
