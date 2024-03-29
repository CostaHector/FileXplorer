#include "RenameWidget.h"
#include "Component/NotificatorFrame.h"
#include "Tools/NameTool.h"
#include "Tools/RenameNamesUnique.h"
#include "UndoRedo.h"

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
    qDebug("insert index is not number[%s]", qPrintable(insertAtStr));
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
    qDebug("%s", qPrintable(msg));
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
  QString startNoStr = m_startNo->text();

  bool isnumeric = false;
  int startNo = startNoStr.toInt(&isnumeric);
  if (not isnumeric) {
    qDebug("start index is not number[%s]", qPrintable(startNoStr));
    return replaceeList;
  }

  if (m_completeBaseName->text().isEmpty()) {
    // set default complete basename
    m_completeBaseName->setText(replaceeList[0]);
    m_completeBaseName->selectAll();
  }
  const QString& completeBaseNameString = m_completeBaseName->text();
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
  auto* caseAct = caseAG->checkedAction();  // todo checked
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
  qDebug("Case rule: %s", qPrintable(caseRuleName));
  QStringList replacedList;
  if (g_renameAg()._UPPER_CASE->isChecked()) {
    for (const QString& nm : replaceeList) {
      replacedList.append(nm.toUpper());
    }
  } else if (g_renameAg()._LOWER_CASE->isChecked()) {
    for (const QString& nm : replaceeList) {
      replacedList.append(nm.toLower());
    }
  } else if (g_renameAg()._LOOSE_CAPITALIZE->isChecked()) {  // henry cavill -> Henry cavill and HENRY CAVILL -> HENRY CAVILL
    for (const QString& nm : replaceeList) {
      replacedList.append(NameTool::CapitaliseEachWordFirstLetterOnly(nm));
    }
  } else if (g_renameAg()._STRICT_CAPITALIZE->isChecked()) {  // henry cavill -> Henry cavill and HENRY CAVILL -> Henry cavill
    for (const QString& nm : replaceeList) {
      replacedList.append(NameTool::CapitaliseEachWordFirstLetterLowercaseOthers(nm));
    }
  } else if (g_renameAg()._SWAP_CASE->isChecked()) {
    for (const QString& nm : replaceeList) {
      replacedList.append(NameTool::ToggleSentenceCase(nm));
    }
  } else {
    qDebug("Case rule[%s] not supported", qPrintable(caseRuleName));
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

auto RenameWidget::onApply(const bool isOnlyHelp, const bool isInterative) -> bool {
  const QStringList& relNameList = relName->toPlainText().split('\n');
  const QStringList& oldCompleteNameList = oldCompleteName->toPlainText().split('\n');
  const QStringList& oldSuffixList = oldSuffix->toPlainText().split('\n');

  const QStringList& newCompleteNameList = newCompleteName->toPlainText().split('\n');
  const QStringList& newSuffixList = newSuffix->toPlainText().split('\n');

  RenameNamesUnique renameHelper(m_pre, relNameList, oldCompleteNameList, oldSuffixList, newCompleteNameList, newSuffixList,
                                 includingSub->isChecked());
  renameHelper();
  if (not renameHelper) {
    const QString& rejectMsg = renameHelper.Details();
    qWarning("Cannot do rename commands[%s]", qPrintable(rejectMsg));
    Notificator::badNews("Cannot do rename commands", rejectMsg);
    return false;
  }
  const auto& reversedcmds(renameHelper.getRenameCommands());  // rename files first, than its folders;

  if (isOnlyHelp) {
    for (const QStringList& cmd : reversedcmds) {
      te->appendPlainText(cmd.join('\t'));
    }
    te->setWindowTitle(QString("Rename names unique | Total %1 Command(s)").arg(reversedcmds.size()));
    te->setMinimumWidth(1024);
    te->raise();
    te->show();
    return true;
  }
  bool isAllSuccess = g_undoRedo.Do(reversedcmds);
  if (isInterative) {
    if (isAllSuccess) {
      qInfo("Batch rename ok %d command(s).", reversedcmds.size());
      Notificator::goodNews("Batch rename ok", QString("%1 command(s).").arg(reversedcmds.size()));
    }
  }
  close();
  return isAllSuccess;
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
