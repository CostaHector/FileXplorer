#ifndef _LINE_EDIT_CSV_H
#define _LINE_EDIT_CSV_H

#include <QLineEdit>

class LineEditCSV : public QLineEdit {
 public:
  explicit LineEditCSV(const QString& formName, const QString &text="", QWidget *parent = nullptr);
  int AppendFromStringList(const QStringList& sl);
  void ReadFromStringList(const QStringList& sl);
  void ReadFromVariantList(const QVariantList& vl);
  QString GetFormName() const;
  QStringList GetStringList() const;
  QList<QVariant> GetVariantList() const;
  void Format();
 private:
  QString mFormName;
  static const QString SEPERATOR;
};

#endif
