#include "CustomStatusBar.h"
#include "PublicVariable.h"

CustomStatusBar::CustomStatusBar(QToolBar* views, QWidget* parent)
    : QStatusBar(parent),
      _views(views),
      process(new QProgressBar),
      labelLst{new QLabel("items"), new QLabel("selected"), new QLabel("")},
      m_clickMe{new QLabel("click me")} {
  process->setRange(0, 100);
  process->setValue(0);

  _views->setIconSize(QSize(CustomStatusBar::STATUS_BAR_ICON_SIZE, CustomStatusBar::STATUS_BAR_ICON_SIZE));
#ifdef _WIN32
  QString logPrePath = PreferenceSettings().value(MemoryKey::WIN32_RUNLOG.name).toString();
#else
  QString logPrePath = PreferenceSettings().value(MemoryKey::LINUX_RUNLOG.name).toString();
#endif
  m_clickMe->setText(QString("<a href=\"file:///%1\">click me</a>").arg(logPrePath));
  m_clickMe->setToolTip("click me to see the logs. under path:\n" + logPrePath);
  m_clickMe->setOpenExternalLinks(true);

          // [QSizeGrip, Here is 1st~(n-1)th Widget, QHBoxLayout, here is nth widget];
  addPermanentWidget(labelLst[0]);     // start=1, dev=0
  addPermanentWidget(labelLst[1]);     // start=1, dev=1
  addPermanentWidget(labelLst[2], 1);  // 1+3
  addPermanentWidget(m_clickMe);
  addPermanentWidget(process);
  addPermanentWidget(_views);  // -1
  setContentsMargins(0, 0, 0, 0);
}

#include <QMainWindow>
#include <QFileInfo>
class CustomStatusBarIll : public QMainWindow {
 public:
  CustomStatusBar* statusBar;
  explicit CustomStatusBarIll(QWidget* parent = nullptr) : QMainWindow(parent), statusBar{new CustomStatusBar(new QToolBar("test"))} {
    setStatusBar(statusBar);
    statusBar->pathInfo(1, 0);
    statusBar->pathInfo(255, 1);
    statusBar->msg("Process Finished");
    setWindowTitle(QFileInfo(__FILE__).absoluteFilePath()); // PROJECT_NAME/
  }
};

//#define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  CustomStatusBarIll wid;
  wid.show();
  return a.exec();
}
#endif

