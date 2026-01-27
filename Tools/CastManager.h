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
  const CAST_MGR_DATA_T& SingleWordActorSet() const { return mSingleWordActors; }

  std::pair<CAST_MGR_DATA_T,CAST_MGR_DATA_T> ReadOutActors() const;
  int ForceReloadImpl();

  CAST_MGR_DATA_T ActorIncrement(const CAST_MGR_DATA_T& actors);
  CAST_MGR_DATA_T SingleWordActorIncrement(const CAST_MGR_DATA_T& singleWordActors);
  int WriteIntoLocalDictionaryFiles(const CAST_MGR_DATA_T& increments, bool bWriteIntoSingleWordFilePath=false) const;

  static QString RmvBelongLetter(const QString& word);
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words, const bool bIsActorFromSingleWordStudio) const;
  QStringList operator()(const QString& sentence, const bool bIsActorFromSingleWordStudio=false) const;

 private:
  CastManager();
  CastManager(const CastManager& rhs) noexcept = delete;
  static bool IsActorNameValid(const QString& actorName);
  void InitializeImpl(const QString& path, const QString& blackPath="");
  CAST_MGR_DATA_T mSingleWordActors;
  QString mLocalFilePath, mLocalSingleWordFilePath;
};

#endif  // CastManager_H
