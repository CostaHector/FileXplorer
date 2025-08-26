#ifndef CASTDBMODEL_H
#define CASTDBMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include <QDir>

class CastDbModel : public QSqlTableModel {
public:
  explicit CastDbModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
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

  QFileInfo fileInfo(const QModelIndex& curIndex) const {
    return QFileInfo{filePath(curIndex)};
  }

  QString psonFilePath(const QModelIndex& curIndex) const {
    return filePath(curIndex) + '/' + fileName(curIndex) + ".pson";
  }

  static constexpr int MAX_RATE{10};
private:
  const QString m_imageHostPath;
};

#endif // CASTDBMODEL_H
