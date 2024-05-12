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

  QAction* LISTS{new QAction{"lists", this}};
  QAction* BROWSER{new QAction{"browser", this}};
  QAction* LABELS{new QAction{"labels", this}};
  QActionGroup* PREVIEW_AG{new QActionGroup{this}};
};

FolderPreviewActions& g_folderPreviewActions();
#endif  // FOLDERPREVIEWACTIONS_H
