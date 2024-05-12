#include "OtherItemsListPreview.h"
#include "OtherItemFileSystemModel.h"

OtherItemsListPreview::OtherItemsListPreview() : FolderListView(new OtherItemFileSystemModel(nullptr), "others") {
  OtherItemsListPreview::InitViewSettings();
}

auto OtherItemsListPreview::InitViewSettings() -> void {
  setViewMode(QListView::ViewMode::ListMode);
  setFlow(QListView::Flow::LeftToRight);
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
#include <QHBoxLayout>
#include <QTableView>

#include "OtherItemFileSystemModel.h"
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

  FolderListView* previewer = new OtherItemsListPreview;
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
