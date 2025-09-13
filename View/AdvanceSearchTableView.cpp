#include "AdvanceSearchTableView.h"
#include "FileBasicOperationsActions.h"
#include "ViewActions.h"

AdvanceSearchTableView::AdvanceSearchTableView(AdvanceSearchModel* sourceModel, SearchProxyModel* searchProxyModel, QWidget* parent)
  : CustomTableView{"ADVANCE_SEARCH_SYSTEM", parent},
  _sourceModel(sourceModel),
  _searchProxyModel(searchProxyModel) {

  _searchProxyModel->setSourceModel(_sourceModel);
  setModel(_searchProxyModel);
  setDragDropMode(QAbstractItemView::DragDrop);
  setAcceptDrops(true);
  setDragEnabled(true);
  setDropIndicatorShown(true);
  setSortingEnabled(true);

  subscribe();

  setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  InitTableView();
}

void AdvanceSearchTableView::subscribe() {
  auto& fileOpInst = g_fileBasicOperationsActions();
  connect(fileOpInst._FORCE_RESEARCH, &QAction::triggered, _sourceModel, &AdvanceSearchModel::forceRefresh);

  addAction(g_viewActions()._SYS_VIDEO_PLAYERS);
  addActions(fileOpInst.OPEN_AG->actions());
  addActions(fileOpInst.COPY_PATH_AG->actions());
  addActions(fileOpInst.CUT_COPY_PASTE->actions());
  addActions(fileOpInst.DELETE_ACTIONS->actions());
}

void AdvanceSearchTableView::keyPressEvent(QKeyEvent* e) {
  if (e->modifiers() == Qt::KeyboardModifier::NoModifier && e->key() == Qt::Key_Delete) {
    emit g_fileBasicOperationsActions().MOVE_TO_TRASHBIN->triggered();
    return;
  }
  QTableView::keyPressEvent(e);
}
