#ifndef SCENETABLEVIEW_H
#define SCENETABLEVIEW_H

#include "View/CustomTableView.h"
class ScenesTableModel;
class QStyledItemDelegate;
class SceneActionsSubscribe;
class QMenu;
class SceneTableView : public CustomTableView {
 public:
  explicit SceneTableView(ScenesTableModel* sceneModel, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  void subscribe();
  void onCopyBaseName();
 private:
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QMenu* m_menu{nullptr};
  ScenesTableModel* _sceneModel;
  QStyledItemDelegate* mAlignDelegate{nullptr};
};


#endif // SCENETABLEVIEW_H
