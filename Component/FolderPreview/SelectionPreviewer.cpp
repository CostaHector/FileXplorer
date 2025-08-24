#include "SelectionPreviewer.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

constexpr int SelectionPreviewer::NEXT_FOLDER_TIME_INTERVAL;  // ms

SelectionPreviewer::SelectionPreviewer(QWidget* parent)  //
    : QStackedWidget{parent},                  //
      m_parentDocker{parent}                   //
{
  if (isTimerDisabled()) {
    return;
  }
  m_nextFolderTimer->setInterval(SelectionPreviewer::NEXT_FOLDER_TIME_INTERVAL);
  m_nextFolderTimer->setSingleShot(true);
  connect(m_nextFolderTimer, &QTimer::timeout, this, &SelectionPreviewer::UpdatePreview);
}

void SelectionPreviewer::UpdatePreview() {
  auto* curPreview = currentWidget();
  CHECK_NULLPTR_RETURN_VOID(curPreview)
  if (curPreview == m_imgInFolderBrowser) {
    m_imgInFolderBrowser->operator()(m_curPath);
  } else if (curPreview == m_imgInFolderLabels) {
    m_imgInFolderLabels->operator()(m_curPath);
  } else if (curPreview == m_fileFolderPreviewStackedWid) {
    m_fileFolderPreviewStackedWid->operator()(m_curPath);
  } else {
    qWarning("Current previewer[%d] is not supported", currentIndex());
  }
}

QSize SelectionPreviewer::sizeHint() const {
  static const int w {Configuration().value("SELECTION_PREVIEWER_WIDTH", DOCKER_DEFAULT_SIZE.width()).toInt()};
  static const int h {Configuration().value("SELECTION_PREVIEWER_HEIGHT", DOCKER_DEFAULT_SIZE.height()).toInt()};
  static const QSize sz{w, h};
  return sz;
}
