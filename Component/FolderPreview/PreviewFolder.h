#ifndef PREVIEWFOLDER_H
#define PREVIEWFOLDER_H

#include <QHash>
#include <QStackedWidget>

#include "PreviewBrowser.h"
#include "PreviewLabels.h"
#include "FloatingPreview.h"

class FolderPreviewSwitcher;
class NavigationViewSwitcher;

class PreviewFolder : public QStackedWidget {
 public:
  friend class FolderPreviewSwitcher;
  friend class NavigationViewSwitcher;
  explicit PreviewFolder(QWidget* parent = nullptr);

  QString GetCurViewName() const {
    const int curInd = currentIndex();
    for (auto it = m_name2PreviewIndex.cbegin(); it != m_name2PreviewIndex.cend(); ++it) {
      if (it.value() == curInd) {
        return it.key();
      }
    }
    qWarning("no view name find for index[%d]", curInd);
    return "";
  }
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

  inline bool isTimerDisabled() const { return PreviewFolder::NEXT_FOLDER_TIME_INTERVAL <= 0; }

  void UpdatePreview();

  QString GetCurPath() const { return m_curPath; }

  QSize sizeHint() const override;

 private:
  QHash<QString, int> m_name2PreviewIndex;
  QWidget* m_parentDocker;

  QString m_curPath;
  PreviewBrowser* m_browser{nullptr};
  PreviewLabels* m_labels{nullptr};
  FloatingPreview* m_lists{nullptr};

  QTimer* m_nextFolderTimer = new QTimer{this};

  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 0.1 * 1000;
  // ms, when NEXT_FOLDER_TIME_INTERVAL <= 0. update preview imgs imediately(may cause lag).
};

#endif  // PREVIEWFOLDER_H
