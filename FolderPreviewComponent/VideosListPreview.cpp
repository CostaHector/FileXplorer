#include "VideosListPreview.h"

#include <QHBoxLayout>
#include "VideosFileSystemModel.h"

VideosListPreview::VideosListPreview() : FolderListView(new VideosFileSystemModel(nullptr, false), "vids") {
  VideosListPreview::InitViewSettings();
}

auto VideosListPreview::InitViewSettings() -> void {
  setViewMode(QListView::ViewMode::ListMode);
  setFlow(QListView::Flow::TopToBottom);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setUniformItemSizes(false);

  setResizeMode(QListView::ResizeMode::Adjust);
  setMovement(QListView::Movement::Free);
  setWrapping(true);

  //    const int width=128, height=64;
  //    const int textHeight = 16;
  //    QSize gridSize(width, height);
  //    QSize iconSize(width, height - textHeight);
  //    setGridSize(gridSize);
  //    setIconSize(iconSize);
  //    UpdateListIconSize(size);
}

// #define __MAIN__EQ__NAME__ 1
#ifdef __MAIN__EQ__NAME__
#include <QApplication>
#include <QTableView>
#include "VideosFileSystemModel.h"
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);

  QTableView* tb = new QTableView;
  QFileSystemModel* fsm = new QFileSystemModel;
  tb->setModel(fsm);

  QString rootPath = QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath();
  QString testDir = QDir(rootPath).absoluteFilePath("test");

  qDebug("%s", testDir.toStdString().c_str());
  tb->setRootIndex(fsm->setRootPath(testDir));

  QHBoxLayout* lo = new QHBoxLayout;

  FolderListView* previewer = new VideosListPreview;
  lo->addWidget(tb);
  lo->addWidget(previewer);

  QTableView::connect(tb, &QTableView::doubleClicked, [previewer, fsm](QModelIndex clickedIndex) -> void {
    QFileInfo fi = fsm->fileInfo(clickedIndex);
    qDebug("doubleclicked [%s]", fi.absoluteFilePath().toStdString().c_str());
    (*previewer)(fi.absoluteFilePath());
  });

  QWidget w;
  w.setLayout(lo);

  w.show();
  return a.exec();
}
#endif
