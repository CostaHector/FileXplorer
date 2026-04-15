#ifndef FAVORITESTREEVIEW_H
#define FAVORITESTREEVIEW_H

#include "CustomTreeView.h"
#include "FavoritesTreeModel.h"
#include "RecursiveFilterProxyTreeModel.h"
#include "EnumIntAction.h"

extern template struct EnumIntAction<FavoriteItemData::Role>;

class FavoritesTreeView : public CustomTreeView {
  Q_OBJECT
 public:
  explicit FavoritesTreeView(QWidget* parent = nullptr);
  ~FavoritesTreeView();
  void setFilter(const QString& filter);

 signals:
  bool reqIntoAPath(QString fullPath, bool isNew);

 protected:
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dropEvent(QDropEvent* event) override;

 private:
  void subscribe();

  bool onRenameDisplayRole();
  bool onAddAGroup();
  int onRemoveSelection();
  bool onItemClicked(const QModelIndex& proIndex);
  void onSortRoleActionTriggered(const QAction* newSortRoleAction);

  bool isExactlyOneGroupSelected(QModelIndex* grpSrcIndex = nullptr) const;
  bool isNoSelectionOrExactlyOneGroup(QModelIndex* grpOrRootSrcIndex = nullptr) const;

  FavoritesTreeModel* mFavModel{nullptr};
  RecursiveFilterProxyTreeModel* mFavProxyModel{nullptr};

  QAction* mSortByName{nullptr};
  QAction* mSortByIsGroup{nullptr};
  QAction* mSortByFullPathRole{nullptr};
  QAction* mSortByLastAccessTime{nullptr};
  QAction* mSortByAccessCount{nullptr};
  EnumIntAction<FavoriteItemData::Role> mSortRoleIntAction;
  QAction* mSortReverse{nullptr};

  QAction* mRenameDisplayRole{nullptr};
  QAction* mAddAGroup{nullptr};
  QAction* mRemoveSelection{nullptr};

  QAction* mAddInitialExamples{nullptr};
  QAction* mNotSavedDatasThisTime{nullptr};
  QAction* mSaveRightNow{nullptr};
};

#endif  // FAVORITESTREEVIEW_H
