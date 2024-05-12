#include "ImagesListPreview.h"
#include <QHBoxLayout>
#include "ImagesFileSystemModel.h"

ImagesListPreview::ImagesListPreview(QWidget* parent) : FolderListView(new ImagesFileSystemModel(nullptr, true), "imgs") {
  ImagesListPreview::InitViewSettings();
}

auto ImagesListPreview::InitViewSettings() -> void {
  setViewMode(QListView::ViewMode::IconMode);
  setFlow(QListView::Flow::TopToBottom);
  setTextElideMode(Qt::TextElideMode::ElideMiddle);
  setUniformItemSizes(false);

  setResizeMode(QListView::ResizeMode::Adjust);
  setMovement(QListView::Movement::Free);
  setWrapping(false);

  //    const int width=128, height=64;
  //    const int textHeight = 16;
  //    QSize gridSize(width, height);
  //    QSize iconSize(width, height - textHeight);
  //    setGridSize(gridSize);
  setIconSize(QSize{width(), width()});
  //    UpdateListIconSize(size);
}

// #define __MAIN__EQ__NAME__ 1
#ifdef __MAIN__EQ__NAME__
#include <QApplication>
#include <QTableView>
#include "ImagesFileSystemModel.h"
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);

  QTableView* tb = new QTableView;
  QFileSystemModel* fsm = new QFileSystemModel;
  tb->setModel(fsm);

  QString rootPath = QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath();
  QString testDir = QDir(rootPath).absoluteFilePath("test");

  qDebug("%s", qPrintable(testDir));
  tb->setRootIndex(fsm->setRootPath(testDir));

  QHBoxLayout* lo = new QHBoxLayout;

  FolderListView* previewer = new ImagesListPreview;
  lo->addWidget(tb);
  lo->addWidget(previewer);

  QTableView::connect(tb, &QTableView::doubleClicked, [previewer, fsm](QModelIndex clickedIndex) -> void {
    QFileInfo fi = fsm->fileInfo(clickedIndex);
    qDebug("doubleclicked [%s]", qPrintable(fi.absoluteFilePath()));
    (*previewer)(fi.absoluteFilePath());
  });

  QWidget w;
  w.setLayout(lo);

  w.show();
  return a.exec();
}
#endif
