#ifndef CUSTOMSTATUSBAR_H
#define CUSTOMSTATUSBAR_H

#include <QLabel>
#include <QStatusBar>
#include <QToolBar>

enum class STATUS_ALERT_LEVEL { NORMAL = 0, ABNORMAL = 1 };

class CustomStatusBar : public QStatusBar {
  Q_OBJECT
public:
  enum MSG_INDEX {
    BEGIN,
    ITEMS = BEGIN,
    SELECTED,
    MSG,
    BUTT
  };
  explicit CustomStatusBar(QWidget* parent = nullptr);

  void onPathInfoChanged(const int count, const int index = 0);
  void onMsgChanged(const QString& text = "", const STATUS_ALERT_LEVEL alertLvl = STATUS_ALERT_LEVEL::NORMAL);

  QToolBar* m_viewsSwitcher{nullptr};
private:
  QList<QLabel*> mLabelsLst; // total count, selected count, message
};
#endif  // CUSTOMSTATUSBAR_H
