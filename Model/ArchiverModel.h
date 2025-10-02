#ifndef ARCHIVERMODEL_H
#define ARCHIVERMODEL_H

#include "QAbstractTableModelPub.h"
#include "ArchiveFiles.h"

class ArchiverModel : public QAbstractTableModelPub {
 public:
  using QAbstractTableModelPub::QAbstractTableModelPub;
  int rowCount(const QModelIndex& /*parent*/ = {}) const override { return m_paf.size(); }
  int columnCount(const QModelIndex& /*parent*/ = {}) const override { return ARCHIVE_HORIZONTAL_HEADER.size(); }
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  void setRootPath(const QString& qzPath);

  QString GetRelativeName(int r) const;
  const QByteArray& GetByteArrayData(int r) const;

 private:
  ArchiveFilesReader m_paf;
  static const QStringList ARCHIVE_HORIZONTAL_HEADER;
};

#endif  // ARCHIVERMODEL_H
