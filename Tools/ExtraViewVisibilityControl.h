#ifndef EXTRAVIEWVISIBILITYCONTROL_H
#define EXTRAVIEWVISIBILITYCONTROL_H
#include <QObject>
#include <QWidget>

class ExtraViewVisibilityControl : public QObject {
 public:
  explicit ExtraViewVisibilityControl(QWidget* parent);

  auto subscribe() -> void;

 private:
  QWidget* _parent;
  QWidget* torrentsManager;
  QWidget* m_recycleBin;
};

#endif  // EXTRAVIEWVISIBILITYCONTROL_H
