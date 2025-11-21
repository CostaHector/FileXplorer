#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "SceneSortProxyModel.h"
#include "ScenePageControl.h"
#include "SceneStyleDelegate.h"

class ScenesListModel;

namespace SceneListViewMocker{
inline bool& MockSetRootPathQuery() {
  static bool bConfirm = false;
  return bConfirm;
}
}

class SceneListView : public CustomListView {
  Q_OBJECT
public:
  explicit SceneListView(ScenesListModel* sceneModel, SceneSortProxyModel* sceneSortProxyModel, ScenePageControl* scenePageControl, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  int onUpdateJsonFiles();
  int onUpdateScnFiles();
  int onClearScnFiles();
  void subscribe();
  bool onCopyBaseName();
  bool onOpenCorrespondingFolder();
  bool onClickEvent(const QModelIndex &idx);

signals:
  void currentSceneChanged(const QString& name, const QString& jsonAbsFilePath, const QStringList& imgPthLst, const QStringList& vidsLst);
  void sceneGridClicked(const QModelIndex& ind, const QRect& vRect, const QPoint& clickedPnt);

 public slots:
  void onCellVisualUpdateRequested(const QModelIndex& ind);

protected:
  void mousePressEvent(QMouseEvent* event) override;

private:
  static bool IsPathAtShallowDepth(const QString& path);
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  ScenesListModel* _sceneModel{nullptr};
  SceneSortProxyModel* _sceneSortProxyModel{nullptr};
  SceneStyleDelegate* mAlignDelegate{nullptr};
  ScenePageControl* _scenePageControl{nullptr};

  mutable QModelIndex mLastClickedIndex;
};

#endif  // SCENELISTVIEW_H
