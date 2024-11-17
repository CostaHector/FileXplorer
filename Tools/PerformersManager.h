#ifndef PERFORMERSMANAGER_H
#define PERFORMERSMANAGER_H

#include <QCompleter>
#include <QMessageBox>
#include <QSet>
#include <QString>
#include <QWidget>

class PerformersManager {
 public:
  static PerformersManager& getIns();
  PerformersManager(const PerformersManager& rhs) noexcept = delete;

  QSet<QString> ReadOutPerformers();
  int ForceReloadPerformers();

  auto LearningFromAPath(const QString& path) -> int;

  static auto RmvBelongLetter(const QString& word) -> QString;
  static QStringList SplitSentence(QString sentence);
  QStringList FilterPerformersOut(const QStringList& words) const;
  auto operator()(const QString& sentence) const -> QStringList { return FilterPerformersOut(SplitSentence(sentence)); }
  QSet<QString> m_performers;
  QCompleter perfsCompleter;

  inline int count() const { return m_performers.size(); }

 private:
  PerformersManager();
};

#endif  // PERFORMERSMANAGER_H
