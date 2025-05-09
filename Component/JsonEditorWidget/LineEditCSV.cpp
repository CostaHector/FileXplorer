#include "LineEditCSV.h"
#include "Tools/NameTool.h"
#include <QRegularExpression>

constexpr char LineEditCSV::CSV_COMMA;
const QRegularExpression LineEditCSV::CAST_STR_SPLITTER{R"( & |&| ,|,|\r\n|\n| and )"};

LineEditCSV::LineEditCSV(const QString& formName, const QString& text, const bool bNoDuplicate, QWidget* parent)//
  : QLineEdit{text, parent}, //
    mNoDuplicate{bNoDuplicate}, //
    mFormName{formName} //
{
}

QString LineEditCSV::GetFormName() const {
  return mFormName;
}

QStringList LineEditCSV::GetStringList() const {
  const QString&s = text();
  if (s.isEmpty()){
    return {};
  }
  QStringList lst = s.split(CAST_STR_SPLITTER);
  if (mNoDuplicate){
    lst.removeDuplicates();
  }
  return lst;
}

QList<QVariant> LineEditCSV::GetVariantList() const {
  const QStringList& sl = GetStringList();
  QList<QVariant> ans;
  for (const QString& s : sl) {
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

int LineEditCSV::AppendFromStringList(const QStringList& sl) {
  QStringList curSl = GetStringList();
  curSl += sl;
  curSl.sort();
  if (mNoDuplicate) {
    curSl.removeDuplicates();
  }
  ReadFromStringList(curSl);
  return curSl.size();
}

void LineEditCSV::ReadFromStringList(const QStringList& sl) {
  setText(sl.join(CSV_COMMA));
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
  QStringList lst = nt(lineTxt);
  if (mNoDuplicate) {
    lst.removeDuplicates();
  }
  const QString& formatedLineTxt = lst.join(CSV_COMMA);
  if (formatedLineTxt == lineTxt) {
    return;
  }
  setText(formatedLineTxt);
  qDebug("csv line updated to %s", qPrintable(formatedLineTxt));
}
