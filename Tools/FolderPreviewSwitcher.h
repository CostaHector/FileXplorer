#ifndef FOLDERPREVIEWSWITCHER_H
#define FOLDERPREVIEWSWITCHER_H

#include "SelectionPreviewer.h"

class FolderPreviewSwitcher : public QObject {
 public:
  explicit FolderPreviewSwitcher(SelectionPreviewer* folderPreview, QObject* parent = nullptr);

  void onSwitchByViewType(const QString& viewType);

  void onSwitchByViewAction(const QAction* activatedAction);

 private:
  SelectionPreviewer* _folderPreview;
};
#endif  // FOLDERPREVIEWSWITCHER_H
