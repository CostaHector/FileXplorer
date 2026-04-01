#include "FavoritesTreeView.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMimeData>

FavoritesTreeView::FavoritesTreeView(QWidget* parent)  //
    : CustomTreeView{"FavoritesTreeView", parent} {
  setObjectName("FavoritesTreeView");
  setIndentation(12);
  mFavModel = new FavoritesTreeModel{GetName(), this};

  mFavProxyModel = new RecursiveFilterProxyTreeModel{this};
  mFavProxyModel->setSourceModel(mFavModel);
  setModel(mFavProxyModel);

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setDragDropMode(QAbstractItemView::DragDrop);

  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setExpandsOnDoubleClick(true);
  setSortingEnabled(true);

  QList<QAction*> exclusiveActs;
  exclusiveActs.reserve(20);

  // Expand all
  mExpandAll = new QAction(QIcon{":img/EXPAND_ALL"}, tr("Expand All"), this);
  mExpandAll->setToolTip("Expand all groups to show their contents");
  exclusiveActs.push_back(mExpandAll);

  // Collapse all
  mCollapseAll = new QAction(QIcon{":img/COLLAPSE_ALL"}, tr("Collapse All"), this);
  mCollapseAll->setToolTip("Collapse all groups to hide their contents");
  exclusiveActs.push_back(mCollapseAll);

  // Animated
  mAnimatedEnableAct = new QAction(tr("Enable Animation"), this);
  mAnimatedEnableAct->setToolTip("Enable or disable smooth expand/collapse animations");
  exclusiveActs.push_back(mAnimatedEnableAct);
  {
    const bool bDefAnimated{false};
    mAnimatedEnableAct->setCheckable(true);
    mAnimatedEnableAct->setChecked(bDefAnimated);
    setAnimated(bDefAnimated);
  }

  mRootDecorationEnabled = new QAction(tr("Root Decoration"), this);
  exclusiveActs.push_back(mRootDecorationEnabled);
  {
    const bool bShowRootDecoration{true};
    mRootDecorationEnabled->setCheckable(true);
    mRootDecorationEnabled->setChecked(bShowRootDecoration);
    setRootIsDecorated(bShowRootDecoration);
  }

  {
    QMenu* mSortRoleMenu = new QMenu(tr("Sort"), this);
    mSortByName = mSortRoleMenu->addAction(tr("Name"));
    mSortByIsGroup = mSortRoleMenu->addAction(tr("Is Group"));
    mSortByFullPathRole = mSortRoleMenu->addAction(tr("Full Path"));
    mSortByLastAccessTime = mSortRoleMenu->addAction(tr("Last Access Time"));
    mSortByAccessCount = mSortRoleMenu->addAction(tr("Access Count"));
    mSortByName->setCheckable(true);
    mSortByIsGroup->setCheckable(true);
    mSortByFullPathRole->setCheckable(true);
    mSortByLastAccessTime->setCheckable(true);
    mSortByAccessCount->setCheckable(true);
    mSortRoleIntAction.init(
        {
            {mSortByName, FavoriteItemData::Role::DEF_NAME_TEXT_ROLE},          //
            {mSortByIsGroup, FavoriteItemData::Role::IS_GROUP_ROLE},            //
            {mSortByFullPathRole, FavoriteItemData::Role::FULL_PATH_ROLE},      //
            {mSortByLastAccessTime, FavoriteItemData::Role::LAST_ACCESS_ROLE},  //
            {mSortByAccessCount, FavoriteItemData::Role::ACCESS_COUNT_ROLE},    //
        },
        FavoriteItemData::DEF_SORT_ROLE, QActionGroup::ExclusionPolicy::Exclusive);
    const FavoriteItemData::Role initMemoryRole{FavoriteItemData::GetInitialSortRole()};
    mSortRoleIntAction.setCheckedIfActionExist(initMemoryRole);

    const bool isSortReverse{FavoriteItemData::GetInitialSortOrderReverse()};
    mSortReverse = new QAction(tr("Sort Reverse"), this);
    mSortReverse->setCheckable(true);
    mSortReverse->setChecked(isSortReverse);

    mFavProxyModel->initSortProxy(initMemoryRole, isSortReverse);

    exclusiveActs.push_back(mSortRoleMenu->menuAction());
  }

  // Rename
  mRenameDisplayRole = new QAction(QIcon{":img/RENAME"}, tr("Rename"), this);
  mRenameDisplayRole->setToolTip("Rename the selected item");
  exclusiveActs.push_back(mRenameDisplayRole);

  // Add group
  mAddAGroup = new QAction(QIcon{":img/WHERE_CLAUSE_HISTORY_ADD"}, tr("Add Group"), this);
  mAddAGroup->setToolTip("Add a new group to organize favorites");
  exclusiveActs.push_back(mAddAGroup);

  // Remove selection
  mRemoveSelection = new QAction(QIcon{":img/WHERE_CLAUSE_HISTORY_REMOVED"}, tr("Delete Selected"), this);
  mRemoveSelection->setToolTip("Remove the selected items");
  exclusiveActs.push_back(mRemoveSelection);

  // Add initial examples
  mAddInitialExamples = new QAction(tr("Add Initial Examples"), this);
  exclusiveActs.push_back(mAddInitialExamples);

  // Skip saving
  mNotSavedDatasThisTime = new QAction(tr("Skip Saving"), this);
  mNotSavedDatasThisTime->setCheckable(true);
  mNotSavedDatasThisTime->setChecked(false);
  mNotSavedDatasThisTime->setToolTip("Skip saving changes on exit (temporary mode). by default not skip");
  exclusiveActs.push_back(mNotSavedDatasThisTime);

  // Save immediately
  mSaveRightNow = new QAction(QIcon{":/JsonEditor/SAVE_CHANGES"}, tr("Save Now"), this);
  mSaveRightNow->setToolTip("Save all changes immediately");
  exclusiveActs.push_back(mSaveRightNow);

  PushFrontExclusiveActions(exclusiveActs);
  subscribe();
}

FavoritesTreeView::~FavoritesTreeView() {
  FavoriteItemData::SaveInitialSortRole(mSortRoleIntAction.curVal());
  FavoriteItemData::SaveSortOrderReverse(mSortReverse->isChecked());
}

void FavoritesTreeView::subscribe() {
  connect(this, &QTreeView::clicked, this, &FavoritesTreeView::onItemClicked);
  connect(mExpandAll, &QAction::triggered, this, &FavoritesTreeView::expandAll);
  connect(mCollapseAll, &QAction::triggered, this, &FavoritesTreeView::collapseAll);

  connect(mAnimatedEnableAct, &QAction::toggled, this, &FavoritesTreeView::setAnimated);
  connect(mRootDecorationEnabled, &QAction::toggled, this, &FavoritesTreeView::setRootIsDecorated);

  connect(mSortRoleIntAction.getActionGroup(), &QActionGroup::triggered, this, &FavoritesTreeView::onSortRoleActionTriggered);
  connect(mSortReverse, &QAction::toggled, mFavProxyModel, &RecursiveFilterProxyTreeModel::setSortOrder);

  connect(mRenameDisplayRole, &QAction::triggered, this, &FavoritesTreeView::onRenameDisplayRole);
  connect(mAddAGroup, &QAction::triggered, this, &FavoritesTreeView::onAddAGroup);
  connect(mRemoveSelection, &QAction::triggered, this, &FavoritesTreeView::onRemoveSelection);

  connect(mAddInitialExamples, &QAction::triggered, mFavModel, &FavoritesTreeModel::addInitialFavoritesGroup);
  connect(mNotSavedDatasThisTime, &QAction::toggled, mFavModel, &FavoritesTreeModel::setThisTimeNotSave);
  connect(mSaveRightNow, &QAction::triggered, mFavModel, &FavoritesTreeModel::saveToSettings);
}

void FavoritesTreeView::setFilter(const QString& filter) {
  mFavProxyModel->setFilterString(filter);
}

bool FavoritesTreeView::onRenameDisplayRole() {
  const QModelIndexList& proxyIndexes{selectionModel()->selectedRows()};
  if (proxyIndexes.size() != 1) {
    LOG_WARN_NP("Cannot rename", "must exactly 1 row selected");
    return false;
  }
  const QModelIndex& srcIndex = mFavProxyModel->mapToSource(proxyIndexes.front());
  const QString& oldName = srcIndex.data(Qt::DisplayRole).toString();
  const QString& newName = QInputDialog::getText(nullptr,                 //
                                                 "Rename item name",      //
                                                 "just for distinguish",  //
                                                 QLineEdit::Normal,       //
                                                 oldName);
  if (newName.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel rename");
    return false;
  }
  if (newName == oldName) {
    LOG_WARN_P("Skip", "name no changes. remains[%s]", qPrintable(oldName));
    return false;
  }
  const bool renameResult{mFavModel->onRename(srcIndex, newName)};
  if (!renameResult) {
    LOG_ERR_P("Failed", "Cannot rename name[%s->%s]", qPrintable(oldName), qPrintable(newName));
    return false;
  }
  return renameResult;
}

bool FavoritesTreeView::onAddAGroup() {
  QModelIndex grpOrRootSrcIndex;
  if (!isNoSelectionOrExactlyOneGroup(&grpOrRootSrcIndex)) {
    LOG_WARN_NP("Cannot add group", "must no selection or exactly 1 group selected");
    return false;
  }

  const QString& grpName = QInputDialog::getText(nullptr, "Input a group name", "just for distinguish");
  if (grpName.isEmpty()) {
    LOG_INFO_NP("Skip", "User cancel");
    return false;
  }
  QStandardItem* grpItem = mFavModel->addGroup(grpName, grpOrRootSrcIndex);
  bool addResult{grpItem != nullptr};
  LOG_OE_NP(addResult, "Add a group", grpName);
  return addResult;
}

int FavoritesTreeView::onRemoveSelection() {
  if (!selectionModel()->hasSelection()) {
    LOG_INFO_NP("Cancel remove", "no row selected");
    return 0;
  }
  const QModelIndexList& proxyIndexes{selectionModel()->selectedRows()};
  QModelIndexList srcIndexes;
  srcIndexes.reserve(proxyIndexes.size());
  for (const QModelIndex& proInd : proxyIndexes) {
    srcIndexes.push_back(mFavProxyModel->mapToSource(proInd));
  }
  const int cnt{mFavModel->removeParentIndexes(srcIndexes)};
  const bool bAllSucceed{cnt > 0};
  LOG_OE_P(bAllSucceed, "Remove rows", "%d/%d succeed(redundant selection may exist)", cnt, srcIndexes.size());
  return cnt;
}

bool FavoritesTreeView::onItemClicked(const QModelIndex& proIndex) {
  if (!proIndex.isValid()) {
    return false;
  }
  const QModelIndex& srcInd{mFavProxyModel->mapToSource(proIndex)};
  if (mFavModel->isGroup(srcInd)) {
    return false;
  }
  QString filePath = mFavModel->filePath(srcInd);
  emit reqIntoAPath(filePath, true);
  return true;
}

void FavoritesTreeView::onSortRoleActionTriggered(const QAction* newSortRoleAction) {
  FavoriteItemData::Role newSortRole = mSortRoleIntAction.act2Enum(newSortRoleAction);
  mFavProxyModel->setSortRole(newSortRole);
}

bool FavoritesTreeView::isExactlyOneGroupSelected(QModelIndex* grpSrcIndex) const {
  const QModelIndexList& proxyIndexes{selectionModel()->selectedRows()};
  if (proxyIndexes.size() != 1) {
    return false;
  }
  const QModelIndex proInd{proxyIndexes.front()};
  if (!proInd.isValid()) {
    return false;
  }
  QModelIndex grpInd{mFavProxyModel->mapToSource(proInd)};
  if (!mFavModel->isGroup(grpInd)) {
    return false;
  }
  if (grpSrcIndex != nullptr) {
    *grpSrcIndex = grpInd;
  }
  return true;
}

bool FavoritesTreeView::isNoSelectionOrExactlyOneGroup(QModelIndex* grpOrRootSrcIndex) const {
  if (!selectionModel()->hasSelection()) {
    if (grpOrRootSrcIndex != nullptr) {
      *grpOrRootSrcIndex = QModelIndex{};
    }
    return true;
  }
  return isExactlyOneGroupSelected(grpOrRootSrcIndex);
}

void FavoritesTreeView::dragEnterEvent(QDragEnterEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData* mimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(mimeData);
  if (mimeData->hasFormat(FavoritesTreeModel::MIME_TYPE) || mimeData->hasUrls()) {
    event->acceptProposedAction();
    return;
  }
  QTreeView::dragEnterEvent(event);
}

void FavoritesTreeView::dragMoveEvent(QDragMoveEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  const QMimeData* mimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(mimeData);
  if (mimeData->hasFormat(FavoritesTreeModel::MIME_TYPE) || mimeData->hasUrls()) {
    QModelIndex proxyIndex = indexAt(event->pos());
    QModelIndex srcIndex = mFavProxyModel->mapToSource(proxyIndex);
    if (mFavModel->canDropOn(srcIndex)) {
      event->acceptProposedAction();
    } else {
      event->ignore();
    }
    return;
  }
  QTreeView::dragMoveEvent(event);
}

void FavoritesTreeView::dropEvent(QDropEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  QTreeView::dropEvent(event);
  if (event->isAccepted()) {
    QModelIndex proxyIndex = indexAt(event->pos());
    if (proxyIndex.isValid()) {
      expand(proxyIndex);
    }
  }
}
