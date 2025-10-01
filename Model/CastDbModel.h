#ifndef CASTDBMODEL_H
#define CASTDBMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include <QDir>
#include <QItemSelectionModel>

class CastDbModel : public QSqlTableModel {
public:
  explicit CastDbModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());
  static bool isDbValidAndOpened(const QSqlDatabase& db);

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
    if (orientation == Qt::Vertical && role == Qt::TextAlignmentRole) {
      return Qt::AlignRight;
    }
    return QSqlTableModel::headerData(section, orientation, role);
  }

  QDir rootDirectory(const QString& /*placeHolder*/ = "") const { return QDir{rootPath()}; }

  QString rootPath(const QString& /*placeHolder*/ = "") const { return m_imageHostPath; }

  QString fileName(const QModelIndex& curIndex) const;

  QString filePath(const QModelIndex& curIndex) const;

  QFileInfo fileInfo(const QModelIndex& curIndex) const;

  QString oriPath(const QModelIndex& curIndex) const;

  QString psonFilePath(const QModelIndex& curIndex) const;

  QModelIndexList GetAllRowsIndexes() const;
  int SyncImageFieldsFromImageHost(const QModelIndexList& selectedRows);
  int DumpRecordsIntoPsonFile(const QModelIndexList& selectedRows);
  int DeleteSelectionRange(const QItemSelection& selectionRangeList);
  int RefreshVidsForRecords(const QModelIndexList& indices, QSqlDatabase videoDb);
  int MigrateCastsTo(const QModelIndexList& selectedRows, const QString& destinationPath);

  bool submitSaveAllChanges();
  bool repopulate();
  bool onRevert();

  static constexpr int MAX_RATE{10};
private:  
  const QString m_imageHostPath;
};

#endif // CASTDBMODEL_H
