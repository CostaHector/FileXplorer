#ifndef CastManager_H
#define CastManager_H

#include <QSet>
#include <QString>

class CastManager {
 public:
  static CastManager& getIns();
  CastManager(const CastManager& rhs) noexcept = delete;

  QSet<QString> ReadOutPerformers() const;
  int ForceReloadCast();

  int LearningFromAPath(const QString& path, bool* bHasWrite = nullptr);
  int CastIncrement(QSet<QString>& increments, QSet<QString> delta);
  int WriteIntoLocalDictionaryFiles(const QSet<QString>& increments) const;

  static QString RmvBelongLetter(const QString& word);
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  QStringList operator()(const QString& sentence) const;
  QSet<QString> m_casts;

  inline int count() const { return m_casts.size(); }

 private:
  static const QRegularExpression EFFECTIVE_CAST_NAME;
  static constexpr int EFFECTIVE_CAST_NAME_LEN = 12;

  static QString GetLocalFilePath(const QString& localFilePath);
  CastManager(const QString& localFilePath = "");  // valid localFilePath only used in llt
  const QString mLocalFilePath;
};

#endif  // CastManager_H
