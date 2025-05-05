#include "PreviewFolder.h"

#include "public/PublicVariable.h"

constexpr int PreviewFolder::NEXT_FOLDER_TIME_INTERVAL;  // ms

PreviewFolder::PreviewFolder(QWidget* parent)  //
    : QStackedWidget{parent},                  //
      m_parentDocker{parent}                   //
{
  if (isTimerDisabled()) {
    return;
  }
  m_nextFolderTimer->setInterval(PreviewFolder::NEXT_FOLDER_TIME_INTERVAL);
  m_nextFolderTimer->setSingleShot(true);
  connect(m_nextFolderTimer, &QTimer::timeout, this, &PreviewFolder::UpdatePreview);
}

void PreviewFolder::UpdatePreview() {
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

QSize PreviewFolder::sizeHint() const {
  auto w = PreferenceSettings().value("dockerFolderPreviewWidth", DOCKER_DEFAULT_SIZE.width()).toInt();
  auto h = PreferenceSettings().value("dockerFolderPreviewHeight", DOCKER_DEFAULT_SIZE.height()).toInt();
  return QSize(w, h);
}
