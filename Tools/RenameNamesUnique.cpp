#include "RenameNamesUnique.h"
#include "PathTool.h"

#include <QDir>
#include <QFileInfo>

RenameNamesUnique::RenameNamesUnique(const QString& pre,
                                     const QStringList& relNameList,
                                     const QStringList& oldCompleteNameList,
                                     const QStringList& oldSuffixList,
                                     const QStringList& newCompleteNameList,
                                     const QStringList& newSuffixList,
                                     const bool isIncludeSubDir)
    : m_pre(pre),
      m_relNameList{relNameList},
      m_oldCompleteNameList{oldCompleteNameList},
      m_oldSuffixList{oldSuffixList},
      m_newCompleteNameList{newCompleteNameList},
      m_newSuffixList{newSuffixList},
      m_isIncludeSubDir{isIncludeSubDir} {
  const QSet<int> inputSizeSet{relNameList.size(), oldCompleteNameList.size(), oldSuffixList.size(), newCompleteNameList.size(), newSuffixList.size()};
  m_isArrLenUnequal = inputSizeSet.size() != 1;
  if (m_isArrLenUnequal) {
    LOG_C("Invalid. Parms with [%d] unequal array lengths.", inputSizeSet.size());
    return;
  }
  m_nameLineEmpty = newCompleteNameList.contains("");
  if (m_nameLineEmpty) {
    LOG_C("Invalid. Some complete filename is empty");
    return;
  }

  for (auto i = 0; i < m_oldCompleteNameList.size(); ++i) {
    m_leftNames.append(m_oldCompleteNameList[i] + m_oldSuffixList[i]);
    m_rightNames.append(m_newCompleteNameList[i] + m_newSuffixList[i]);
  }
}

QSet<QString> RenameNamesUnique::getOccupiedPostPath(const QString& pre, const QStringList& leftRoots, const QStringList& leftNames, bool includeSub) {
  // m_pre + '/' + (rel2Names + '/' + (filename = basename + '.' + extension))
  // i.e.. m_pre + '/' + postPath
  // only filename is viable
  // rel2Names is not viable. when directory name need change. directory would just become filename
  // m_occupiedNames only need to store postPath
  const QStringList itemsInPre = QDir{pre}.entryList({}, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::NoSort);
  QSet<QString> occupied{itemsInPre.cbegin(), itemsInPre.cend()};
  if (!includeSub) {
    return occupied;
  }
  for (int i = 0; i < leftNames.size(); ++i) {
    QFileInfo fi{pre + '/' + leftRoots[i]};
    if (fi.isDir()) {
      const QStringList& itemsInLeftRoot =
          join2Path(leftRoots[i], QDir(pre + '/' + leftRoots[i]).entryList({}, QDir::Filter::Files | QDir::Filter::Dirs | QDir::Filter::NoDotAndDotDot, QDir::SortFlag::NoSort));
      occupied.unite(QSet<QString>{itemsInLeftRoot.cbegin(), itemsInLeftRoot.cend()});

      continue;
    }
    occupied.insert(join2Path(leftRoots[i], leftNames[i]));
  }
  return occupied;
}

QString RenameNamesUnique::join2Path(const QString& rel2Name, const QString& name) {
  return rel2Name.isEmpty() ? name : rel2Name + '/' + name;
}

QStringList RenameNamesUnique::join2Path(const QString& rel2Name, const QStringList& names) {
  if (rel2Name.isEmpty()) {
    return names;
  }
  QStringList needPushFront{names};
  std::for_each(needPushFront.begin(), needPushFront.end(), [&rel2Name](QString& s) { s = rel2Name + '/' + s; });
  return needPushFront;
}

bool RenameNamesUnique::CheckConflict(QSet<QString> occupied, const QStringList& leftRoots, const QStringList& leftNames, const QStringList& rightNames, QStringList& conflictNames) {
  conflictNames.clear();
  for (int i = 0; i < leftRoots.size(); ++i) {
    const QString& leftPostPath = join2Path(leftRoots[i], leftNames[i]);
    const QString& rightPostPath = join2Path(leftRoots[i], rightNames[i]);
    if (leftPostPath == rightPostPath) {
      continue;
    }
    if (occupied.contains(rightPostPath)) {
      conflictNames.push_back(rightPostPath);
    } else {  // not conflict
      auto renamedAwayIt = occupied.find(leftPostPath);
      occupied.erase(renamedAwayIt);
      occupied.insert(rightPostPath);
    }
  }
  return conflictNames.isEmpty();
}

bool RenameNamesUnique::operator()() {
  if (!isInputValid()) {
    LOG_C("Reject do check. input is invalid");
    return false;
  }
  m_occupiedNames = getOccupiedPostPath(m_pre, m_relNameList, m_leftNames, m_isIncludeSubDir);
  return CheckConflict(m_occupiedNames, m_relNameList, m_leftNames, m_rightNames, m_conflictNames);
}

using namespace FileOperatorType;
BATCH_COMMAND_LIST_TYPE RenameNamesUnique::getRenameCommands() const {
  if (!operator bool()) {
    return {};
  }
  BATCH_COMMAND_LIST_TYPE cmds;
  for (auto i = 0; i < m_oldCompleteNameList.size(); ++i) {
    if (m_leftNames[i] == m_rightNames[i]) {
      continue;
    }
    QString prePath{m_pre};
    if (!m_relNameList[i].isEmpty()) {
      prePath += '/';
      prePath += m_relNameList[i];
    }
    cmds.append(ACMD::GetInstRENAME(prePath, m_leftNames[i], m_rightNames[i]));
  }
  return {cmds.crbegin(), cmds.crend()};  // rename files first, than its folders;
}
