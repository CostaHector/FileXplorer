#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "SceneSortProxyModel.h"
#include "ScenePageControl.h"
#include <QStyledItemDelegate>
#include <QMenu>

class ScenesListModel;

namespace SceneListViewMocker{
inline bool& MockSetRootPathQuery() {
  static bool bConfirm = false;
  return bConfirm;
}
}

class AlignDelegate : public QStyledItemDelegate {
 public:
  void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
  QString displayText(const QVariant& value, const QLocale& /**/) const override;
};

class SceneListView : public CustomListView {
  Q_OBJECT
public:
  explicit SceneListView(ScenesListModel* sceneModel, SceneSortProxyModel* sceneSortProxyModel, ScenePageControl* scenePageControl, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  int onUpdateScnFiles();
  int onClearScnFiles();
  void subscribe();
  void onCopyBaseName();
  void onOpenCorrespondingFolder();
  void onClickEvent(const QModelIndex &idx, const QModelIndex &previous);

signals:
  void currentSceneChanged(const QString& name, const QStringList& imgPthLst, const QStringList& vidsLst);

private:
  static bool IsPathAtShallowDepth(const QString& path);
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  ScenesListModel* _sceneModel{nullptr};
  SceneSortProxyModel* _sceneSortProxyModel{nullptr};
  QStyledItemDelegate* mAlignDelegate{nullptr};
  ScenePageControl* _scenePageControl{nullptr};
};

#endif  // SCENELISTVIEW_H
