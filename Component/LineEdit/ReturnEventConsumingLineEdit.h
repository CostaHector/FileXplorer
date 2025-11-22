#ifndef RETURNEVENTCONSUMINGLINEEDIT_H
#define RETURNEVENTCONSUMINGLINEEDIT_H

#include <QLineEdit>
class ReturnEventConsumingLineEdit : public QLineEdit {
public:
  using QLineEdit::QLineEdit;
  void keyPressEvent(QKeyEvent* event) override;
};


#endif // RETURNEVENTCONSUMINGLINEEDIT_H
