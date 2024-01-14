#include "RenameWidget.h"

const QString RenameWidget::INVALID_CHARS("*?\"<>|");
const QSet<QChar> RenameWidget::INVALID_FILE_NAME_CHAR_SET(INVALID_CHARS.cbegin(), INVALID_CHARS.cend());

QStringList RenameWidget_Insert::RenameCore(const QStringList& replaceeList) {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  const QString& insertString = insertStr->text();
  if (insertString.isEmpty()) {
    return replaceeList;
  }
  const QString& insertAtStr = insertAt->text();

  bool isnumeric = false;
  int insertAt = insertAtStr.toInt(&isnumeric);
  if (not isnumeric) {
    qDebug("insert index is not number[%s]", insertAtStr.toStdString().c_str());
    return replaceeList;
  }

  QStringList afterInsert;
  for (const QString& replacee : replaceeList) {
    int realInsertAt = (insertAt > replacee.size()) ? replacee.size() : insertAt;
    afterInsert.append(replacee.left(realInsertAt) + insertString + replacee.mid(realInsertAt));
  }
  return afterInsert;
}

QStringList RenameWidget_Replace::RenameCore(const QStringList& replaceeList) {
  const QString& oldString = oldStr->text();
  const QString& newString = newStr->text();
  auto regexEnable = regex->isChecked();
  if (oldString.isEmpty()) {
    return replaceeList;
  }
  if (not regexEnable) {
    QStringList replacedLst(replaceeList);
    for (QString& s : replacedLst) {
      s.replace(oldString, newString);
    }
    return replacedLst;
  }

  QRegExp repRegex(oldString);
  if (not repRegex.isValid()) {
    const QString& msg = QString("invalid regex[%1]").arg(oldString);
    qDebug("%s", msg.toStdString().c_str());
    regexValidLabel->ToNotSaved();
    return replaceeList;
  }
  regexValidLabel->ToSaved();
  QStringList replacedLst(replaceeList);
  for (QString& s : replacedLst) {
    s.replace(repRegex, newString);
  }
  return replacedLst;
}

QStringList RenameWidget_Numerize::RenameCore(const QStringList& replaceeList) {
  QString startNoStr = startNo->text();

  bool isnumeric = false;
  int startNo = startNoStr.toInt(&isnumeric);
  if (not isnumeric) {
    qDebug("start index is not number[%s]", startNoStr.toStdString().c_str());
    return replaceeList;
  }

  if (completeBaseName->text().isEmpty()) {
    // set default complete basename
    completeBaseName->setText(replaceeList[0]);
    completeBaseName->selectAll();
  }
  const QString& completeBaseNameString = completeBaseName->text();
  const QStringList& suffixs = oldSuffix->toPlainText().split('\n');

  QMap<QString, int> sufCntMap;
  for (const QString& suf : suffixs) {
    if (sufCntMap.contains(suf)) {
      ++sufCntMap[suf];
    } else {
      sufCntMap[suf] = 1;
    }
  }
  QMap<QString, int> sufCurIndex;
  for (const QString& suf : sufCntMap.keys()) {
    if (sufCntMap[suf] > 1) {
      sufCurIndex[suf] = startNo;
    }
  }
  QStringList numerizedNames;
  for (const QString& suf : suffixs) {
    if (not sufCurIndex.contains(suf)) {
      numerizedNames.append(completeBaseNameString);
      continue;
    }
    const QString& newBaseName =
        QString("%1%2(%3)").arg(completeBaseNameString).arg(completeBaseNameString.isEmpty() ? "" : " ").arg(sufCurIndex[suf]);
    numerizedNames.append(newBaseName);
    sufCurIndex[suf] += 1;
  }
  return numerizedNames;
}

auto RenameWidget_Case::RenameCore(const QStringList& replaceeList) -> QStringList {
  auto* caseAct = caseAG->checkedAction();
  if (caseAct == nullptr) {
    qDebug("No rule enabled");
    return replaceeList;
  }
  return RenameWidget_Case::ChangeCaseRename(replaceeList, caseAct->text());
}

QStringList RenameWidget_Case::ChangeCaseRename(const QStringList& replaceeList, const QString& caseRuleName) {
  if (replaceeList.isEmpty()) {
    return {};
  }
  QStringList replacedList;
  if (caseRuleName == "Upper") {
    for (const QString& nm : replaceeList) {
      replacedList.append(nm.toUpper());
    }
  } else if (caseRuleName == "Lower") {
    for (const QString& nm : replaceeList) {
      replacedList.append(nm.toLower());
    }
  } else if (caseRuleName == "Capitalize weak") {  // henry cavill -> Henry cavill and HENRY CAVILL -> HENRY CAVILL
    for (const QString& nm : replaceeList) {
      replacedList.append(CapitaliseEachWordFirstLetterOnly(nm));
    }
  } else if (caseRuleName == "Capitalize strong") {  // henry cavill -> Henry cavill and HENRY CAVILL -> Henry cavill
    for (const QString& nm : replaceeList) {
      replacedList.append(CapitaliseEachWordFirstLetterLowercaseOthers(nm));
    }
  } else if (caseRuleName == "Swapcase") {
    for (const QString& nm : replaceeList) {
      replacedList.append(ToggleSentenceCase(nm));
    }
  } else {
    qDebug("Case rule[%s] not supported", caseRuleName.toStdString().c_str());
    return {};
  }
  return replacedList;
}

auto RenameWidget_SwapSection::RenameCore(const QStringList& replaceeList) -> QStringList {
  if (replaceeList.isEmpty()) {
    return replaceeList;
  }
  QAction* caseAct = caseAG->checkedAction();
  if (caseAct == nullptr) {
    return replaceeList;
  }
  int f = caseAct->text().left(1).toInt();
  int s = caseAct->text().right(1).toInt();
  QStringList sectionSwapped;
  for (const QString nm : replaceeList) {
    QStringList secList = nm.split('-');
    if (secList.size() > s) {  // swap element at f, s index;
      std::swap(secList[f], secList[s]);
      sectionSwapped.append(secList.join('-'));
    } else {
      sectionSwapped.append(nm);
    }
  }
  return sectionSwapped;
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>

int main(int argc, char* argv[]) {
  QDir dir(QFileInfo(QFileInfo(__FILE__).absolutePath()).absolutePath());
  QStringList rels = dir.entryList(QDir::Filter::AllEntries | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::DirsFirst);

  QApplication a(argc, argv);
  RenameWidget_Insert winI;
  RenameWidget_Replace winR;
  RenameWidget_Delete winD;
  RenameWidget_Numerize winN;
  RenameWidget_Case winC;

  winI.init();
  winR.init();
  winD.init();
  winN.init();
  winC.init();

  winI.InitTextContent(dir.absolutePath(), rels);
  winR.InitTextContent(dir.absolutePath(), rels);
  winD.InitTextContent(dir.absolutePath(), rels);
  winN.InitTextContent(dir.absolutePath(), rels);
  winC.InitTextContent(dir.absolutePath(), rels);
  // winR.InitTextContent(pre, rels)
  // winD.InitTextContent(pre, rels)
  // winN.InitTextContent(pre, rels)
  winI.show();
  winR.show();
  winD.show();
  winN.show();
  winC.show();
  return a.exec();
}
#endif
