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

  QAction* LISTS{nullptr};
  QAction* BROWSER{nullptr};
  QAction* LABELS{nullptr};
  QActionGroup* PREVIEW_AG{nullptr};
};

FolderPreviewActions& g_folderPreviewActions();
#endif  // FOLDERPREVIEWACTIONS_H
