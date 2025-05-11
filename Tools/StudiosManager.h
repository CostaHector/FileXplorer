#ifndef STUDIOSMANAGER_H
#define STUDIOSMANAGER_H

#include <QHash>
#include <QString>

class StudiosManager {
 public:
  static StudiosManager& getIns();
  StudiosManager(const StudiosManager& rhs) noexcept = delete;

  int LearningFromAPath(const QString& path, const bool bWriteInLocalFile = true, bool* bHasWrite = nullptr);
  int ForceReloadCast();

  QString ProductionStudioFilterOut(const QString& words) const;

  QStringList GetCoarseStudioNames(QString standardPs) const;

  QString hintStdStudioName(const QString& sentence) const;  // from a studio name

  QString operator()(const QString& sentence) const;  // from a file name
  QString operator[](const QString& nm) const { return m_prodStudioMap.value(nm.toLower(), nm); }

  int ForceReloadStudio();

  inline int count() const { return m_prodStudioMap.size(); }

  // coarse studio name -> std studio name
  // warner bros. => WarnerBros.
  // warnerbros. => WarnerBros.
  QHash<QString, QString> m_prodStudioMap;

 protected:
  static QHash<QString, QString> ReadOutStdStudioName();

 private:
  QString FileName2StudioNameSection(QString sentence) const;

  StudiosManager();
};

#endif  // STUDIOSMANAGER_H
