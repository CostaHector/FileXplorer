#ifndef JSONTABLEMODEL_H
#define JSONTABLEMODEL_H
#include "QAbstractTableModelPub.h"
#include "Tools/Json/JsonPr.h"

class JsonTableModel : public QAbstractTableModelPub {
 public:
  JsonTableModel(QObject* object = nullptr);
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return mCachedJsons.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return JsonKey::JSON_KEY_E::JSON_KEY_BUTT; }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& /*index*/) const override {
    return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  int ReadADirectory(const QString& path);
  int AppendADirectory(const QString& path);

 private:

  void setModified(int row, bool modified = true);

  QVector<JsonPr> mCachedJsons;
  QSet<int> m_modifiedRows;
};

#endif  // JSONTABLEMODEL_H
