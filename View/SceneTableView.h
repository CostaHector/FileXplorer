#ifndef SCENETABLEVIEW_H
#define SCENETABLEVIEW_H


#include "View/CustomTableView.h"
class ScenesTableModel;
class QStyledItemDelegate;
class SceneTableView : public CustomTableView {
 public:
  explicit SceneTableView(ScenesTableModel* sceneModel, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
 private:
  ScenesTableModel* _sceneModel;
  QStyledItemDelegate* mAlignDelegate{nullptr};
};


#endif // SCENETABLEVIEW_H
