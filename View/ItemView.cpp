#include "ItemView.h"
#include "PublicMacro.h"
#include "FileTool.h"
#include "NotificatorMacro.h"
#include "UndoRedo.h"
#include <QFile>

ItemView::ItemView(const QString& itemViewName, QWidget* parent) //
  : CustomListView{itemViewName, parent} {
  setMovement(QListView::Movement::Free);

  _PLAY_ITEM = new (std::nothrow) QAction{QIcon{":img/OPEN_IN_TERMINAL"}, "Open", this};
  _RECYCLE_ITEM = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, "Recycle", this};
  m_menu->addAction(_PLAY_ITEM);
  m_menu->addAction(_RECYCLE_ITEM);
  AddItselfAction2Menu();

  subscribe();
}

void ItemView::subscribe() {
  connect(this, &QListView::doubleClicked, this, &ItemView::onCellDoubleClicked);
  connect(_PLAY_ITEM, &QAction::triggered, this, [this]() { onCellDoubleClicked(currentIndex()); });
  connect(_RECYCLE_ITEM, &QAction::triggered, this, &ItemView::onRecycleSelections);
}

bool ItemView::onCellDoubleClicked(const QModelIndex& clickedIndex) const {
  CHECK_NULLPTR_RETURN_FALSE(mModels);
  if (!clickedIndex.isValid()) {
    return false;
  }
  const QString& path = mModels->filePath(clickedIndex);
  if (!QFile::exists(path)) {
    LOG_INFO_P("Cannot open", "Path[%s] not exists", qPrintable(path));
    return false;
  }
#ifdef RUNNING_UNIT_TESTS
  return true;
#endif
  return FileTool::OpenLocalFile(path);
}

bool ItemView::onRecycleSelections() const {
  CHECK_NULLPTR_RETURN_FALSE(mModels);
  const QModelIndexList& inds{selectionModel()->selectedIndexes()};
  if (inds.isEmpty()) {
    LOG_INFO_NP("Skip Recyle", "nothing selected");
    return true;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(inds.size());
  for (const QModelIndex& ind: inds) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH("", mModels->filePath(ind)));
  }
  if (!UndoRedo::GetInst().Do(removeCmds)) {
    LOG_ERR_NP("[MoveToTrash] Partially failed", "Some item(s) move to trashbin failed");
    return false;
  }
  LOG_OK_P("[MoveToTrash] ok", "%d item(s) recycled succeed", inds.size());
  return true;
}
