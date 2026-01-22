#ifndef CastManager_H
#define CastManager_H

#include <QSet>
#include <QString>
#include "SingletonManager.h"
using CAST_MGR_DATA_T = QSet<QString>;
class CastManager;
extern template class SingletonManager<CastManager, CAST_MGR_DATA_T>;

class CastManager final : public SingletonManager<CastManager, CAST_MGR_DATA_T> {
 public:
  friend class SingletonManager<CastManager, CAST_MGR_DATA_T>;
  CAST_MGR_DATA_T& CastSet() { return data(); }
  const CAST_MGR_DATA_T& CastSet() const { return data(); }

  CAST_MGR_DATA_T ReadOutCasts() const;
  int ForceReloadImpl();

  int LearningFromAPath(const QString& path, bool* bHasWrite = nullptr);
  int CastIncrement(CAST_MGR_DATA_T& increments, CAST_MGR_DATA_T delta);
  int WriteIntoLocalDictionaryFiles(const CAST_MGR_DATA_T& increments) const;

  static QString RmvBelongLetter(const QString& word);
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  QStringList operator()(const QString& sentence) const;

 private:
  CastManager();
  CastManager(const CastManager& rhs) noexcept = delete;
  static bool IsActorNameValid(const QString& actorName);
  void InitializeImpl(const QString& path, const QString& blackPath="");
  QString mLocalFilePath, mLocalBlackFilePath;
};

#endif  // CastManager_H
