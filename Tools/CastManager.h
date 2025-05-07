#ifndef CastManager_H
#define CastManager_H

#include <QCompleter>
#include <QSet>
#include <QString>

class CastManager {
 public:
  static CastManager& getIns();
  CastManager(const CastManager& rhs) noexcept = delete;

  QSet<QString> ReadOutPerformers();
  int ForceReloadPerformers();

  auto LearningFromAPath(const QString& path) -> int;

  static auto RmvBelongLetter(const QString& word) -> QString;
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  QStringList operator()(const QString& sentence) const;
  QSet<QString> m_performers;
  QCompleter perfsCompleter;

  inline int count() const { return m_performers.size(); }

 private:
  CastManager();
};

#endif  // CastManager_H
