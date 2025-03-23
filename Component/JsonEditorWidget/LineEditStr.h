#ifndef _LINE_EDIT_STR_H
#define _LINE_EDIT_STR_H

#include <QLineEdit>

class LineEditStr : public QLineEdit {
 public:
  explicit LineEditStr(const QString& formName, const QString &text="", QWidget *parent = nullptr);
  QString GetFormName() const;
 private:
  QString mFormName;
};

#endif
