#ifndef _LINE_EDIT_CSV_H
#define _LINE_EDIT_CSV_H

#include <QLineEdit>

class LineEditCSV : public QLineEdit {
 public:
  explicit LineEditCSV(const QString& formName, const QString &text="", const bool bNoDuplicate = false, QWidget *parent = nullptr);
  int AppendFromStringList(const QStringList& sl);
  void ReadFromStringList(const QStringList& sl);
  void ReadFromVariantList(const QVariantList& vl);
  QString GetFormName() const;
  QStringList GetStringList() const;
  QList<QVariant> GetVariantList() const;
  void Format();
 private:
  const bool mNoDuplicate;
  QString mFormName;
  static constexpr char CSV_COMMA = ',';
  static const QRegularExpression CAST_STR_SPLITTER;
};

#endif
