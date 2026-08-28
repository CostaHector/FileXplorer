#ifndef SCENELISTVIEW_H
#define SCENELISTVIEW_H

#include "CustomListView.h"
#include "SceneSortProxyModel.h"
#include "ScenePageControl.h"
#include "SceneStyleDelegate.h"
#include "VideoTierTool.h"

class ScenesListModel;

class SceneListView : public CustomListView {
  Q_OBJECT
public:
  explicit SceneListView(ScenesListModel* sceneModel,
                         SceneSortProxyModel* sceneSortProxyModel,
                         ScenePageControl* scenePageControl,
                         QWidget* parent = nullptr);
  ~SceneListView();

  void setRootPath(const QString& rootPath);
  int onUpdateJsonFiles();
  int onUpdateScnFiles();
  int onCreateFrontImageThumbnail();
  int onClearScnFiles();
  void subscribe();
  bool onOpenCorrespondingFolder();
  bool onClickEvent(const QModelIndex& idx);
  QModelIndexList selectedRowsSource() const;

signals:
  void currentSceneChanged(const QString& name, const QString& jsonAbsFilePath, const QStringList& imgPthLst, const QStringList& vidsLst);
  void sceneGridClicked(const QModelIndex& ind, const QRect& vRect, const QPoint& clickedPnt);

public slots:
  void onCellVisualUpdateRequested(const QModelIndex& ind);
  void toggleSortRequestImplementer(bool bPageByPage);
  int onArchiveActionTriggered(const QAction* archivedToAct);
  int onArchiveTo(int videoTier);
  int onArchiveToByMovieRate();
  int onAddTags(const QString& tags) const;

protected:
  void mousePressEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* e) override;

private:
  void initExclusivePreferenceSetting() override;
  int ArchiveToCore(const QModelIndexList& indexes, const QStringList (&movieTier2Jsons)[(int)VideoTierTool::VideoTierE::BUTT_INVALID]);

  QAction* _OPEN_CORRESPONDING_FOLDER{nullptr};

  ScenesListModel* _sceneModel{nullptr};
  SceneSortProxyModel* _sceneSortProxyModel{nullptr};
  SceneStyleDelegate* mAlignDelegate{nullptr};
  ScenePageControl* _scenePageControl{nullptr};

  QMetaObject::Connection mSortRoleConn;
  QMetaObject::Connection mSortOrderReverseConn;

  mutable QModelIndex mLastClickedIndex;
};

#endif  // SCENELISTVIEW_H
