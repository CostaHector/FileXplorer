#ifndef PERFORMERSMANAGER_H
#define PERFORMERSMANAGER_H

#include <QSet>
#include <QString>
#include <QCompleter>

class PerformersManager {
 public:
  PerformersManager(const PerformersManager& rhs) noexcept = delete;
  auto loadExistedPerformers() -> QSet<QString>;
  auto LearningFromAPath(const QString& path) -> int;
  static PerformersManager& getIns();
  QStringList MovieNameWordsSplit(QString sentence) const;
  QStringList PeformersFilterOut(const QStringList& words) const;
  auto operator()(const QString& sentence) const -> QStringList{
    return PeformersFilterOut(MovieNameWordsSplit(sentence));
  }
  QSet<QString> m_performers;
  QCompleter perfsCompleter;
 private:
  PerformersManager();
};

#endif  // PERFORMERSMANAGER_H
