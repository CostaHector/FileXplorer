#include "LineEditCSV.h"
#include "Tools/NameTool.h"

const QString LineEditCSV::SEPERATOR = ", ";

LineEditCSV::LineEditCSV(const QString& formName, const QString& text, QWidget* parent) : QLineEdit{text, parent}, mFormName{formName} {}

QString LineEditCSV::GetFormName() const {
  return mFormName;
}

QStringList LineEditCSV::GetStringList() const {
  return text().split(", ");
}

QList<QVariant> LineEditCSV::GetVariantList() const {
  const QStringList& sl = GetStringList();
  QList<QVariant> ans;
  for (const QString& s: sl) {
    bool isInt = false;
    int iVal = s.toInt(&isInt);
    if (!isInt) {
      qDebug("%s is not a number", qPrintable(s));
      continue;
    }
    ans << iVal;
  }
  return ans;
}

int LineEditCSV::AppendFromStringList(const QStringList& sl)
{
  QStringList curSl = GetStringList();
  curSl += sl;
  curSl.sort();
  curSl.removeDuplicates();
  ReadFromStringList(curSl);
  return curSl.size();
}

void LineEditCSV::ReadFromStringList(const QStringList& sl) {
  setText(sl.join(SEPERATOR));
}

void LineEditCSV::ReadFromVariantList(const QVariantList& vl) {
  QStringList hotSceneSL;
  for (const QVariant& ivariant : vl) {
    bool isInt = false;
    int hot = ivariant.toInt(&isInt);
    if (!isInt) {
      continue;
    }
    hotSceneSL.append(QString::number(hot));
  }
  ReadFromStringList(hotSceneSL);
}

void LineEditCSV::Format() {
  const QString& lineTxt = text();
  NameTool nt;
  const QStringList& lst = nt(lineTxt);
  const QString& formatedLineTxt = lst.join(SEPERATOR);
  if (formatedLineTxt == lineTxt) {
    return;
  }
  setText(formatedLineTxt);
  qDebug("csv line updated to %s", qPrintable(formatedLineTxt));
}
