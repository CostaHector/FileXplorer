#ifndef _TEXT_EDIT_MULTILINE_H
#define _TEXT_EDIT_MULTILINE_H

#include <QTextEdit>

class TextEditMultiLine : public QTextEdit {
 public:
  explicit TextEditMultiLine(const QString& formName, const QString &text="", QWidget *parent = nullptr);
  QString GetFormName() const;
 private:
  QString mFormName;
};

#endif
