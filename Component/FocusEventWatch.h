#ifndef FOCUSEVENTWATCH_H
#define FOCUSEVENTWATCH_H

#include <QObject>
#include <QEvent>

class FocusEventWatch : public QObject {
  Q_OBJECT
 public:
  explicit FocusEventWatch(QObject* parent = nullptr);
  bool eventFilter(QObject* watched, QEvent* event) override;

 signals:
  void focusChanged(bool hasFocus);

 private:
  bool mouseButtonPressedBefore{false};
};

#endif
