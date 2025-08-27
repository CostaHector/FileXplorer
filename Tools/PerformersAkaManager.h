#ifndef PERFORMERSAKAMANAGER_H
#define PERFORMERSAKAMANAGER_H

#include <QHash>
#include <QMessageBox>
#include <QSqlRecord>
#include <QStack>
#include <QString>
#include <QWidget>

class PerformersAkaManager {
public:
  static PerformersAkaManager& getIns();
  PerformersAkaManager(const PerformersAkaManager& rhs) noexcept = delete;

  QHash<QString, QString> ReadOutAkaName();
  int ForceReloadAkaName();

  static int UpdateAKAHash(const bool isForce = false);
  QHash<QString, QString> m_akaPerf;

  inline int count() const { return m_akaPerf.size(); }

private:
  PerformersAkaManager();
};

#endif  // PERFORMERSAKAMANAGER_H
