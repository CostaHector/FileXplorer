#ifndef PRODUCTIONSTUDIOMANAGER_H
#define PRODUCTIONSTUDIOMANAGER_H

#include <QMessageBox>
#include <QRegExp>
#include <QSet>
#include <QVariantHash>
#include <QWidget>

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

  inline int count() const { return m_prodStudioMap.size(); }
  inline void DisplayStatistic(QWidget* parent) { QMessageBox::information(parent, "Studios Count", QString::number(count())); }

 protected:
  static QVariantHash ReadOutStdStudioName();
  QVariantHash m_prodStudioMap;

 private:
  ProductionStudioManager();
};

#endif  // PRODUCTIONSTUDIOMANAGER_H
