#include "LineEditCSV.h"
#include "NameTool.h"
#include "Logger.h"
#include <QRegularExpression>

LineEditCSV::LineEditCSV(const QString& formName, const QString& text, const bool bNoDuplicate, QWidget* parent)  //
    : QLineEdit{text, parent},                                                                                    //
      mEleUnique{bNoDuplicate},                                                                                   //
      mFormName{formName}                                                                                         //
{}

QString LineEditCSV::GetFormName() const {
  return mFormName;
}

QStringList LineEditCSV::GetStringList() const {  // sort
  return NameTool::CastTagSentenceParse2Lst(text(), mEleUnique);
}

QVariantList LineEditCSV::GetVariantList() const {  // sort
  const QStringList& sl = GetStringList();
  QList<int> ansSet;
  for (const QString& s : sl) {
    bool isInt = false;
    int iVal = s.toInt(&isInt);
    if (!isInt) {
      LOG_D("Not a number[%s]", qPrintable(s));
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
  if (mEleUnique) {
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
  if (mEleUnique) {
    castLst.removeDuplicates();
  }
  setText(castLst.join(NameTool::CSV_COMMA));
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
  if (mEleUnique) {
    hotSceneSL.removeDuplicates();
  }
  setText(hotSceneSL.join(NameTool::CSV_COMMA));
  return hotSceneSL.size();
}

void LineEditCSV::Format() {
  const QString& lineTxt = text();
  NameTool nt;
  QStringList lst = nt(lineTxt);
  if (mEleUnique) {
    lst.removeDuplicates();
  }
  const QString& formatedLineTxt = lst.join(NameTool::CSV_COMMA);
  if (formatedLineTxt == lineTxt) {
    return;
  }
  setText(formatedLineTxt);
  LOG_D("CSV line updated to %s", qPrintable(formatedLineTxt));
}
