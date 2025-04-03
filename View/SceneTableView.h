#ifndef SCENETABLEVIEW_H
#define SCENETABLEVIEW_H

#include "View/CustomTableView.h"
#include "Component/FolderPreview/FloatingPreview.h"
#include <QMenu>

class ScenesTableModel;
class QStyledItemDelegate;
class SceneActionsSubscribe;

class SceneTableView : public CustomTableView {
 public:
  explicit SceneTableView(ScenesTableModel* sceneModel, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  void subscribe();
  void onCopyBaseName();
  void onOpenCorrespondingFolder();

 private:
  void mouseMoveEvent(QMouseEvent *event) override;
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  QMenu* m_menu{nullptr};
  ScenesTableModel* _sceneModel;
  QStyledItemDelegate* mAlignDelegate{nullptr};
  FloatingPreview* m_fPrev{nullptr};
};

#endif  // SCENETABLEVIEW_H
