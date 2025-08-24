#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "FileFolderPreviewer.h"
#include <QMenu>

class ScenesListModel;
class QStyledItemDelegate;
class SceneActionsSubscribe;

class SceneListView : public CustomListView {
 public:
  explicit SceneListView(ScenesListModel* sceneModel, QWidget* parent = nullptr);
  void setFloatingPreview(FileFolderPreviewer* floatingPreview);
  void setRootPath(const QString& rootPath);
  void subscribe();
  void onCopyBaseName();
  void onOpenCorrespondingFolder();
  void onClickEvent(const QModelIndex& idx);

 private:
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  ScenesListModel* _sceneModel;
  QStyledItemDelegate* mAlignDelegate{nullptr};
  FileFolderPreviewer* mPrev_{nullptr};
};

#endif  // SCENELISTVIEW_H
