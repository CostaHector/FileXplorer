#ifndef NOENTERLINEEDIT_H
#define NOENTERLINEEDIT_H

#include <QLineEdit>
class NoEnterLineEdit : public QLineEdit {
public:
  using QLineEdit::QLineEdit;
  void keyPressEvent(QKeyEvent* event) override;
};


#endif // NOENTERLINEEDIT_H
