#ifndef FOLDERPREVIEWSWITCHER_H
#define FOLDERPREVIEWSWITCHER_H

#include "CurrentRowPreviewer.h"
#include "PreviewTypeTool.h"

class FolderPreviewSwitcher : public QObject {
 public:
  explicit FolderPreviewSwitcher(CurrentRowPreviewer* folderPreview, QObject* parent = nullptr);
  void onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E viewType);
 private:
  CurrentRowPreviewer* _folderPreview{nullptr};
};
#endif  // FOLDERPREVIEWSWITCHER_H
