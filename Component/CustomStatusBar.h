#ifndef CUSTOMSTATUSBAR_H
#define CUSTOMSTATUSBAR_H

#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>


enum class STATUS_STR_TYPE { NORMAL = 0, ABNORMAL = 1 };

class CustomStatusBar : public QStatusBar {
 public:
  static constexpr int STATUS_BAR_ICON_SIZE = 16;
  QToolBar* _views;
  QProgressBar* process;
  QList<QLabel*> labelLst;

  explicit CustomStatusBar(QToolBar* views, QWidget* parent = nullptr);

  auto pathInfo(const int count, const int index = 0) -> void {
    if (index == 0) {
      labelLst[0]->setText(QString("Total %1 item(s) |").arg(count));
    } else if (index == 1) {
      labelLst[1]->setText(QString("%1 selected |").arg(count));
    }
  }
  auto msg(const QString& msg = "", const STATUS_STR_TYPE statusStrType = STATUS_STR_TYPE::NORMAL) -> void {
    if (statusStrType == STATUS_STR_TYPE::ABNORMAL) {  // abnormal
      labelLst[2]->setStyleSheet("QLabel{color:red;font-weight:bold;}");
    } else {  // normal;
      labelLst[2]->setStyleSheet("");
    }
    qDebug("logger: msg[%s]", qPrintable(msg));
    labelLst[2]->setText(msg);
  }
};
#endif  // CUSTOMSTATUSBAR_H
