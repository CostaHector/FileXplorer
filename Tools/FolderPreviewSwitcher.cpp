#include "FolderPreviewSwitcher.h"
#include "FolderPreviewActions.h"
#include "PublicMacro.h"
#include "PreviewTypeToolBar.h"

FolderPreviewSwitcher::FolderPreviewSwitcher(CurrentRowPreviewer* folderPreview, QObject* parent)//
  : QObject{parent}, _folderPreview{folderPreview} //
{
  PreviewTypeToolBar* previewToolbarTmp = g_folderPreviewActions().GetPreviewsToolbar(folderPreview);
  onSwitchByViewType(previewToolbarTmp->mCurrentPreviewType);
}

bool FolderPreviewSwitcher::onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E viewType) {
  CHECK_NULLPTR_RETURN_FALSE(_folderPreview)
  if (viewType == PreviewTypeTool::PREVIEW_TYPE_E::NONE) {
    return false;
  }
  if (_folderPreview->NeedInitPreviewWidget(viewType)) {
    bool addPreviewRet = _folderPreview->InitPreviewAndAddView(viewType);
    if (!addPreviewRet) {
      LOG_C("Add Preview type[%s] failed!", PreviewTypeTool::c_str(viewType));
      return false;
    }
  }
  _folderPreview->operator()(_folderPreview->GetCurPath());
  _folderPreview->setCurrentPreviewType(viewType);
  return true;
}
