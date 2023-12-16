#ifndef PERFORMERSSTRINGPARSER_H
#define PERFORMERSSTRINGPARSER_H

#include <QSet>
#include <QString>
#include <QCompleter>

class PerformersStringParser {
 public:
  PerformersStringParser(const PerformersStringParser& rhs) noexcept = delete;
  auto loadExistedPerformers() -> QSet<QString>;
  auto LearningFromAPath(const QString& path) -> int;
  static PerformersStringParser& getIns();

  static auto RmvBelongLetter(const QString& word) -> QString;
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  auto operator()(const QString& sentence) const -> QStringList{
    return FilterPerformersOut(SplitSentence(sentence));
  }
  QSet<QString> m_performers;
  QCompleter perfsCompleter;
 private:
  PerformersStringParser();
};

#endif  // PERFORMERSSTRINGPARSER_H
