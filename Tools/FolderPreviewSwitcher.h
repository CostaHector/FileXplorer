#ifndef FOLDERPREVIEWSWITCHER_H
#define FOLDERPREVIEWSWITCHER_H

#include "PreviewFolder.h"

class FolderPreviewSwitcher : public QObject {
 public:
  explicit FolderPreviewSwitcher(PreviewFolder* folderPreview, QWidget* parentDocker = nullptr, QObject* parent = nullptr);

  void onSwitchByViewType(const QString& viewType);

  void onSwitchByViewAction(const QAction* activatedAction);

 private:
  PreviewFolder* _folderPreview;
  QWidget* _parentDocker;
};
#endif  // FOLDERPREVIEWSWITCHER_H
