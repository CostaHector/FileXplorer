#include "FolderPreviewSwitcher.h"
#include "FolderPreviewActions.h"
#include "PublicMacro.h"
#include "PreviewTypeToolBar.h"

FolderPreviewSwitcher::FolderPreviewSwitcher(CurrentRowPreviewer* folderPreview, QObject* parent)//
  : QObject{parent}, _folderPreview{folderPreview} //
{
  PreviewTypeToolBar* previewToolbar = g_folderPreviewActions().GetPreviewsToolbar(folderPreview);
  onSwitchByViewType(previewToolbar->mCurrentPreviewType);
}

void FolderPreviewSwitcher::onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E viewType) {
  CHECK_NULLPTR_RETURN_VOID(_folderPreview)
  using namespace PreviewTypeTool;
  switch (viewType) {
    case PREVIEW_TYPE_E::STACKS: {
      if (_folderPreview->m_fileFolderPreviewStackedWid == nullptr) {
        _folderPreview->m_fileFolderPreviewStackedWid = new (std::nothrow) FileFolderPreviewer{"DockerList", _folderPreview};
        CHECK_NULLPTR_RETURN_VOID(_folderPreview->m_fileFolderPreviewStackedWid)
        _folderPreview->AddView(viewType, _folderPreview->m_fileFolderPreviewStackedWid);
      }
      _folderPreview->m_fileFolderPreviewStackedWid->operator()(_folderPreview->GetCurPath());
      break;
    }
    case PREVIEW_TYPE_E::BROWSER: {
      if (_folderPreview->m_imgInFolderBrowser == nullptr) {
        _folderPreview->m_imgInFolderBrowser = new (std::nothrow) ImagesInFolderBrowser{_folderPreview};
        CHECK_NULLPTR_RETURN_VOID(_folderPreview->m_imgInFolderBrowser)
        _folderPreview->AddView(viewType, _folderPreview->m_imgInFolderBrowser);
      }
      _folderPreview->m_imgInFolderBrowser->operator()(_folderPreview->GetCurPath());
      break;
    }
    case PREVIEW_TYPE_E::SLIDERS: {
      if (_folderPreview->m_imgInFolderLabels == nullptr) {
        _folderPreview->m_imgInFolderLabels = new (std::nothrow) ImagesInFolderSlider{_folderPreview};
        CHECK_NULLPTR_RETURN_VOID(_folderPreview->m_imgInFolderLabels)
        _folderPreview->AddView(viewType, _folderPreview->m_imgInFolderLabels);
      }
      _folderPreview->m_imgInFolderLabels->operator()(_folderPreview->GetCurPath());
      break;
    }
    case PREVIEW_TYPE_E::NONE: {
      _folderPreview->setVisible(false);
      return;
    }
    default: {
      LOG_W("previewType[%s] not support now.", c_str(viewType));
      return;
    }
  }
  _folderPreview->setVisible(true);
  int viewIndex = _folderPreview->m_name2PreviewIndex[viewType];
  _folderPreview->setCurrentIndex(viewIndex);
}
