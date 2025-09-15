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
  if (_folderPreview->NeedInitPreviewWidget(viewType)) {
    bool addPreviewRet = _folderPreview->InitPreviewAndAddView(viewType);
    if (!addPreviewRet) {
      LOG_C("Add Preview type[%s] failed!", PreviewTypeTool::c_str(viewType));
      return;
    }
  }
  _folderPreview->operator()(_folderPreview->GetCurPath());
  _folderPreview->setVisible(true);
  _folderPreview->setCurrentPreviewType(viewType);
}
