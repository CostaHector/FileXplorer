#ifndef NAMESECTIONARRANGE_H
#define NAMESECTIONARRANGE_H
#include <QString>
#include <QStringList>

bool SubscriptsStr2Int(const QString& subscripts, QList<int>& sortedIndLst);
bool SubscriptsDigitChar2Int(const QString& subscripts, QList<int>& sortedIndLst);

class NameSectionArrange {
 public:
  NameSectionArrange();
  NameSectionArrange(const QList<int>& sortedIndlst, const bool strictMode = true);
  NameSectionArrange(const int section1, const int section2, const bool strictMode = true);

  QString operator()(const QString& names);
  QStringList BatchSwapper(const QStringList& lst);
  bool HasWasted() const { return !m_wastedList.isEmpty(); }
  QString GetWastedNames() const { return m_wastedList.join("\n"); }

  static const QStringList PATTERN_INDEX_FREQ;
  static const QStringList SWAP_INDEX_FREQ;
  static const QList<int> INDEX_ARR;

 private:
  bool m_recordWasted;
  QList<int> m_seq;

  QStringList m_wastedList;  // remember to check this member when m_strictMode is false.
};

#endif  // NAMESECTIONARRANGE_H
