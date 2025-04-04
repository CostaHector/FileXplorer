#ifndef FLOATINGMODELS_H
#define FLOATINGMODELS_H

#include "QAbstractListModelPub.h"
#include <QPixmapCache>

class FloatingModels : public QAbstractListModelPub {
 public:
  explicit FloatingModels(QObject* object = nullptr) : QAbstractListModelPub{object} {}
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return m_curLoadedCount; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  virtual int UpdateData(const QStringList& newDataLst);  // load all in a time
  QString filePath(const QModelIndex& index) const;
  bool IsEmpty() const { return mDataLst.isEmpty(); }
  void Clear() { UpdateData({}); }
 public slots:
  int setDirPath(const QString& path, const QStringList& sFilters, bool loadAllIn1Time = true);

 protected:
  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;
  inline bool isOuterBound(const int& r) const { return r < 0 || r >= mDataLst.size(); }

  QStringList mDataLst;
  int m_curLoadedCount{0};  // already loaded items count
  static constexpr int BATCH_LOAD_COUNT = 8;
};

class ImgsModel : public FloatingModels {
 public:
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

 private:
  QPixmapCache mPixCache;
};

class VidsModel : public FloatingModels {
 public:
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
};

class OthersModel : public VidsModel {};
#endif  // FLOATINGMODELS_H
