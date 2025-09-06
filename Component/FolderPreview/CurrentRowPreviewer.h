#ifndef CURRENTROWPREVIEWER_H
#define CURRENTROWPREVIEWER_H

#include <QStackedWidget>
#include <QSqlRecord>
#include <QMap>

#include "ImagesInFolderBrowser.h"
#include "ImagesInFolderSlider.h"
#include "FileFolderPreviewer.h"
#include "PreviewTypeTool.h"

class FolderPreviewSwitcher;
class ViewSwitchHelper;

class CurrentRowPreviewer : public QStackedWidget {
  Q_OBJECT
public:
  friend class FolderPreviewSwitcher;
  friend class ViewSwitchHelper;
  explicit CurrentRowPreviewer(QWidget* parent = nullptr);
  ~CurrentRowPreviewer();

  int AddView(PreviewTypeTool::PREVIEW_TYPE_E viewType, QWidget* w) {  //
    int index = addWidget(w);
    m_name2PreviewIndex[viewType] = index;
    m_previewIndex2NameE[index] = viewType;
    return index;
  }

  PreviewTypeTool::PREVIEW_TYPE_E GetCurrentViewE() const { return mCurrentPreviewType; }

  void operator()(const QString& path) {
    mCurrentSrcFrom = SRC_FROM::FILE_SYSTEM_VIEW;
    m_curPath = path;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }

  void operator()(const QSqlRecord& newRecord, const QString imageHostPath) {
    mCurrentSrcFrom = SRC_FROM::CAST;
    m_curRecord = newRecord;
    m_curImageHostPath = imageHostPath;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }

  void operator()(const QString& name, const QStringList& imgPthLst, const QStringList& vidsLst) {
    mCurrentSrcFrom = SRC_FROM::SCENE;
    m_sceneName = name;
    m_sceneimgPthLst = imgPthLst;
    m_scenevidsLst = vidsLst;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }


  bool isTimerDisabled() const { return CurrentRowPreviewer::NEXT_FOLDER_TIME_INTERVAL <= 0; }

  void UpdatePreview();

  QString GetCurPath() const { return m_curPath; }
  QSize sizeHint() const override;

public slots:
  void setCurrentIndex(int index);

private:
  QMap<PreviewTypeTool::PREVIEW_TYPE_E, int> m_name2PreviewIndex;
  QMap<int, PreviewTypeTool::PREVIEW_TYPE_E> m_previewIndex2NameE;
  QWidget* m_parentDocker{nullptr};

  /*For File System View*/
  QString m_curPath;
  /*For Cast View*/
  QSqlRecord m_curRecord;
  QString m_curImageHostPath;
  /*For Scene View*/
  QString m_sceneName;
  QStringList m_sceneimgPthLst;
  QStringList m_scenevidsLst;

  ImagesInFolderBrowser* m_imgInFolderBrowser{nullptr};
  ImagesInFolderSlider* m_imgInFolderLabels{nullptr};
  FileFolderPreviewer* m_fileFolderPreviewStackedWid{nullptr};

  QTimer m_nextFolderTimer;

  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 150; // f=26 frame/s T=40ms; 100ms to avoid quick locate by prefix string not work
  // ms, when NEXT_FOLDER_TIME_INTERVAL <= 0. update preview imgs imediately(may cause lag).
  PreviewTypeTool::PREVIEW_TYPE_E mCurrentPreviewType{PreviewTypeTool::PREVIEW_TYPE_E::NONE};

  enum class SRC_FROM {
    FILE_SYSTEM_VIEW,
    SCENE,
    CAST,
  };
  SRC_FROM mCurrentSrcFrom{SRC_FROM::FILE_SYSTEM_VIEW};
};

#endif  // CURRENTROWPREVIEWER_H
