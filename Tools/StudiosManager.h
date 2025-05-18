#ifndef STUDIOSMANAGER_H
#define STUDIOSMANAGER_H

#include <QHash>
#include <QString>

class StudiosManager {
 public:
  static StudiosManager& getIns();
  StudiosManager(const StudiosManager& rhs) noexcept = delete;

  QHash<QString, QString> ReadOutStdStudioName() const;
  int ForceReloadStudio();

  int LearningFromAPath(const QString& path, bool* bHasWrite = nullptr);
  int StudioIncrement(QHash<QString, QString>& increments, const QString& newStudio);
  int WriteIntoLocalDictionaryFiles(const QHash<QString, QString>& increments) const;

  QString ProductionStudioFilterOut(const QString& words) const;
  QSet<QString> GetCoarseStudioNames(QString standardPs) const;

  QString operator()(const QString& sentence) const;  // from a file name
  QString operator[](const QString& nm) const { return m_prodStudioMap.value(nm.toLower(), nm); }

  inline int count() const { return m_prodStudioMap.size(); }

  // coarse studio name -> std studio name
  // warner bros. => WarnerBros.
  // warnerbros. => WarnerBros.
  QHash<QString, QString> m_prodStudioMap;

  static bool isHypenIndexValid(const QString& sentence, int& hypenIndex);
  static constexpr int STUDIO_HYPEN_MAX_INDEX = 22;  // hypen index can be no greater then 22 char
 private:
  QString FileName2StudioNameSection(QString sentence) const;

  static QString GetLocalFilePath(const QString& localFilePath);
  StudiosManager(const QString& localFilePath = ""); // valid localFilePath only used in llt
  QString mLocalFilePath;
};

#endif  // STUDIOSMANAGER_H
