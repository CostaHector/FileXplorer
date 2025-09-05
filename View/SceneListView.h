#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "FileFolderPreviewer.h"
#include <QMenu>

class ScenesListModel;
class QStyledItemDelegate;
class SceneActionsSubscribe;

class SceneListView : public CustomListView {
  Q_OBJECT
public:
  explicit SceneListView(ScenesListModel* sceneModel, QWidget* parent = nullptr);
  void setRootPath(const QString& rootPath);
  void subscribe();
  void onCopyBaseName();
  void onOpenCorrespondingFolder();
  void onClickEvent(const QModelIndex &idx, const QModelIndex &previous);

signals:
  void currentSceneChanged(const QString& name, const QStringList& imgPthLst, const QStringList& vidsLst);

private:
  QAction* COPY_BASENAME_FROM_SCENE{nullptr};
  QAction* OPEN_CORRESPONDING_FOLDER{nullptr};
  ScenesListModel* _sceneModel{nullptr};
  QStyledItemDelegate* mAlignDelegate{nullptr};
};

#endif  // SCENELISTVIEW_H
