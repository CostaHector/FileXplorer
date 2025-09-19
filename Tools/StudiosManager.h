#ifndef STUDIOSMANAGER_H
#define STUDIOSMANAGER_H

#include <QHash>
#include <QString>
#include "SingletonManager.h"

// coarse studio name -> std studio name
// warner bros. => WarnerBros.
// warnerbros. => WarnerBros.
using STUDIO_MGR_DATA_T = QHash<QString, QString>;
class StudiosManager;
extern template class SingletonManager<StudiosManager, STUDIO_MGR_DATA_T>;

class StudiosManager final : public SingletonManager<StudiosManager, STUDIO_MGR_DATA_T> {
 public:
  friend class SingletonManager<StudiosManager, STUDIO_MGR_DATA_T>;

  STUDIO_MGR_DATA_T& ProStudioMap() { return data(); }
  const STUDIO_MGR_DATA_T& ProStudioMap() const { return data(); }

  STUDIO_MGR_DATA_T ReadOutStdStudioName() const;

  int ForceReloadImpl();

  int LearningFromAPath(const QString& path, bool* bHasWrite = nullptr);
  int StudioIncrement(STUDIO_MGR_DATA_T& increments, const QString& newStudio);
  int WriteIntoLocalDictionaryFiles(const STUDIO_MGR_DATA_T& increments) const;

  QString ProductionStudioFilterOut(const QString& words) const;
  QSet<QString> GetCoarseStudioNames(QString standardPs) const;

  QString operator()(const QString& sentence) const;  // from a file name
  QString operator[](const QString& nm) const {       // name standardlize not change sequence
    return ProStudioMap().value(nm.toLower(), nm);
  }

  static bool isHypenIndexValid(const QString& sentence, int& hypenIndex);
  static bool isHypenIndexValidReverse(const QString& sentence, int& hypenIndex);
  static constexpr int STUDIO_HYPEN_MAX_INDEX = 22;  // hypen index can be no greater then 22 char
 private:
  StudiosManager();  // valid localFilePath only used in llt
  StudiosManager(const StudiosManager& rhs) noexcept = delete;
  void InitializeImpl(const QString& path);
#ifdef RUNNING_UNIT_TESTS
  int ResetStateForTestImpl(const QString& localFilePath);
#endif

  QString FileName2StudioNameSection(QString sentence) const;
  QString FileNameLastSection2StudioNameSection(QString sentence) const;
  QString mLocalFilePath;
};

#endif  // STUDIOSMANAGER_H
