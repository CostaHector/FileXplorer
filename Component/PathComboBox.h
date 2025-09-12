#ifndef PATHCOMBOBOX_H
#define PATHCOMBOBOX_H

#include <QComboBox>
#include <QFocusEvent>

class PathComboBox : public QComboBox {
  Q_OBJECT
public:
  using QComboBox::QComboBox;
signals:
  void focusChanged(bool hasFocus);
protected:
  void focusInEvent(QFocusEvent* event) override;
  void focusOutEvent(QFocusEvent* event) override;
};

#endif
