#ifndef CASTDBMODEL_H
#define CASTDBMODEL_H

#include <QFileInfo>
#include <QSqlTableModel>
#include <QDir>
class QAction;

class CastDbModel : public QSqlTableModel {
public:
  explicit CastDbModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase());

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

  QFileInfo fileInfo(const QModelIndex& curIndex) const {
    return QFileInfo{filePath(curIndex)};
  }

  QString oriPath(const QModelIndex& curIndex) const;

  QString psonFilePath(const QModelIndex& curIndex) const {
    return filePath(curIndex) + '/' + fileName(curIndex) + ".pson";
  }
  QString portaitPath(const QModelIndex& curIndex) const;

  bool submitAll();

  static constexpr int MAX_RATE{10};
private:  
  const QString m_imageHostPath;
  void onUpdateSubmitAllAction();
  QAction* mSubmitAllAction {nullptr};
};

#endif // CASTDBMODEL_H
