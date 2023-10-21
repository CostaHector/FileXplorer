#include "CustomStatusBar.h"

#include <QMainWindow>
class CustomStatusBarIll : public QMainWindow {
 public:
  CustomStatusBar* statusBar;
  explicit CustomStatusBarIll(QWidget* parent = nullptr) : QMainWindow(parent), statusBar(new CustomStatusBar) {
    setStatusBar(statusBar);
    statusBar->pathInfo(1, 0);
    statusBar->pathInfo(255, 1);
    statusBar->msg("Process Finished");
  }
};

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  CustomStatusBarIll wid;
  wid.show();
  return a.exec();
}
#endif
