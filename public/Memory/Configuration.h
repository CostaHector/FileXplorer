#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QSettings>
#include "KV.h"

static inline QSettings& Configuration() {
#ifndef RUNNING_UNIT_TESTS
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Costa", PROJECT_NAME);
#else
  static QSettings settings(QSettings::IniFormat, QSettings::UserScope, "TestOnly", PROJECT_NAME);
#endif
  return settings;
}

inline QVariant getConfig(const KV& kv) {
  static const auto& cfg = Configuration();
  return cfg.value(kv.name, kv.toVariant());
}

inline void setConfig(const KV& kv, const QVariant& newVar) {
  const bool needCallBack{kv.changedCallback != nullptr && newVar != getConfig(kv)};
  static auto& cfg = Configuration();
  cfg.setValue(kv.name, newVar);
  if (needCallBack) {
    kv.changedCallback(newVar);
  }
}

#endif // CONFIGURATION_H
