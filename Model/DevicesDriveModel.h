#ifndef DEVICESDRIVEMODEL_H
#define DEVICESDRIVEMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>

class DevicesDriveModel : public QSqlTableModel {
 public:
  explicit DevicesDriveModel(QObject* parent = nullptr, QSqlDatabase con = QSqlDatabase());

  QVariant data(const QModelIndex& idx, int role = Qt::DisplayRole) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QString GetRootPath(const QModelIndex& idx) const;
  QString GetGuid(const QModelIndex& idx) const;
  QString GetMountedPoint(const QModelIndex& idx) const;
};

#endif // DEVICESDRIVEMODEL_H
