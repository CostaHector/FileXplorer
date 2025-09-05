#ifndef FOLDERPREVIEWACTIONS_H
#define FOLDERPREVIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>
#include "PreviewTypeToolBar.h"

class FolderPreviewActions : public QObject {
 public:
  explicit FolderPreviewActions(QObject* parent = nullptr);
  PreviewTypeToolBar* GetPreviewsToolbar(QWidget* parent = nullptr);
  PreviewTypeToolBar* mFolderPreviewToolBar{nullptr};
};

FolderPreviewActions& g_folderPreviewActions();
#endif  // FOLDERPREVIEWACTIONS_H
