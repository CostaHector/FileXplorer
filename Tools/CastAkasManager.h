#ifndef CASTAKASMANAGER_H
#define CASTAKASMANAGER_H

#include <QHash>
#include <QString>

#include "SingletonManager.h"
using CAST_AKA_MGR_DATA_T = QHash<QString, QString>;
class CastAkasManager;
extern template class SingletonManager<CastAkasManager, CAST_AKA_MGR_DATA_T>;

class CastAkasManager final : public SingletonManager<CastAkasManager, CAST_AKA_MGR_DATA_T> {
 public:
  friend class SingletonManager<CastAkasManager, CAST_AKA_MGR_DATA_T>;
  CAST_AKA_MGR_DATA_T& CastAkaMap() { return data(); }
  const CAST_AKA_MGR_DATA_T& CastAkaMap() const { return data(); }

  QHash<QString, QString> ReadOutCastAkas() const;
  int ForceReloadImpl();

 private:
  CastAkasManager();
  CastAkasManager(const CastAkasManager& rhs) noexcept = delete;
  void InitializeImpl(const QString& path, const QString& blackPath="");
  QString mLocalFilePath;
};

#endif  // CASTAKASMANAGER_H
