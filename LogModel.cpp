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
    const QStringList& lns = ts.readLine().split('\t');
    tempLogs.append(Log(lns));
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
    const auto& record = mlogs[index.row()];
    if (record.level == "Info")
      return QBrush(QColor(0xFF, 0xFF, 0xFF));
    else if (record.level == "Debug")
      return QBrush(QColor(0xC0, 0xC0, 0xC0));
    else if (record.level == "Warning")
      return QBrush(QColor(0xFF, 0xBF, 0x00));
    else if (record.level == "Fatal")
      return QBrush(QColor(0xFF, 0x7F, 0x50));
    return QVariant();
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == 1) {
      return int(Qt::AlignRight | Qt::AlignVCenter);
    }
    return int(Qt::AlignLeft | Qt::AlignVCenter);
  }
  return QVariant();
}

QString LogModel::getFileNameAndLineNo(const QModelIndex& ind) const {
  if (not ind.isValid()) {
    return {};
  }
  const auto& record = mlogs[ind.row()];
  return record.fileName.mid(1) + ":" + record.lineNo;
}
