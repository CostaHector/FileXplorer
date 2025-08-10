#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "FloatingPreview.h"
#include <QMenu>

class ScenesListModel;
class QStyledItemDelegate;
class SceneActionsSubscribe;

class SceneListView : public CustomListView {
 public:
  explicit SceneListView(ScenesListModel* sceneModel, QWidget* parent = nullptr);
  void setFloatingPreview(FloatingPreview* floatingPreview);
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
  FloatingPreview* mPrev_{nullptr};
};

#endif  // SCENELISTVIEW_H
