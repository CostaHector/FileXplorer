#ifndef LOGMODEL_H
#define LOGMODEL_H

#include "Tools/Log.h"
#include <QAbstractTableModel>

// curTime, "Debug", qPrintable(msg), context.file).arg(context.line).arg(context.function);
class LogModel : public QAbstractTableModel {
 public:
  explicit LogModel(QObject* parent = nullptr);
  const QString& rootPath() const { return m_rootPath; }

  void _reloadLogFiles();

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return mlogs.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return Log::INDEX2COLUMNNAME.size(); }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return Log::INDEX2COLUMNNAME[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  QString getFileNameAndLineNo(const QModelIndex& ind) const;

  QString fullInfo(const QModelIndex& ind) const;
 private:
  QString m_rootPath = "logs_info.log";
  QList<Log> mlogs;
};

#endif  // LOGMODEL_H
