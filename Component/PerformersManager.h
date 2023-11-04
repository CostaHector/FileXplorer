#ifndef PERFORMERSMANAGER_H
#define PERFORMERSMANAGER_H

#include <QSet>
#include <QString>

class PerformersManager {
 public:
  PerformersManager(const PerformersManager& rhs) noexcept = delete;
  auto loadExistedPerformers() -> QSet<QString>;
  auto LearningFromAPath(const QString& path) -> int;
  static PerformersManager& getIns();

  QSet<QString> performers;

 private:
  PerformersManager();
};

#endif  // PERFORMERSMANAGER_H
