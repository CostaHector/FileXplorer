#include "NameSectionArrange.h"

const QStringList NameSectionArrange::PATTERN_INDEX_FREQ{"0213456", "0132456"};
const QStringList NameSectionArrange::SWAP_INDEX_FREQ{"1,2", "2,3"};
const int NameSectionArrange::INDEX_ARR[10]{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

NameSectionArrange::NameSectionArrange() : m_valid{false}, m_strictMode{true} {}

NameSectionArrange::NameSectionArrange(const QString& indexPattern, const bool strictMode) : m_valid{true}, m_strictMode{strictMode} {
  m_seq.reserve(indexPattern.size());
  for (auto c : indexPattern) {
    if (not c.isDigit()) {
      m_valid = false;
      return;
    }
    m_seq.push_back(c.toLatin1() - '0');
  }
}

NameSectionArrange::NameSectionArrange(const int section1, const int section2, const bool strictMode) : NameSectionArrange{"0123456789", strictMode} {  // swap section edition
  if (!(0 <= section1 && section1 < m_seq.size() && 0 <= section2 && section2 < m_seq.size())) {
    m_valid = false;
    return;
  }
  std::swap(m_seq[section1], m_seq[section2]);
}

NameSectionArrange NameSectionArrange::FromUserInput(const QString& userInput, const bool strictMode) {
  const int commaCnt = userInput.count(',');
  if (commaCnt > 1) {
    return NameSectionArrange();
  }
  if (commaCnt == 1) {
    int ind = userInput.indexOf(',');
    bool is1stIndexInt;
    const int section1 = userInput.left(ind).toInt(&is1stIndexInt);
    bool is2ndIndexInt;
    const int section2 = userInput.mid(ind + 1).toInt(&is2ndIndexInt);
    if (not is1stIndexInt or not is2ndIndexInt or section1 == section2 or section1 < 0 or section2 < 0 or section1 > 9 or section2 > 9) {
      qWarning("userInput[%s] is invalid[section1=%d, section2=%d]", qPrintable(userInput), section1, section2);
      return NameSectionArrange();
    }
    return NameSectionArrange{section1, section2, strictMode};
  }
  return NameSectionArrange{userInput, strictMode};
}

QStringList NameSectionArrange::BatchSwapper(const QStringList& lst) {
  m_wastedList.clear();
  m_wastedList.reserve(lst.size());

  if (!operator bool()) {
    return lst;
  }

  QStringList ansList;
  ansList.reserve(lst.size());
  for (const QString& name : lst) {
    ansList.push_back(operator()(name));
  }
  return ansList;
}

QString NameSectionArrange::operator()(const QString& names) {
  if (!operator bool()) {
    return names;
  }
  const QStringList& section = names.split('-');
  if (m_seq == QList<int>(INDEX_ARR, INDEX_ARR + m_seq.size()) and m_seq.size() == section.size()) {
    return names;  // nothing changes
  }

  int sectionMet = 0;
  QString ans;
  ans.reserve(names.size() + 3);
  for (int index : m_seq) {
    if (0 > index || index >= section.size()) {
      continue;
    }
    ans += section[index].trimmed() + " - ";
    ++sectionMet;
  }
  if (sectionMet < section.size()) {
    // Attention: some section part is wasted
    m_wastedList.push_back(names);
    if (m_strictMode) {
      return names;
    }
  }
  if (!ans.isEmpty()) {
    ans.remove(ans.size() - 3, ans.size());
  }
  return ans;
}
