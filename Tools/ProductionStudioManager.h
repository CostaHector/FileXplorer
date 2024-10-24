#ifndef PRODUCTIONSTUDIOMANAGER_H
#define PRODUCTIONSTUDIOMANAGER_H

#include <QMessageBox>
#include <QRegularExpression>
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

  QString hintStdStudioName(const QString& sentence) const;// from a studio name

  QString operator()(const QString& sentence) const; // from a file name
  QString operator[](const QString& nm) const {
    auto it = m_prodStudioMap.find(nm.toLower());
    return it != m_prodStudioMap.cend() ? it.value().toString() : nm;
  }

  int ForceReloadStdStudioName();

  inline int count() const { return m_prodStudioMap.size(); }
  inline void DisplayStatistic(QWidget* parent) { QMessageBox::information(parent, "Studios Count", QString::number(count())); }

 protected:
  static QVariantHash ReadOutStdStudioName();
  QVariantHash m_prodStudioMap;

 private:
  QString FileName2StudioNameSection(QString sentence) const;

  ProductionStudioManager();
};

#endif  // PRODUCTIONSTUDIOMANAGER_H
