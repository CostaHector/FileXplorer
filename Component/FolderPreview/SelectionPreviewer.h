#ifndef SELECTIONPREVIEWER_H
#define SELECTIONPREVIEWER_H

#include <QMap>
#include <QStackedWidget>

#include "ImagesInFolderBrowser.h"
#include "ImagesInFolderSlider.h"
#include "FileFolderPreviewer.h"
#include "PreviewTypeTool.h"

class FolderPreviewSwitcher;
class ToolBarAndViewSwitcher;

class SelectionPreviewer : public QStackedWidget {
  Q_OBJECT
public:
  friend class FolderPreviewSwitcher;
  friend class ToolBarAndViewSwitcher;
  explicit SelectionPreviewer(QWidget* parent = nullptr);
  ~SelectionPreviewer();

  int AddView(PreviewTypeTool::PREVIEW_TYPE_E viewType, QWidget* w) {  //
    int index = addWidget(w);
    m_name2PreviewIndex[viewType] = index;
    m_previewIndex2NameE[index] = viewType;
    return index;
  }

  PreviewTypeTool::PREVIEW_TYPE_E GetCurrentViewE() const { return mCurrentPreviewType; }

  void operator()(const QString& path) {
    m_curPath = path;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }

  bool isTimerDisabled() const { return SelectionPreviewer::NEXT_FOLDER_TIME_INTERVAL <= 0; }

  void UpdatePreview();

  QString GetCurPath() const { return m_curPath; }
  QSize sizeHint() const override;

public slots:
  void setCurrentIndex(int index);

private:
  QMap<PreviewTypeTool::PREVIEW_TYPE_E, int> m_name2PreviewIndex;
  QMap<int, PreviewTypeTool::PREVIEW_TYPE_E> m_previewIndex2NameE;
  QWidget* m_parentDocker{nullptr};

  QString m_curPath;
  ImagesInFolderBrowser* m_imgInFolderBrowser{nullptr};
  ImagesInFolderSlider* m_imgInFolderLabels{nullptr};
  FileFolderPreviewer* m_fileFolderPreviewStackedWid{nullptr};

  QTimer m_nextFolderTimer;

  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 0; // f=26 frame/s T=40ms; 100ms to avoid quick locate by prefix string not work
  // ms, when NEXT_FOLDER_TIME_INTERVAL <= 0. update preview imgs imediately(may cause lag).
  PreviewTypeTool::PREVIEW_TYPE_E mCurrentPreviewType{PreviewTypeTool::PREVIEW_TYPE_E::NONE};
};

#endif  // SELECTIONPREVIEWER_H
