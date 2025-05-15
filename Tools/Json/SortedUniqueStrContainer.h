#ifndef SORTEDUNIQUESTRCONTAINER_H
#define SORTEDUNIQUESTRCONTAINER_H
#include "qvariant.h"
#include <QSet>
#include <QString>

class SortedUniqueStrContainer {
 public:
  friend struct JsonPr;

  operator QVariant() const { return join(); }

  SortedUniqueStrContainer() = default;
  explicit SortedUniqueStrContainer(const QString& sentence);
  explicit SortedUniqueStrContainer(const QStringList& initList);
  bool operator==(const SortedUniqueStrContainer& rhs) const {  //
    return m_set == rhs.m_set;
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

#endif  // SORTEDUNIQUESTRCONTAINER_H
