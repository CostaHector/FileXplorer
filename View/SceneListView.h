#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "FileFolderPreviewer.h"
#include "SceneSortProxyModel.h"
#include "ScenePageControl.h"
#include <QMenu>

class ScenesListModel;
class QStyledItemDelegate;

class SceneListView : public CustomListView {
  Q_OBJECT
public:
  explicit SceneListView(ScenesListModel* sceneModel, SceneSortProxyModel* sceneSortProxyModel, ScenePageControl* scenePageControl, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  void subscribe();
  void onCopyBaseName();
  void onOpenCorrespondingFolder();
  void onClickEvent(const QModelIndex &idx, const QModelIndex &previous);

  bool PageIndexIncDec(const QAction* pageAct);

signals:
  void currentSceneChanged(const QString& name, const QStringList& imgPthLst, const QStringList& vidsLst);

private:
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  ScenesListModel* _sceneModel{nullptr};
  SceneSortProxyModel* _sceneSortProxyModel{nullptr};
  QStyledItemDelegate* mAlignDelegate{nullptr};
  ScenePageControl* _scenePageControl{nullptr};
};

#endif  // SCENELISTVIEW_H
