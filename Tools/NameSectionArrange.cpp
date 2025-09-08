#include "NameSectionArrange.h"
#include "Logger.h"
#include <QRegularExpression>

bool SubscriptsStr2Int(const QString& subscripts, QList<int>& sortedIndLst) {
  static const QRegularExpression SORT_INDEX_SEP{R"(,\s*|\s+)"};
  const QStringList& sortedIndLstStr = subscripts.split(SORT_INDEX_SEP);
  sortedIndLst.reserve(sortedIndLstStr.size());
  int ind = 0;
  for (const QString& indStr : sortedIndLstStr) {
    bool isNumber = false;
    ind = indStr.toInt(&isNumber);
    if (!isNumber) {
      return false;
    }
    sortedIndLst << ind;
  }
  return true;
}

bool SubscriptsDigitChar2Int(const QString& subscripts, QList<int>& sortedIndLst) {
  sortedIndLst.reserve(subscripts.size());
  for (auto c : subscripts) {
    if (!c.isDigit()) {
      continue;
    }
    sortedIndLst.push_back(c.toLatin1() - '0');
  }
  return true;
}

const QStringList NameSectionArrange::PATTERN_INDEX_FREQ{"0213456", "0132456"};
const QStringList NameSectionArrange::SWAP_INDEX_FREQ{"1,2", "2,3", "1,3", "0,1", "0,2", "0,3"};
const QList<int> NameSectionArrange::INDEX_ARR{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

NameSectionArrange::NameSectionArrange() : m_recordWasted{true} {  //
}

NameSectionArrange::NameSectionArrange(const QList<int>& sortedIndlst, const bool strictMode)  //
    : m_recordWasted{strictMode},                                                              //
      m_seq{sortedIndlst} {                                                                    //
}

NameSectionArrange::NameSectionArrange(const int section1, const int section2, const bool strictMode)  //
    : NameSectionArrange{INDEX_ARR, strictMode} {                                                      // swap section edition
  std::swap(m_seq[section1], m_seq[section2]);
}

QStringList NameSectionArrange::BatchSwapper(const QStringList& lst) {
  m_wastedList.clear();
  m_wastedList.reserve(lst.size());

  QStringList ansList;
  ansList.reserve(lst.size());
  for (const QString& name : lst) {
    ansList.push_back(operator()(name));
  }
  return ansList;
}

QString NameSectionArrange::operator()(const QString& names) {
  const QStringList& section = names.split('-');
  if (m_seq == INDEX_ARR && m_seq.size() == section.size()) {
    LOG_D("nothing changes");
    return names;
  }

  QStringList newNameSec;
  for (int index : m_seq) {
    if (index < 0 || index >= section.size()) {
      continue;
    }
    newNameSec << section[index].trimmed();
  }
  if (m_recordWasted && newNameSec.size() < section.size()) {  // Attention: some section part is wasted
    m_wastedList.push_back(names);
  }
  static const QString SECTION_JOIN_SEP = " - ";
  return newNameSec.join(SECTION_JOIN_SEP);
}
