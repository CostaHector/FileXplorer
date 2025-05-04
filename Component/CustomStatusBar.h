#ifndef CUSTOMSTATUSBAR_H
#define CUSTOMSTATUSBAR_H

#include <QLabel>
#include <QProgressBar>
#include <QStatusBar>
#include <QToolBar>

enum class STATUS_STR_TYPE { NORMAL = 0, ABNORMAL = 1 };

class CustomStatusBar : public QStatusBar {
 public:
  enum MSG_INDEX {
    BEGIN,
    ITEMS = BEGIN,
    SELECTED,
    MSG,
    BUTT
  };
  explicit CustomStatusBar(QToolBar* views, QWidget* parent = nullptr);

  void pathInfo(const int count, const int index = 0);
  void msg(const QString& text = "", const STATUS_STR_TYPE statusStrType = STATUS_STR_TYPE::NORMAL);
  void SetProgressValue(int value = 100);

  static int GetValidProgressValue(int value);

 private:
  QToolBar* m_viewsSwitcher_;
  QProgressBar* mProcess {nullptr};
  QList<QLabel*> mLabelsLst;
};
#endif  // CUSTOMSTATUSBAR_H
