#include "FolderPreviewActions.h"
#include "MemoryKey.h"
#include "StyleSheet.h"

FolderPreviewActions::FolderPreviewActions(QObject* parent) : QObject{parent} { }

PreviewTypeToolBar* FolderPreviewActions::GetPreviewsToolbar(QWidget* parent) {
  if (mFolderPreviewToolBar == nullptr) {
    mFolderPreviewToolBar = new (std::nothrow) PreviewTypeToolBar{"Folder Previews toolbar", parent};
    CHECK_NULLPTR_RETURN_NULLPTR(mFolderPreviewToolBar)
  }
  return mFolderPreviewToolBar;
}

FolderPreviewActions& g_folderPreviewActions() {
  static FolderPreviewActions ins;
  return ins;
}
