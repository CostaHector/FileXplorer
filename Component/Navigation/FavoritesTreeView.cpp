#include "FavoritesTreeView.h"
#include "MemoryKey.h"
#include "NotificatorMacro.h"
#include "PublicMacro.h"
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMimeData>

FavoritesTreeView::FavoritesTreeView(const QString& name, QWidget* parent) : QTreeView{parent}, m_name{name} {
  setObjectName("FavoritesTreeView");
  setIndentation(12);
  mFavModel = new FavoritesTreeModel{GetName(), this};

  mFavProxyModel = new RecursiveFilterProxyTreeModel{this};
  mFavProxyModel->setSourceModel(mFavModel);
  setModel(mFavProxyModel);

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  setDragDropMode(QAbstractItemView::DragDrop);
  setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setExpandsOnDoubleClick(true);
  setSortingEnabled(true);

  const int fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(font());
  defaultFont.setPointSize(fontSize);
  setFont(defaultFont);

  mMenu = new QMenu{"Favorites Menu", this};

  // Expand all
  mExpandAll = mMenu->addAction(QIcon{":img/EXPAND_ALL"}, tr("Expand All"));
  mExpandAll->setToolTip("Expand all groups to show their contents");

  // Collapse all
  mCollapseAll = mMenu->addAction(QIcon{":img/COLLAPSE_ALL"}, tr("Collapse All"));
  mCollapseAll->setToolTip("Collapse all groups to hide their contents");

  // Animated
  mAnimatedEnableAct = mMenu->addAction(tr("Enable Animation"));
  mAnimatedEnableAct->setToolTip("Enable or disable smooth expand/collapse animations");
  {
    const bool bDefAnimated{false};
    mAnimatedEnableAct->setCheckable(true);
    mAnimatedEnableAct->setChecked(bDefAnimated);
    setAnimated(bDefAnimated);
  }

  mHeaderHidden = mMenu->addAction(tr("Hide Header"));
  {
    const bool bHiddenHeader{false};
    mHeaderHidden->setCheckable(true);
    mHeaderHidden->setChecked(bHiddenHeader);
    setHeaderHidden(bHiddenHeader);
  }

  mRootDecorationEnabled = mMenu->addAction(tr("Root Decoration"));
  {
    const bool bShowRootDecoration{true};
    mRootDecorationEnabled->setCheckable(true);
    mRootDecorationEnabled->setChecked(bShowRootDecoration);
    setRootIsDecorated(bShowRootDecoration);
  }

  mMenu->addSeparator();

  {
    mSortRoleMenu = mMenu->addMenu(tr("Sort"));
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
    mSortReverse = mMenu->addAction(tr("Sort Reverse"));
    mSortReverse->setCheckable(true);
    mSortReverse->setChecked(isSortReverse);

    mFavProxyModel->initSortProxy(initMemoryRole, isSortReverse);
  }

  mMenu->addSeparator();

  // Rename
  mRenameDisplayRole = mMenu->addAction(QIcon{":img/RENAME"}, tr("Rename"));
  mRenameDisplayRole->setToolTip("Rename the selected item");

  // Add group
  mAddAGroup = mMenu->addAction(QIcon{":img/WHERE_CLAUSE_HISTORY_ADD"}, tr("Add Group"));
  mAddAGroup->setToolTip("Add a new group to organize favorites");

  // Remove selection
  mRemoveSelection = mMenu->addAction(QIcon{":img/WHERE_CLAUSE_HISTORY_REMOVED"}, tr("Delete Selected"));
  mRemoveSelection->setToolTip("Remove the selected items");

  mMenu->addSeparator();

  // Add initial examples
  mAddInitialExamples = mMenu->addAction(tr("Add Initial Examples"));

  // Skip saving
  mNotSavedDatasThisTime = mMenu->addAction(tr("Skip Saving"));
  mNotSavedDatasThisTime->setCheckable(true);
  mNotSavedDatasThisTime->setChecked(false);
  mNotSavedDatasThisTime->setToolTip("Skip saving changes on exit (temporary mode). by default not skip");

  // Save immediately
  mSaveRightNow = mMenu->addAction(QIcon{":/JsonEditor/SAVE_CHANGES"}, tr("Save Now"));
  mSaveRightNow->setToolTip("Save all changes immediately");

  connect(this, &QTreeView::clicked, this, &FavoritesTreeView::onItemClicked);
  connect(mExpandAll, &QAction::triggered, this, &FavoritesTreeView::expandAll);
  connect(mCollapseAll, &QAction::triggered, this, &FavoritesTreeView::collapseAll);

  connect(mAnimatedEnableAct, &QAction::toggled, this, &FavoritesTreeView::setAnimated);
  connect(mHeaderHidden, &QAction::toggled, this, &FavoritesTreeView::setHeaderHidden);
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

FavoritesTreeView::~FavoritesTreeView() {
  FavoriteItemData::SaveInitialSortRole(mSortRoleIntAction.curVal());
  FavoriteItemData::SaveSortOrderReverse(mSortReverse->isChecked());
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

void FavoritesTreeView::contextMenuEvent(QContextMenuEvent* event) {
  CHECK_NULLPTR_RETURN_VOID(event);
  CHECK_NULLPTR_RETURN_VOID(mMenu);
  if (mMenu != nullptr) {
#ifndef RUNNING_UNIT_TESTS
    mMenu->popup(event->globalPos());
#endif
  }
  event->accept();
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
