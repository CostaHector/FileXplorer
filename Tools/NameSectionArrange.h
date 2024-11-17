#ifndef NAMESECTIONARRANGE_H
#define NAMESECTIONARRANGE_H
#include <QString>
#include <QStringList>

class NameSectionArrange {
 public:
  NameSectionArrange();
  NameSectionArrange(const QString& indexPattern, const bool strictMode = true);
  NameSectionArrange(const int section1, const int section2, const bool strictMode = true);

  static NameSectionArrange FromUserInput(const QString& userInput, const bool strictMode = true);

  operator bool() const { return m_valid; }

  QString operator()(const QString& names);

  QStringList BatchSwapper(const QStringList& lst);

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
