#include "RightClickMenu.h"
#include <QFileSystemModel>
#include <QListView>

class RightClickMenuIll : public QListView {
 public:
  explicit RightClickMenuIll(const QString& path) : QListView() {
    setWindowTitle("TestRightClickMenu");
    auto* fileSysModel = new QFileSystemModel();
    fileSysModel->setRootPath(path);
    fileSysModel->setReadOnly(false);

    setModel(fileSysModel);
    setRootIndex(fileSysModel->index(path));
    auto* menu = new RightClickMenu("Right click menu", this);

    setContextMenuPolicy(Qt::CustomContextMenu);

    auto ShowContextMenu = [this, menu](const QPoint pnt) {
      menu->popup(this->mapToGlobal(pnt));  // or QCursor::pos()
    };
    connect(this, &QListView::customContextMenuRequested, ShowContextMenu);
    show();
  }
  auto sizeHint() const -> QSize override { return QSize(600, 400); }
};

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  RightClickMenuIll rightClickMenuIll(QFileInfo(__FILE__).absolutePath());
  rightClickMenuIll.show();
  return a.exec();
}
#endif
