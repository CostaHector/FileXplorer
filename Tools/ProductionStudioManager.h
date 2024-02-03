#ifndef PRODUCTIONSTUDIOMANAGER_H
#define PRODUCTIONSTUDIOMANAGER_H

#include <QRegExp>
#include <QSet>
#include <QVariantHash>

class ProductionStudioManager {
 public:
  static ProductionStudioManager& getIns();
  ProductionStudioManager(const ProductionStudioManager& rhs) noexcept = delete;

  auto LearningFromAPath(const QString& path) -> int;

  QString ProductionStudioFilterOut(const QString& words) const;

  QStringList StandardProductionStudioFrom(QString standardPs) const;

  auto operator()(QString sentence) const -> QString;
  auto operator[](const QString& nm) const -> QString {
    return m_prodStudioMap.contains(nm.toLower()) ? m_prodStudioMap[nm.toLower()].toString() : nm;
  }

  int ForceReloadStdStudioName();

 protected:
  static QVariantHash ReadOutStdStudioName();
  QVariantHash m_prodStudioMap;

 private:
  ProductionStudioManager();
};

#endif  // PRODUCTIONSTUDIOMANAGER_H
