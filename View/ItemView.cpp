#include "ItemView.h"
#include "PublicMacro.h"
#include "FileTool.h"
#include "NotificatorMacro.h"
#include "UndoRedo.h"
#include "BatchRenameBy.h"
#include <QFile>

ItemView::ItemView(const QString& itemViewName, QWidget* parent)  //
    : CustomListView{itemViewName, parent} {
  setMovement(QListView::Movement::Free);

  _PLAY_ITEM = new (std::nothrow) QAction{tr("Open"), this};
  _RENAME_SCENE_RELATED_FILES_NUMERIZE = new (std::nothrow) QAction(QIcon(":img/NAME_STR_NUMERIZER_PATH"), tr("Rename (ith)"), this);
  _RENAME_SCENE_RELATED_FILES_NUMERIZE->setShortcutVisibleInContextMenu(true);
  _RENAME_SCENE_RELATED_FILES_NUMERIZE->setToolTip(QString("<b>%1 (%2)</b><br/> Numerizer each file in a sequence.")
                                                       .arg(_RENAME_SCENE_RELATED_FILES_NUMERIZE->text())
                                                       .arg(_RENAME_SCENE_RELATED_FILES_NUMERIZE->shortcut().toString()));
  _RECYCLE_ITEM = new (std::nothrow) QAction{QIcon{":img/MOVE_TO_TRASH_BIN"}, tr("Recycle"), this};
  QList<QAction*> exclusiveActions{_RENAME_SCENE_RELATED_FILES_NUMERIZE, _PLAY_ITEM, _RECYCLE_ITEM};
  PushFrontExclusiveActions(exclusiveActions);
  subscribe();
}

bool ItemView::SetCurrentModel(FloatingModels* mdl) {
  CHECK_NULLPTR_RETURN_FALSE(mdl);
  if (mModels != nullptr) {
    LOG_W("Cannot rebind again");
    return false;
  }
  setModel(mdl);
  mModels = mdl;
  PushBackExclusiveActions(mdl->GetExcusiveActions());
  return true;
}

void ItemView::subscribe() {
  connect(this, &QListView::doubleClicked, this, &ItemView::onCellDoubleClicked);
  connect(_PLAY_ITEM, &QAction::triggered, this, &ItemView::onPlayCurrentIndex);
  connect(_RENAME_SCENE_RELATED_FILES_NUMERIZE, &QAction::triggered, this, &ItemView::onRenameSelectedItems);
  connect(_RECYCLE_ITEM, &QAction::triggered, this, &ItemView::onRecycleSelections);
}

void ItemView::initExclusivePreferenceSetting() {
  if (CustomListView::m_name.contains("img", Qt::CaseSensitivity::CaseInsensitive) ||
      CustomListView::m_name.contains("image", Qt::CaseSensitivity::CaseInsensitive)) {
    CustomListView::m_defaultFlowLeft2Right = true;
    CustomListView::m_defaultViewModeIcon = true;
  } else {
    CustomListView::m_defaultFlowLeft2Right = false;
    CustomListView::m_defaultViewModeIcon = false;
  }
  CustomListView::m_defaultWrapping = true;
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
  return FileTool::OpenLocalFile(path);
}

bool ItemView::onPlayCurrentIndex() const {
  const QModelIndex curIndex = currentIndex();
  if (!curIndex.isValid()) {
    return false;
  }
  return onCellDoubleClicked(curIndex);
}

bool ItemView::onRecycleSelections() const {
  CHECK_NULLPTR_RETURN_FALSE(mModels);
  const QModelIndexList& inds{selectionModel()->selectedIndexes()};
  if (inds.isEmpty()) {
    LOG_INFO_NP("Skip Recyle", "nothing selected");
    return false;
  }
  using namespace FileOperatorType;
  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(inds.size());
  for (const QModelIndex& ind : inds) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH("", mModels->filePath(ind)));
  }
  if (!UndoRedo::GetInst().Do(removeCmds)) {
    LOG_ERR_NP("[MoveToTrash] Partially failed", "Some item(s) move to trashbin failed");
    return false;
  }
  LOG_OK_P("[MoveToTrash] ok", "%d item(s) recycled succeed", inds.size());
  return true;
}

int ItemView::onRenameSelectedItems() {
  const QModelIndexList& srcIndexes{selectionModel()->selectedRows()};
  if (srcIndexes.isEmpty()) {
    LOG_INFO_NP("Skip rename(numerize)", "no row selected");
    return 0;
  }

  const QString& localPath{mModels->rootPath()};
  const QStringList& fileNamesSelected{mModels->rel2fileNames(srcIndexes)};
  const BatchRenameBy::RnmResult rnmResult{BatchRenameBy::NumerizerQueryAndConfirm(localPath, fileNamesSelected)};
  const int removeRowCnt{mModels->AfterRowsRemoved(srcIndexes)};
  return fileNamesSelected.size();
}
