#include "CurrentRowPreviewer.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

constexpr int CurrentRowPreviewer::NEXT_FOLDER_TIME_INTERVAL;  // ms

CurrentRowPreviewer::CurrentRowPreviewer(QWidget* parent)  //
  : QStackedWidget{parent},                  //
  m_parentDocker{parent}, m_nextFolderTimer{this}                   //
{
  if (!isTimerDisabled()) {
    m_nextFolderTimer.setInterval(CurrentRowPreviewer::NEXT_FOLDER_TIME_INTERVAL);
    m_nextFolderTimer.setSingleShot(true);
    connect(&m_nextFolderTimer, &QTimer::timeout, this, &CurrentRowPreviewer::UpdatePreview);
  }
}

CurrentRowPreviewer::~CurrentRowPreviewer() {
  Configuration().setValue(MemoryKey::FOLDER_PREVIEW_TYPE.name, (int)mCurrentPreviewType);
}

void CurrentRowPreviewer::UpdatePreview() {
  using namespace PreviewTypeTool;
  switch (mCurrentPreviewType) {
    case PREVIEW_TYPE_E::NONE: // no preview
      return;
    case PREVIEW_TYPE_E::STACKS:
      m_fileFolderPreviewStackedWid->operator()(m_curPath);
      return;
    case PREVIEW_TYPE_E::BROWSER:
      m_imgInFolderBrowser->operator()(m_curPath);
      return;
    case PREVIEW_TYPE_E::SLIDERS:
      m_imgInFolderLabels->operator()(m_curPath);
      return;
    default:
      qWarning("Current previewer[%s] is not supported", c_str(mCurrentPreviewType));
  }
}

QSize CurrentRowPreviewer::sizeHint() const {
  return {Configuration().value("SELECTION_PREVIEWER_WIDTH", DOCKER_DEFAULT_SIZE.width()).toInt(),
          Configuration().value("SELECTION_PREVIEWER_HEIGHT", DOCKER_DEFAULT_SIZE.height()).toInt()};
}

void CurrentRowPreviewer::setCurrentIndex(int index) {
  QStackedWidget::setCurrentIndex(index);
  mCurrentPreviewType = m_previewIndex2NameE[index];
  emit windowTitleChanged(QString{"Preview: %1"}.arg(PreviewTypeTool::c_str(mCurrentPreviewType)));
}
