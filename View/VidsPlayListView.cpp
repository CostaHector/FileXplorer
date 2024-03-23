#include "VidsPlayListView.h"
#include "Actions/VideoPlayerActions.h"
#include "Component/NotificatorFrame.h"
#include "FileOperation/FileOperation.h"
#include "Tools/PathTool.h"
#include "UndoRedo.h"
#include "View/ViewHelper.h"
#include "View/ViewStyleSheet.h"

#include <QDesktopServices>

VidsPlayListView::VidsPlayListView(VidModel* model_, QWidget* parent)
    : QListView{parent}, m_vidModel{model_}, m_vidMenu{new QMenu("playList", this)} {
  setModel(m_vidModel);

  m_vidMenu->addAction(g_videoPlayerActions()._REVEAL_IN_EXPLORER);
  m_vidMenu->addAction(g_videoPlayerActions()._MOVE_SELECTED_ITEMS_TO_TRASHBIN);

  setSelectionMode(QAbstractItemView::ExtendedSelection);
  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::NoDragDrop);
  setAcceptDrops(false);
  setDragEnabled(false);
  setDropIndicatorShown(false);

  subscribe();

  setStyleSheet(ViewStyleSheet::LISTVIEW_STYLESHEET);
}

void VidsPlayListView::subscribe() {
  connect(g_videoPlayerActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this, &VidsPlayListView::onRevealInSystemExplorer);
  connect(g_videoPlayerActions()._UPDATE_ITEM_PLAYABLE, &QAction::triggered, m_vidModel, &VidModel::updatePlayableForeground);
  connect(g_videoPlayerActions()._MOVE_SELECTED_ITEMS_TO_TRASHBIN, &QAction::triggered, this, &VidsPlayListView::onRecycleSelectedItems);
}

auto VidsPlayListView::InitViewSettings() -> void {
  setAlternatingRowColors(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);

  sizeHintForRow(ViewStyleSheet::ROW_SECTION_HEIGHT);
  View::UpdateItemViewFontSizeCore(this);
}

void VidsPlayListView::contextMenuEvent(QContextMenuEvent* event) {
  m_vidMenu->popup(viewport()->mapToGlobal(event->pos()));  // or QCursor::pos()
  QListView::contextMenuEvent(event);
}

void VidsPlayListView::onRevealInSystemExplorer() {
  if (not currentIndex().isValid()) {
    qInfo("Cannot reveal. nothing selected");
    Notificator::information("Cannot reveal", "nothing selected");
    return;
  }
  const QString& filePath = currentFilePath();
  const QString& dirPath = PATHTOOL::absolutePath(filePath);
  if (not QFile::exists(filePath)) {
    qInfo("Cannot reveal. dirpath[%s] not exists", qPrintable(dirPath));
    Notificator::information("Cannot reveal", QString("dirpath[%1] not exists").arg(dirPath));
    return;
  }
  const QString& dirUrl = PATHTOOL::linkPath(dirPath);
  QDesktopServices::openUrl(dirUrl);
}

int VidsPlayListView::appendToPlayList(const QStringList& fileAbsPathList) {
  QStringList validList;
  validList.reserve(fileAbsPathList.size());
  for (const auto& fileAbsPath : fileAbsPathList) {
    QFileInfo fi(fileAbsPath);
    if (not fi.isFile()) {
      continue;
    }
    if (not TYPE_FILTER::VIDEO_TYPE_SET.contains("*." + fi.suffix())) {
      continue;
    }
    validList.append(fileAbsPath);
  }
  return m_vidModel->appendRows(validList);
}

int VidsPlayListView::onRecycleSelectedItems() {
  if (not selectionModel()->hasSelection()) {
    return 0;
  }
  FileOperation::BATCH_COMMAND_LIST_TYPE recycleCmds;
  for (const auto& ind : selectionModel()->selectedIndexes()) {
    QFileInfo fi(filePath(ind));
    if (fi.exists()) {
      recycleCmds.append({"moveToTrash", fi.absolutePath(), fi.fileName()});
    }
  }
  bool recycleRet = g_undoRedo.Do(recycleCmds);
  if (recycleRet) {
    qDebug("Recycle succeed. %d files", recycleCmds.size());
    Notificator::information("Recycle succeed", QString("%1 files").arg(recycleCmds.size()));
  } else {
    qWarning("Some recycle failed. %d files", recycleCmds.size());
    Notificator::warning("Some Recycle Failed", QString("%1 files").arg(recycleCmds.size()));
  }
  m_vidModel->whenFilesDeleted(selectionModel()->selection());
  return recycleCmds.size();
}
