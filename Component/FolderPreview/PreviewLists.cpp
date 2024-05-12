#include "PreviewLists.h"

#include "FolderPreviewComponent/ImagesListPreview.h"
#include "FolderPreviewComponent/OtherItemsListPreview.h"
#include "FolderPreviewComponent/VideosListPreview.h"
#include "qevent.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <QDesktopServices>
#include <QUrl>

#include <QMenu>

#include <QContextMenuEvent>

PreviewLists::PreviewLists(QWidget* parent)
    : QWidget(parent),
      m_parentDocker{parent},
      m_vidsPreview(new VideosListPreview),
      m_imgsPreview(new ImagesListPreview),
      m_othersPreview(new OtherItemsListPreview),
      m_folderPreviewMenu(new QMenu) {
  QVBoxLayout* pre(new QVBoxLayout);
  pre->setSpacing(0);
  pre->setContentsMargins(0, 0, 0, 0);

  pre->addWidget(m_vidsPreview);
  pre->addWidget(m_imgsPreview);
  pre->addWidget(m_othersPreview);

  pre->setStretch(0, 1);
  pre->setStretch(1, 95);
  pre->setStretch(2, 4);
  setLayout(pre);

  subscribe();

  m_folderPreviewMenu->addAction(m_vidsPreview->hideWidget);
  m_folderPreviewMenu->addAction(m_imgsPreview->hideWidget);
  m_folderPreviewMenu->addAction(m_othersPreview->hideWidget);
}

bool PreviewLists::operator()(const QString& path) {
  if (not QFileInfo(path).isDir()) {
    return true;
  }
  (*m_vidsPreview)(path);
  (*m_imgsPreview)(path);
  (*m_othersPreview)(path);
  return true;
}

void PreviewLists::subscribe() {
  if (m_parentDocker == nullptr) {
    qDebug("will neither record imgs/vids count nor update docker title");
    return;
  }

  connect(dynamic_cast<QFileSystemModel*>(m_imgsPreview->model()), &QFileSystemModel::directoryLoaded, this, [this]() {
    mImgCnt = m_imgsPreview->model()->rowCount(m_imgsPreview->rootIndex());
    if (isCntOk()) {
      setDockerWindowTitle();
    }
  });
  connect(dynamic_cast<QFileSystemModel*>(m_vidsPreview->model()), &QFileSystemModel::directoryLoaded, this, [this]() {
    mVidsCnt = m_vidsPreview->model()->rowCount(m_vidsPreview->rootIndex());
    if (isCntOk()) {
      setDockerWindowTitle();
    }
  });
}

void PreviewLists::contextMenuEvent(QContextMenuEvent* event) {
  m_folderPreviewMenu->popup(mapToGlobal(event->pos()));
}

// #define __MAIN__EQ__NAME__ 1
#ifdef __MAIN__EQ__NAME__
#include <QApplication>
#include <QTableView>
int main(int argc, char* argv[]) {
  QString rootPath = QFileInfo(__FILE__).absolutePath();
  QString testDir = QDir(rootPath).absoluteFilePath("test");

  QApplication a(argc, argv);

  QTableView* tb = new QTableView;
  QFileSystemModel* fsm = new QFileSystemModel;
  tb->setModel(fsm);
  tb->setRootIndex(fsm->setRootPath(testDir));

  QHBoxLayout* lo = new QHBoxLayout;

  FolderPreviewWidget* previewer = new FolderPreviewWidget;
  lo->addWidget(tb);
  lo->addWidget(previewer);

  QTableView::connect(tb, &QTableView::doubleClicked, [previewer, fsm](QModelIndex clickedIndex) -> void {
    QFileInfo fi = fsm->fileInfo(clickedIndex);
    (*previewer)(fi.absoluteFilePath());
  });

  QWidget w;
  w.setLayout(lo);

  w.show();
  return a.exec();
}
#endif
