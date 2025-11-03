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
  if (mCurrentSrcFrom == SRC_FROM::FILE_SYSTEM_VIEW) {
    switch (mCurrentPreviewType) {
      case PREVIEW_TYPE_E::NONE: // no preview
        return;
      case PREVIEW_TYPE_E::CATEGORY:
        m_fileFolderPreviewStackedWid->operator()(m_curPath);
        return;
      case PREVIEW_TYPE_E::PROGRESSIVE_LOAD:
        m_imgInFolderBrowser->operator()(m_curPath);
        return;
      case PREVIEW_TYPE_E::CAROUSEL:
        m_imgInFolderLabels->operator()(m_curPath);
        return;
      default:
        LOG_W("SrcFrom[%d], Current previewer[%s] is not supported", int(mCurrentSrcFrom), c_str(mCurrentPreviewType));
    }
  } else if (mCurrentSrcFrom == SRC_FROM::CAST) {
    switch (mCurrentPreviewType) {
      case PREVIEW_TYPE_E::CATEGORY:
        m_fileFolderPreviewStackedWid->operator()(m_curRecord, m_curImageHostPath);
        return;
      default:
        LOG_W("SrcFrom[%d], Current previewer[%s] is not supported", int(mCurrentSrcFrom), c_str(mCurrentPreviewType));
    }
  } else if (mCurrentSrcFrom == SRC_FROM::SCENE) {
    switch (mCurrentPreviewType) {
      case PREVIEW_TYPE_E::CATEGORY:
        m_fileFolderPreviewStackedWid->operator()(m_sceneName, m_sceneJsonAbsFilePath, m_sceneimgPthLst, m_scenevidsLst);
        return;
      default:
        LOG_W("SrcFrom[%d], Current previewer[%s] is not supported", int(mCurrentSrcFrom), c_str(mCurrentPreviewType));
    }
  }
}

QSize CurrentRowPreviewer::sizeHint() const {
  static const int w = Configuration().value("SELECTION_PREVIEWER_WIDTH", DOCKER_DEFAULT_SIZE.width()).toInt();
  static const int h = Configuration().value("SELECTION_PREVIEWER_WIDTH", DOCKER_DEFAULT_SIZE.height()).toInt();
  return {w, h};
}

bool CurrentRowPreviewer::NeedInitPreviewWidget(PreviewTypeTool::PREVIEW_TYPE_E previewType) const {
  switch (previewType) {
    case PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY: {
      return m_fileFolderPreviewStackedWid == nullptr;
    }
    case PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD: {
      return m_imgInFolderBrowser == nullptr;
    }
    case PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL: {
      return m_imgInFolderLabels == nullptr;
    }
    case PreviewTypeTool::PREVIEW_TYPE_E::NONE: {
      return false;
    }
    default:
      LOG_E("previewType[%s] not support", PreviewTypeTool::c_str(previewType));
      return false;
  }
  return false;
}

bool CurrentRowPreviewer::InitPreviewAndAddView(PreviewTypeTool::PREVIEW_TYPE_E previewType) {
  switch (previewType) {
    case PreviewTypeTool::PREVIEW_TYPE_E::CATEGORY: {
      m_fileFolderPreviewStackedWid = new (std::nothrow) FileFolderPreviewer{"DockerList", this};
      CHECK_NULLPTR_RETURN_FALSE(m_fileFolderPreviewStackedWid)
      AddView(previewType, m_fileFolderPreviewStackedWid);
      break;
    }
    case PreviewTypeTool::PREVIEW_TYPE_E::PROGRESSIVE_LOAD: {
      m_imgInFolderBrowser = new (std::nothrow) ImagesInFolderBrowser{this};
      CHECK_NULLPTR_RETURN_FALSE(m_imgInFolderBrowser)
      AddView(previewType, m_imgInFolderBrowser);
      break;
    }
    case PreviewTypeTool::PREVIEW_TYPE_E::CAROUSEL: {
      m_imgInFolderLabels = new (std::nothrow) ImagesInFolderSlider{this};
      CHECK_NULLPTR_RETURN_FALSE(m_imgInFolderLabels)
      AddView(previewType, m_imgInFolderLabels);
      break;
    }
    default:
      LOG_E("previewType[%s] not support", PreviewTypeTool::c_str(previewType));
      return false;
  }
  return true;
}

bool CurrentRowPreviewer::setCurrentPreviewType(PreviewTypeTool::PREVIEW_TYPE_E previewType) {
  auto prevIt = m_name2PreviewIndex.find(previewType);
  mCurrentPreviewType = previewType;
  if (prevIt == m_name2PreviewIndex.end()) {
    LOG_D("previewType[%s] not in map", PreviewTypeTool::c_str(previewType));
    return false;
  }
  int viewIndex = prevIt.value();
  QStackedWidget::setCurrentIndex(viewIndex);
  emit windowTitleChanged(QString{"Preview: %1"}.arg(PreviewTypeTool::c_str(previewType)));
  return true;
}
