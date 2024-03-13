#ifndef LOGMODEL_H
#define LOGMODEL_H

#include <QAbstractTableModel>

struct Log {
 public:
  explicit Log(const QStringList& lst) : time{lst[0]}, level{lst[1]}, msg{lst[2]}, fileName{lst[3]}, lineNo{lst[4]}, funcName{lst[5]} {}
  explicit Log(const QString& log) : Log(log.split('\n')) {}

  QString time;
  QString level;
  QString msg;
  QString fileName;
  QString lineNo;
  QString funcName;
};

const QStringList INDEX2COLUMNNAME = {"time", "level", "msg", "fileName", "lineNo", "funcName"};
// curTime, "Debug", qPrintable(msg), context.file).arg(context.line).arg(context.function);
class LogModel : public QAbstractTableModel {
 public:
  explicit LogModel(QObject* parent = nullptr);
  void _reloadLogFiles();

  auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override { return mlogs.size(); }
  auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override { return INDEX2COLUMNNAME.size(); }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

  auto headerData(int section, Qt::Orientation orientation, int role) const -> QVariant override {
    if (role == Qt::TextAlignmentRole) {
      if (orientation == Qt::Vertical) {
        return Qt::AlignRight;
      }
    }
    if (role == Qt::DisplayRole) {
      if (orientation == Qt::Orientation::Horizontal) {
        return INDEX2COLUMNNAME[section];
      }
      return section + 1;
    }
    return QAbstractTableModel::headerData(section, orientation, role);
  }

  QString getFileNameAndLineNo(const QModelIndex& ind) const;

 private:
  QString m_rootPath = "logs_info.log";
  QList<Log> mlogs;
};

#endif  // LOGMODEL_H
