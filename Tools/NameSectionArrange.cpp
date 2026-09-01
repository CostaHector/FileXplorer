#include "NameSectionArrange.h"
#include <QRegularExpression>

QString ChopPostfix(const QString& baseName) {
  // 1. resolution/date
  static const QRegularExpression resDatePattern{" (?:2160p|1080p|360p|480p|720p|810p|4K|FHD|HD|SD|\\d{4} - \\d{2} - \\d{2}|\\d{8}|\\d{4})$", QRegularExpression::PatternOption::CaseInsensitiveOption};
  QString result = baseName;
  result.remove(resDatePattern);

  // 2. "Part/Scene/Pt./Sc./Pt/Sc \d{1,2}" no need chop
  static const QRegularExpression keywordPattern{" (?:Part|Scene|Pt\\.?|Sc\\.?) ?\\d{1,2}$", QRegularExpression::PatternOption::CaseInsensitiveOption};
  if (keywordPattern.match(result).hasMatch()) {
    return result;  // 保留关键词和数字，只删除了年份
  }

  // 3. 删除末尾的数字（以及前面可能的分隔符 "- "）
  static const QRegularExpression numberPattern{" (?:- )?\\d{1,2}$"};
  result.remove(numberPattern);

  // 4. resolution/date again
  result.remove(resDatePattern);

  return result;
}

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

NameSectionArrange::NameSectionArrange() : m_recordWasted{true}, m_chopPostfix{false} {  //
}

NameSectionArrange::NameSectionArrange(const QList<int>& sortedIndlst, const bool strictMode, bool bChopPostfix)  //
    : m_recordWasted{strictMode},                                                              //
      m_seq{sortedIndlst},
      m_chopPostfix{bChopPostfix}{                                                                    //
}

NameSectionArrange::NameSectionArrange(const int section1, const int section2, const bool strictMode, bool bChopPostfix)  //
    : NameSectionArrange{INDEX_ARR, strictMode, bChopPostfix} {                                                      // swap section edition
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

QString NameSectionArrange::operator()(const QString& baseName) {
  const int postfixStartIndex = m_chopPostfix ? ChopPostfix(baseName).size() : baseName.size();
  const QStringList& section = baseName.left(postfixStartIndex).split('-');
  if (m_seq == INDEX_ARR && m_seq.size() == section.size()) {
    return baseName;
  }

  QStringList newNameSec;
  for (int index : m_seq) {
    if (index < 0 || index >= section.size()) {
      continue;
    }
    newNameSec << section[index].trimmed();
  }
  if (m_recordWasted && newNameSec.size() < section.size()) {  // Attention: some section part is wasted
    m_wastedList.push_back(baseName);
  }
  static const QString SECTION_JOIN_SEP = " - ";
  return newNameSec.join(SECTION_JOIN_SEP) + baseName.mid(postfixStartIndex);
}
