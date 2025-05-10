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

  int LearningFromAPath(const QString& path);

  static QString RmvBelongLetter(const QString& word);
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  QStringList operator()(const QString& sentence) const;
  QSet<QString> m_performers;
  QCompleter perfsCompleter;

  inline int count() const { return m_performers.size(); }

 private:
  CastManager();
  static const QRegularExpression EFFECTIVE_CAST_NAME;
  static constexpr int EFFECTIVE_CAST_NAME_LEN = 12;
};

#endif  // CastManager_H
