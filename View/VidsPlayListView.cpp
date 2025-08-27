#include "VidsPlayListView.h"
#include "VideoPlayerActions.h"
#include "NotificatorMacro.h"

#include "PublicVariable.h"
#include "PathTool.h"

#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QFileInfo>

VidsPlayListView::VidsPlayListView(VidModel* model_, QWidget* parent)
    : CustomListView{"VID_PLAYLIST_VIEW", parent}, m_vidModel{model_}, m_vidMenu{new QMenu("playList", this)} {
  setModel(m_vidModel);
  BindMenu(m_vidMenu);

  m_vidMenu->addAction(g_videoPlayerActions()._REVEAL_IN_EXPLORER);
  m_vidMenu->addAction(g_videoPlayerActions()._MOVE_SELECTED_ITEMS_TO_TRASHBIN);
  m_vidMenu->addAction(g_videoPlayerActions()._RENAME_VIDEO);

  setEditTriggers(QAbstractItemView::NoEditTriggers);  // only F2 works. QAbstractItemView.NoEditTriggers
  setDragDropMode(QAbstractItemView::NoDragDrop);

  subscribe();
}

void VidsPlayListView::subscribe() {
  connect(g_videoPlayerActions()._REVEAL_IN_EXPLORER, &QAction::triggered, this, &VidsPlayListView::onRevealInSystemExplorer);
  connect(g_videoPlayerActions()._UPDATE_ITEM_PLAYABLE, &QAction::triggered, m_vidModel, &VidModel::updatePlayableForeground);
}

void VidsPlayListView::onRevealInSystemExplorer() {
  if (!currentIndex().isValid()) {
    LOG_INFO_NP("Cannot reveal", "nothing selected");
    return;
  }
  const QString& filePath = currentFilePath();
  const QString& dirPath = PathTool::absolutePath(filePath);
  if (!QFile::exists(filePath)) {
    LOG_INFO_P("Cannot reveal", "dirpath[%s] not exists", qPrintable(dirPath));
    return;
  }
  const QString& dirUrl = PathTool::linkPath(dirPath);
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
