#include "LogModel.h"
#include <QBrush>
#include <QColor>
#include <QFile>
#include <QIODevice>
#include <QTextStream>

LogModel::LogModel(QObject* parent) : QAbstractTableModel{parent} {}

void LogModel::_reloadLogFiles() {
  QFile fi(m_rootPath);
  if (not fi.exists()) {
    return;
  }
  if (not fi.open(QIODevice::Text | QIODevice::ReadOnly)) {
    return;
  }
  QTextStream ts(&fi);
  decltype(mlogs) tempLogs;
  while (not ts.atEnd()) {
    tempLogs.append(Log(ts.readLine()));
  }
  // equal
  if (mlogs.size() == tempLogs.size()) {
    mlogs.swap(tempLogs);
    fi.close();
    return;
  }
  // isInc
  if (mlogs.size() < tempLogs.size()) {
    beginInsertRows(QModelIndex(), mlogs.size(), tempLogs.size() - 1);  // [begin, newSize-1]
    mlogs.swap(tempLogs);
    endInsertRows();
    fi.close();
    return;
  }
  // isDec
  fi.close();
  beginRemoveRows(QModelIndex(), tempLogs.size(), mlogs.size() - 1);
  mlogs.swap(tempLogs);
  endRemoveRows();
}

auto LogModel::data(const QModelIndex& index, int role) const -> QVariant {
  if (not index.isValid()) {
    return QVariant();
  }
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    const auto& record = mlogs[index.row()];
    const int section = index.column();
    switch (section) {
      case 0:
        return record.time;
      case 1:
        return record.level;
      case 2:
        return record.msg;
      case 3:
        return record.fileName;
      case 4:
        return record.lineNo;
      case 5:
        return record.funcName;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    return QVariant();
  } else if (role == Qt::BackgroundRole) {
    return Log::GetColor(mlogs[index.row()].level);
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 2) {
      return int(Qt::AlignLeft | Qt::AlignTop);
    }
    return int(Qt::AlignRight | Qt::AlignVCenter);
  }
  return QVariant();
}

QString LogModel::getFileNameAndLineNo(const QModelIndex& ind) const {
  if (not ind.isValid()) {
    return {};
  }
  const auto& record = mlogs[ind.row()];
  return record.fileName.mid(1) + ":" + QString::number(record.lineNo);
}

QString LogModel::fullInfo(const QModelIndex& ind) const {
  if (not ind.isValid()) {
    return {};
  }
  const auto& record = mlogs[ind.row()];
  return record.time + '\n' + record.level + '\n' + record.fileName + ':' + QString::number(record.lineNo) + '\n' + record.funcName + '\n' + record.msg;
}
