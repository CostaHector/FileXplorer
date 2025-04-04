#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "View/CustomListView.h"
#include "Component/FolderPreview/FloatingPreview.h"
#include <QMenu>

class ScenesListModel;
class QStyledItemDelegate;
class SceneActionsSubscribe;

class SceneListView : public CustomListView {
 public:
  explicit SceneListView(ScenesListModel* sceneModel, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  void subscribe();
  void onCopyBaseName();
  void onOpenCorrespondingFolder();

 private:
  void mouseMoveEvent(QMouseEvent *event) override;
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  ScenesListModel* _sceneModel;
  QStyledItemDelegate* mAlignDelegate{nullptr};
  FloatingPreview* m_fPrev{nullptr};
};

#endif  // SCENELISTVIEW_H
