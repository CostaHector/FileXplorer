#ifndef _LINE_EDIT_INT_H
#define _LINE_EDIT_INT_H

#include <QLineEdit>

class LineEditInt : public QLineEdit {
 public:
  explicit LineEditInt(const QString& formName, const QString &text="", QWidget *parent = nullptr);
  void ReadFromInt(int iVal);
  QString GetFormName() const;
  bool CheckValueValid() const;
  int GetIntValue() const;
 private:
  QString mFormName;
};

#endif
