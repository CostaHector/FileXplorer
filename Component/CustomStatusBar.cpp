#include "CustomStatusBar.h"
#include "public/PublicMacro.h"

CustomStatusBar::CustomStatusBar(QToolBar* views, QWidget* parent)  //
    : QStatusBar{parent},                                           //
      m_viewsSwitcher_{views} {
  mProcess = new (std::nothrow) QProgressBar{this};
  CHECK_NULLPTR_RETURN_VOID(mProcess);
  mProcess->setRange(0, 100);
  mProcess->setValue(0);

  for (int labelIndex = ITEMS; labelIndex < BUTT; ++labelIndex) {
    auto* p = new QLabel{"", parent};
    CHECK_NULLPTR_RETURN_VOID(p);
    mLabelsLst << p;
    const int stretch{labelIndex == MSG ? 1 : 0};
    addPermanentWidget(p, stretch);  // start=1, dev=0
  }

  addPermanentWidget(mProcess);
  addPermanentWidget(m_viewsSwitcher_);  // -1
  setContentsMargins(0, 0, 0, 0);
}

void CustomStatusBar::pathInfo(const int count, const int index) {
  if (index == ITEMS) {
    mLabelsLst[ITEMS]->setText(QString("Total %1 item(s) |").arg(count));
  } else if (index == SELECTED) {
    mLabelsLst[SELECTED]->setText(QString("%1 selected |").arg(count));
  }
}

void CustomStatusBar::msg(const QString& text, const STATUS_STR_TYPE statusStrType) {
  if (statusStrType == STATUS_STR_TYPE::ABNORMAL) {  // abnormal
    mLabelsLst[MSG]->setStyleSheet("QLabel{color:red;font-weight:bold;}");
    qWarning("%s", qPrintable(text));
  } else {  // normal;
    mLabelsLst[MSG]->setStyleSheet("");
    qDebug("%s", qPrintable(text));
  }
  mLabelsLst[MSG]->setText(text);
}

void CustomStatusBar::SetProgressValue(int value) {
  if (mProcess == nullptr) {
    qCritical("mProcess is nullptr");
    return;
  }
  mProcess->setValue(GetValidProgressValue(value));
}

int CustomStatusBar::GetValidProgressValue(int value) {
  return (value > 100) ? 100               //
                       : ((value < 0) ? 0  //
                                      : value);
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
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
    setWindowTitle(QFileInfo(__FILE__).absoluteFilePath());  // PROJECT_NAME/
  }
};

#include <QApplication>
int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  CustomStatusBarIll wid;
  wid.show();
  return a.exec();
}
#endif
