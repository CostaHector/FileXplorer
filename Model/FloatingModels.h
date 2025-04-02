#ifndef FLOATINGMODELS_H
#define FLOATINGMODELS_H

#include "QAbstractListModelPub.h"

class FloatingModels : public QAbstractListModelPub {
 public:
  explicit FloatingModels(QObject* object = nullptr) : QAbstractListModelPub{object} {}
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mDataLst.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual void UpdateData(const QStringList& newDataLst);  // load all in a time
  QString filePath(const QModelIndex& index) const;

 protected:
  inline bool isOuterBound(const int& r) const { return r < 0 || r >= mDataLst.size(); }
  QStringList mDataLst;
  int m_curLoadedCount{0}; // already loaded items count
};

class ImgsModel : public FloatingModels {
 public:
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return m_curLoadedCount; }

 public slots:
  void setDirPath(const QString& path);

 protected:
  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;
  static constexpr int BATCH_LOAD_COUNT = 8;
};

class VidsModel : public FloatingModels {
 public:
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

class OthersModel : public VidsModel {};
#endif  // FLOATINGMODELS_H
