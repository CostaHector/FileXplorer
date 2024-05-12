#ifndef PREVIEWFOLDER_H
#define PREVIEWFOLDER_H

#include <QHash>
#include <QStackedWidget>

#include "PreviewBrowser.h"
#include "PreviewLabels.h"
#include "PreviewLists.h"

class FolderPreviewSwitcher;

class PreviewFolder : public QStackedWidget {
 public:
  friend class FolderPreviewSwitcher;
  explicit PreviewFolder(QWidget* parent = nullptr) : QStackedWidget{parent}, m_parentDocker{parent} {
    if (isTimerDisabled()) {
      return;
    }
    m_nextFolderTimer->setInterval(PreviewFolder::NEXT_FOLDER_TIME_INTERVAL);
    m_nextFolderTimer->setSingleShot(true);
    connect(m_nextFolderTimer, &QTimer::timeout, this, &PreviewFolder::display);
  }

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
  int AddView(const QString& viewType, QWidget* w) { return m_name2PreviewIndex[viewType] = addWidget(w); }

  void operator()(const QString& path) {
    m_curPath = path;
    if (isTimerDisabled()) {
      display();
    }
    m_nextFolderTimer->stop();
    m_nextFolderTimer->start();
  }

  inline bool isTimerDisabled() const { return PreviewFolder::NEXT_FOLDER_TIME_INTERVAL <= 0; }

  void display() {
    auto* curPreview = currentWidget();
    if (curPreview == nullptr) {
      qWarning("skip current preview is nullptr");
    }

    if (curPreview == m_browser) {
      m_browser->operator()(m_curPath);
    } else if (curPreview == m_labels) {
      m_labels->operator()(m_curPath);
    } else if (curPreview == m_lists) {
      m_lists->operator()(m_curPath);
    } else {
      qWarning("skip current preview is not supported");
    }
  }

  QString GetCurPath() const { return m_curPath; }

  QSize sizeHint() const override;

 private:
  QHash<QString, int> m_name2PreviewIndex;
  QWidget* m_parentDocker;

  QString m_curPath;
  PreviewBrowser* m_browser{nullptr};
  PreviewLabels* m_labels{nullptr};
  PreviewLists* m_lists{nullptr};

  QTimer* m_nextFolderTimer = new QTimer{this};

  static constexpr int NEXT_FOLDER_TIME_INTERVAL = 0.1 * 1000;
  // ms, when NEXT_FOLDER_TIME_INTERVAL <= 0. update preview imgs imediately(may cause lag).
};

#endif  // PREVIEWFOLDER_H
