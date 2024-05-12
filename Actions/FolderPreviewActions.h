#ifndef FOLDERPREVIEWACTIONS_H
#define FOLDERPREVIEWACTIONS_H

#include <QAction>
#include <QActionGroup>
#include <QToolBar>

#include <QWidget>

class FolderPreviewActions : public QObject {
 public:
  explicit FolderPreviewActions(QObject* parent = nullptr);
  QToolBar* GetPreviewsToolbar(QWidget* parent) {
    auto* previewTB = new QToolBar{"previews toolbar", parent};
    previewTB->addActions(PREVIEW_AG->actions());
    return previewTB;
  }

  QAction* LISTS{new QAction{QIcon(":/themes/FOLDER_PREVIEW_LISTS"), "lists", this}};
  QAction* BROWSER{new QAction{QIcon(":/themes/FOLDER_PREVIEW_BROWSER"), "browser", this}};
  QAction* LABELS{new QAction{QIcon(":/themes/FOLDER_PREVIEW_LABELS"), "labels", this}};
  QActionGroup* PREVIEW_AG{new QActionGroup{this}};
};

FolderPreviewActions& g_folderPreviewActions();
#endif  // FOLDERPREVIEWACTIONS_H
