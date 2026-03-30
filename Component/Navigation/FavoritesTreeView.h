#ifndef FAVORITESTREEVIEW_H
#define FAVORITESTREEVIEW_H

#include <QTreeView>
#include <QMenu>
#include "FavoritesTreeModel.h"
#include "RecursiveFilterProxyTreeModel.h"

class FavoritesTreeView : public QTreeView {
  Q_OBJECT
 public:
  explicit FavoritesTreeView(const QString& name = "FavoritesTreeView", QWidget* parent = nullptr);
  void setFilter(const QString& filter);
  QString GetName() const { return m_name; }

 signals:
  bool reqIntoAPath(QString fullPath, bool isNew);

 protected:
  void contextMenuEvent(QContextMenuEvent* event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent* event) override;

 private:
  bool onRenameDisplayRole();
  bool onAddAGroup();
  int onRemoveSelection();
  bool onItemClicked(const QModelIndex& proIndex);

  bool isExactlyOneGroupSelected(QModelIndex* grpSrcIndex=nullptr) const;
  bool isNoSelectionOrExactlyOneGroup(QModelIndex* grpOrRootSrcIndex) const;

  FavoritesTreeModel* mFavModel{nullptr};
  RecursiveFilterProxyTreeModel* mFavProxyModel{nullptr};

  QMenu* mMenu{nullptr};
  QAction *mExpandAll{nullptr}, *mCollapseAll{nullptr};
  QAction* mAnimatedEnableAct{nullptr};
  QAction* mRenameDisplayRole{nullptr};
  QAction* mAddAGroup{nullptr};
  QAction* mRemoveSelection{nullptr};
  QAction* mNotSavedDatasThisTime{nullptr};
  QAction* mSaveRightNow{nullptr};

  const QString m_name;
};

#endif  // FAVORITESTREEVIEW_H
