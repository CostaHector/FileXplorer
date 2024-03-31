#ifndef TOCONSECUTIVEFILENAMENO_H
#define TOCONSECUTIVEFILENAMENO_H

#include <QRegularExpression>
#include <QString>
#include <QStringList>

class ToConsecutiveFileNameNo {
 public:
  explicit ToConsecutiveFileNameNo(const int noStart = 0) : m_noStart(noStart) {}

  QStringList operator()(const QStringList& names);

  static bool nameNeedToProcess(const QString& s, QString& sTemplate, const int fileNo = 0);

  static const QRegularExpression EXCEPTION_FILE_NAME_NO_PATTERN;
  static const QRegularExpression FILE_NAME_NO_PATTERN;
  static constexpr int FILE_NO_START_INDEX = 0;

 private:
  const int m_noStart;
};

#endif  // TOCONSECUTIVEFILENAMENO_H
