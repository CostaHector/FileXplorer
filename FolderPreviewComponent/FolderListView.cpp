#include "FolderListView.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QHBoxLayout>
#include <QMenu>
#include <QUrl>

#include "PublicVariable.h"

FolderListView::FolderListView(MyQFileSystemModel* fileSystemModel_, const QString& viewName_)
    : hideWidget(new QAction(viewName_)), m_fileSystemPreview(fileSystemModel_), m_listViewMenu(new QMenu), m_viewName(viewName_) {
  PreferenceSettings().beginGroup("ShowFolderPreview");
  bool checked = PreferenceSettings().value(m_viewName, true).toBool();
  if (not checked)
    setVisible(checked);
  PreferenceSettings().endGroup();

  hideWidget->setCheckable(true);
  hideWidget->setChecked(checked);

  m_listViewMenu->addAction(hideWidget);

  setModel(m_fileSystemPreview);
  subscribe();
}

bool FolderListView::operator()(const QString& path) {
  if (not QFileInfo(path).isDir()) {
    qDebug("path[%s] not exist", qPrintable(path));
    return true;
  }
  setRootIndex(m_fileSystemPreview->setRootPath(path));
  InitViewSettings();
  return true;
}

void FolderListView::subscribe() {
  connect(this, &QListView::doubleClicked, this, [this](QModelIndex clickedIndex) -> bool {
    if (not clickedIndex.isValid()) {
      return false;
    }
    const QFileInfo fi = m_fileSystemPreview->fileInfo(clickedIndex);
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
  });

  connect(hideWidget, &QAction::triggered, this, [this](const bool triggered) -> void {
    PreferenceSettings().setValue(QString("ShowFolderPreview/%0").arg(m_viewName), hideWidget->isChecked());
    setVisible(triggered);
  });
}

void FolderListView::contextMenuEvent(QContextMenuEvent* event) {
  m_listViewMenu->popup(mapToGlobal(event->pos()));  // or QCursor::pos()
}
