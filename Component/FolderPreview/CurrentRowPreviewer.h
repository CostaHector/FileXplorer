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

  PreviewTypeTool::PREVIEW_TYPE_E GetCurrentViewE() const { return mCurrentPreviewType; }

  void DisplayFileSystemRecord(const QString& path) {
    mCurrentSrcFrom = SRC_FROM::FILE_SYSTEM_VIEW;
    m_curPath = path;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }

  void DisplayCastRecord(const QSqlRecord& newRecord, const QString imageHostPath) {
    mCurrentSrcFrom = SRC_FROM::CAST_DB;
    m_curRecord = newRecord;
    m_curImageHostPath = imageHostPath;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }

  void DisplayMovieRecord(const QSqlRecord& newRecord) {
    mCurrentSrcFrom = SRC_FROM::MOVIE_DB;
    m_curRecord = newRecord;
    if (isTimerDisabled()) {
      UpdatePreview();
      return;
    }
    m_nextFolderTimer.stop();
    m_nextFolderTimer.start();
  }

  void DisplayJsonRecord(const QString& name, const QString& jsonAbsPath, const QStringList& imgPthLst, const QStringList& vidsLst) {
    mCurrentSrcFrom = SRC_FROM::JSON_OR_SCENE;
    m_sceneName = name;
    m_sceneJsonAbsFilePath = jsonAbsPath;
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
  void saveSizeHint() const;

  bool NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E previewType) const;

public slots:
  bool InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E previewType);
  bool setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E previewType);

private:
  int AddView(PreviewTypeTool::PREVIEW_TYPE_E viewType, QWidget* w) {  //
    int index = addWidget(w);
    m_name2PreviewIndex[viewType] = index;
    m_previewIndex2NameE[index] = viewType;
    return index;
  }

  QMap<PreviewTypeTool::PREVIEW_TYPE_E, int> m_name2PreviewIndex;
  QMap<int, PreviewTypeTool::PREVIEW_TYPE_E> m_previewIndex2NameE;
  QWidget* m_parentDocker{nullptr};

  /*For File System View*/
  QString m_curPath;
  /*For Cast/MovieDB View*/
  QSqlRecord m_curRecord;
  /*For Cast View*/
  QString m_curImageHostPath;
  /*For Scene View*/
  QString m_sceneName;
  QString m_sceneJsonAbsFilePath;
  QStringList m_sceneimgPthLst;
  QStringList m_scenevidsLst;

  ImagesInFolderBrowser* m_imgInFolderBrowser{nullptr};
  ImagesInFolderSlider* m_imgInFolderLabels{nullptr};
  FileFolderPreviewer* m_fileFolderPreviewStackedWid{nullptr};

  QTimer m_nextFolderTimer;

#ifdef RUNNING_UNIT_TESTS
  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 0; // in testcase: async is not a good method
#else
  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 150; // f=26 frame/s T=40ms; 100ms to avoid quick locate by prefix string not work
#endif
  // ms, when NEXT_FOLDER_TIME_INTERVAL <= 0. update preview imgs imediately(may cause lag).
  PreviewTypeTool::PREVIEW_TYPE_E mCurrentPreviewType{PreviewTypeTool::PREVIEW_TYPE_E::NONE};

  enum class SRC_FROM {
    FILE_SYSTEM_VIEW,
    JSON_OR_SCENE, // scene or json
    CAST_DB,
    MOVIE_DB,
  };
  SRC_FROM mCurrentSrcFrom{SRC_FROM::FILE_SYSTEM_VIEW};
};

#endif  // CURRENTROWPREVIEWER_H
