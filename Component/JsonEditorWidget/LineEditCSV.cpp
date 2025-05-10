#include "LineEditCSV.h"
#include "Tools/NameTool.h"
#include <QRegularExpression>

constexpr char LineEditCSV::CSV_COMMA;
const QRegularExpression LineEditCSV::CAST_STR_SPLITTER{R"( & |&|, |,|\r\n|\n| and )"};

LineEditCSV::LineEditCSV(const QString& formName, const QString& text, const bool bNoDuplicate, QWidget* parent)  //
    : QLineEdit{text, parent},                                                                                    //
      mNoDuplicate{bNoDuplicate},                                                                                 //
      mFormName{formName}                                                                                         //
{}

QString LineEditCSV::GetFormName() const {
  return mFormName;
}

QStringList LineEditCSV::GetStringList() const {  // sort
  const QString& s = text();
  if (s.isEmpty()) {
    return {};
  }
  QStringList lst = s.split(CAST_STR_SPLITTER);
  lst.sort();
  if (mNoDuplicate) {
    lst.removeDuplicates();
  }
  return lst;
}

QVariantList LineEditCSV::GetVariantList() const {  // sort
  const QStringList& sl = GetStringList();
  QList<int> ansSet;
  for (const QString& s : sl) {
    bool isInt = false;
    int iVal = s.toInt(&isInt);
    if (!isInt) {
      qDebug("Not a number[%s]", qPrintable(s));
      continue;
    }
    ansSet.append(iVal);
  }
  std::sort(ansSet.begin(), ansSet.end(), std::less<int>());
  auto duplicateIt = std::unique(ansSet.begin(), ansSet.end());
  ansSet.erase(duplicateIt, ansSet.end());
  return QVariantList{ansSet.cbegin(), ansSet.cend()};
}

int LineEditCSV::AppendFromStringList(const QStringList& sl) {  // sort
  QStringList curSl = GetStringList();
  curSl += sl;
  curSl.sort();
  if (mNoDuplicate) {
    curSl.removeDuplicates();
  }
  ReadFromStringList(curSl);
  return curSl.size();
}

int LineEditCSV::ReadFromStringList(const QStringList& sl) {  // sort
  QStringList castLst;
  foreach (QString cast, sl) {
    castLst.append(cast.trimmed());
  }
  castLst.sort();
  if (mNoDuplicate) {
    castLst.removeDuplicates();
  }
  setText(castLst.join(CSV_COMMA));
  return castLst.size();
}

int LineEditCSV::ReadFromVariantList(const QVariantList& vl) {  // sort
  QList<int> intLst;
  bool isInt = false;
  int spot = 0;
  for (const QVariant& ivariant : vl) {
    isInt = false;
    spot = ivariant.toInt(&isInt);
    if (!isInt) {
      continue;
    }
    intLst.append(spot);
  }
  std::sort(intLst.begin(), intLst.end(), std::less<int>());

  QStringList hotSceneSL;
  for (auto spot : intLst) {
    hotSceneSL.append(QString::number(spot));
  }
  if (mNoDuplicate) {
    hotSceneSL.removeDuplicates();
  }
  setText(hotSceneSL.join(CSV_COMMA));
  return hotSceneSL.size();
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
  qDebug("CSV line updated to %s", qPrintable(formatedLineTxt));
}
