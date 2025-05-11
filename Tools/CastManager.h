#ifndef CastManager_H
#define CastManager_H

#include <QSet>
#include <QString>

class CastManager {
 public:
  static CastManager& getIns();
  CastManager(const CastManager& rhs) noexcept = delete;

  QSet<QString> ReadOutPerformers();
  int ForceReloadCast();

  int LearningFromAPath(const QString& path, const bool bWriteInLocalFile=true, bool* bHasWrite=nullptr);

  static QString RmvBelongLetter(const QString& word);
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  QStringList operator()(const QString& sentence) const;
  QSet<QString> m_performers;

  inline int count() const { return m_performers.size(); }

 private:
  CastManager();
  static const QRegularExpression EFFECTIVE_CAST_NAME;
  static constexpr int EFFECTIVE_CAST_NAME_LEN = 12;
};

#endif  // CastManager_H
