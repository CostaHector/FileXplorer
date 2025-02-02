#ifndef HARMODEL_H
#define HARMODEL_H

#include "Tools/QAbstractTableModelPub.h"
#include "Tools/HarParserHelper.h"
#include <QItemSelectionModel>
#include <QFileIconProvider>

class HarModel : public QAbstractTableModelPub {
 public:
  explicit HarModel(QObject* parent = nullptr);

  auto rowCount(const QModelIndex& parent = {}) const -> int override { return mHarParser.size(); }
  auto columnCount(const QModelIndex& parent = {}) const -> int override { return HAR_VERTICAL_HEAD.size(); }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Vertical) {
        return section + 1;
      } else {
        return HAR_VERTICAL_HEAD[section];
      }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  Qt::ItemFlags flags(const QModelIndex& index) const override {
    if (index.column() == 2) {
      return Qt::ItemFlag::ItemIsEditable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
    }
    return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable;
  }

  int SetHarFileAbsPath(const QString& harFileAbsPath);
  int SaveToLocal(QString dstRootpath = "", const QList<int>& selectedRows = {});
  QByteArray GetImageByteArray(const int rowIndex, QString& fileName) const;
 private:
  HarParserHelper mHarParser;
  QFileIconProvider m_iconProvider;
};

#endif // HARMODEL_H
