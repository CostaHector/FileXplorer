#ifndef FOLDERPREVIEWSWITCHER_H
#define FOLDERPREVIEWSWITCHER_H

#include "SelectionPreviewer.h"
#include "PreviewTypeTool.h"

class FolderPreviewSwitcher : public QObject {
 public:
  explicit FolderPreviewSwitcher(SelectionPreviewer* folderPreview, QObject* parent = nullptr);
  void onSwitchByViewType(PreviewTypeTool::PREVIEW_TYPE_E viewType);
 private:
  SelectionPreviewer* _folderPreview{nullptr};
};
#endif  // FOLDERPREVIEWSWITCHER_H
