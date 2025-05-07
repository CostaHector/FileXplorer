#ifndef STUDIOSMANAGER_H
#define STUDIOSMANAGER_H

#include <QMessageBox>
#include <QRegularExpression>
#include <QSet>
#include <QVariantHash>
#include <QWidget>

class StudiosManager {
 public:
  static StudiosManager& getIns();
  StudiosManager(const StudiosManager& rhs) noexcept = delete;

  auto LearningFromAPath(const QString& path) -> int;

  QString ProductionStudioFilterOut(const QString& words) const;

  QStringList StandardProductionStudioFrom(QString standardPs) const;

  QString hintStdStudioName(const QString& sentence) const;  // from a studio name

  QString operator()(const QString& sentence) const;  // from a file name
  QString operator[](const QString& nm) const { return m_prodStudioMap.value(nm.toLower(), nm); }

  int ForceReloadStdStudioName();

  inline int count() const { return m_prodStudioMap.size(); }

 protected:
  static QHash<QString, QString> ReadOutStdStudioName();
  QHash<QString, QString> m_prodStudioMap;

 private:
  QString FileName2StudioNameSection(QString sentence) const;

  StudiosManager();
};

#endif  // STUDIOSMANAGER_H
