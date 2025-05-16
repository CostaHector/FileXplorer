#ifndef SORTEDUNIQSTRLST_H
#define SORTEDUNIQSTRLST_H
#include "qvariant.h"
#include <QSet>
#include <QString>

class SortedUniqStrLst {
 public:
  friend struct JsonPr;

  operator QVariant() const { return join(); }

  SortedUniqStrLst() = default;
  explicit SortedUniqStrLst(const QString& sentence);
  explicit SortedUniqStrLst(const QStringList& initList);
  bool operator==(const SortedUniqStrLst& rhs) const {  //
    return m_set == rhs.m_set;
  }
  bool operator!=(const SortedUniqStrLst& rhs) const {  //
    return !(this->operator==(rhs));
  }

  void setBatchFromSentence(const QString& sentence);
  void setBatch(const QStringList& list);

  void insertBatchFromSentence(const QString& sentence);
  void insertBatch(const QStringList& newItems);

  void insertBatch_LE_3(const QStringList& list);
  void insertBatch_LE_5(const QStringList& list);
  void insertBatch_GT_5(const QStringList& newItems);

  bool remove(const QString& target);

  void clear() {
    m_set.clear();
    m_sortedCache.clear();
    mJoinCalled = false;
  }

  const QStringList& toSortedList() const { return m_sortedCache; }
  const QString& join() const;

  int count() const { return m_set.size(); }
  bool isEmpty() const { return m_set.isEmpty(); }

 private:
  QSet<QString> m_set;
  QStringList m_sortedCache;
  mutable QString mAnsCSV;
  mutable bool mJoinCalled{false};
};

#endif  // SORTEDUNIQSTRLST_H
