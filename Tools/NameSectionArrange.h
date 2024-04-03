#ifndef NAMESECTIONARRANGE_H
#define NAMESECTIONARRANGE_H
#include <QString>
#include <QStringList>

class NameSectionArrange {
 public:
  NameSectionArrange() : m_valid{false} {}

  NameSectionArrange(const QString& indexPattern, const bool strictMode = true) : m_strictMode(strictMode), m_valid{true} {
    m_seq.reserve(indexPattern.size());
    for (auto c : indexPattern) {
      if (not c.isDigit()) {
        m_valid = false;
        return;
      }
      m_seq.push_back(c.toLatin1() - '0');
    }
  }

  NameSectionArrange(const int section1, const int section2, const bool strictMode = true)
      : NameSectionArrange{"0123456789", strictMode} {  // swap section edition
    if (not (0 <= section1 and section1 < m_seq.size() and 0 <= section2 and section2 < m_seq.size())){
      m_valid = false;
      return;
    }
    std::swap(m_seq[section1], m_seq[section2]);
  }

  static NameSectionArrange FromUserInput(const QString& userInput, const bool strictMode = true);

  operator bool() const { return m_valid; }

  QString operator()(const QString& names) {
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

  QStringList BatchSwapper(const QStringList& lst) {
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

  QStringList m_wastedList;  // remember to check this member when m_strictMode is false.
  static const QStringList PATTERN_INDEX_FREQ;
  static const QStringList SWAP_INDEX_FREQ;
  static const int INDEX_ARR[10];

 private:
  bool m_valid;

  bool m_strictMode;
  QList<int> m_seq;
};

#endif  // NAMESECTIONARRANGE_H
