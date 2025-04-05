#ifndef CONFLICTSFILESYSTEMMODEL_H
#define CONFLICTSFILESYSTEMMODEL_H

#include <QAbstractTableModel>
#include <QBrush>
#include <QDateTime>
#include <QFile>
#include <QFileIconProvider>
#include <QFileInfo>

#include "FileOperation/FileOperatorPub.h"
#include "Tools/ConflictsItemHelper.h"

struct ConflictItemsProperty {
  QString name;
  bool recycleLeft;  // true for recycle left, false for recycle right
  qint64 lSize;
  QDateTime lTime;
  qint64 rSize;
  QDateTime rTime;
  bool isDir;
  bool lrSameType;  // if not same type, weired name error;
};

class ConflictsFileSystemModel : public QAbstractTableModel {
 public:
  enum class KEEP_PRIORITY {
    LARGE_SIZE = 0,
    NEWER_TIME = 1,
    SIZE_LARGE_THEN_NEWER_TIME = 2,
    NEWER_TIME_THEN_SIZE_LARGE = 3,
    ALWAYS_SIDE_LEFT = 4,
    ALWAYS_SIDE_RIGHT = 5
  };

  explicit ConflictsFileSystemModel(const ConflictsItemHelper& conflictItemsHelper,
                                    const KEEP_PRIORITY keepPriority = KEEP_PRIORITY::SIZE_LARGE_THEN_NEWER_TIME,
                                    const bool revertKeepItemPriority = false,
                                    QObject* parent = nullptr);

  QMap<int, ConflictItemsProperty> InitMap() const;

  explicit ConflictsFileSystemModel(const ConflictsItemHelper& conflictItemsHelper, QObject* parent)
      : ConflictsFileSystemModel{conflictItemsHelper, KEEP_PRIORITY::SIZE_LARGE_THEN_NEWER_TIME, false, parent} {}

  void setKeepItemPriority(KEEP_PRIORITY keepItemPriority) {
    m_keepItemPriority = keepItemPriority;
    updateKeepChoice();
  }

  void setRevertKeepItemPriority(bool revertKeepItemPriority) {
    m_revertKeepItemPriority = revertKeepItemPriority;
    for (auto& property : m_recycleMap) {
      property.recycleLeft = not property.recycleLeft;
    }
    emit dataChanged(index(0, 1), index(rowCount() - 1, 1), {Qt::DisplayRole});
    emit dataChanged(index(0, 2), index(rowCount() - 1, columnCount() - 1), {Qt::BackgroundRole});
  }

  void updateKeepChoice() {
    for (auto& property : m_recycleMap) {
      property.recycleLeft = isRecycleLeft(property);
    }
    emit dataChanged(index(0, 1), index(rowCount() - 1, 1), {Qt::DisplayRole});
    emit dataChanged(index(0, 2), index(rowCount() - 1, columnCount() - 1), {Qt::BackgroundRole});
  }

  bool isRecycleLeft(const ConflictItemsProperty& property) const {
    if (property.isDir) {
      return true;
    }

    bool recycleLeft = true;
    switch (m_keepItemPriority) {
      case KEEP_PRIORITY::LARGE_SIZE:
        // property.lSize < property.rSize
        // property.lSize == property.rSize // to contains fewer commands
        recycleLeft = property.lSize <= property.rSize;
        break;
      case KEEP_PRIORITY::NEWER_TIME:
        recycleLeft = property.lTime <= property.rTime;
        break;
      case KEEP_PRIORITY::SIZE_LARGE_THEN_NEWER_TIME:
        recycleLeft = property.lSize < property.rSize or (property.lSize == property.rSize and property.lTime <= property.rTime);
        break;
      case KEEP_PRIORITY::NEWER_TIME_THEN_SIZE_LARGE:
        recycleLeft = property.lTime < property.rTime or (property.lTime == property.rTime and property.lSize <= property.rSize);
        break;
      case KEEP_PRIORITY::ALWAYS_SIDE_LEFT:
        recycleLeft = false;
        break;
      case KEEP_PRIORITY::ALWAYS_SIDE_RIGHT:
        recycleLeft = true;
        break;
      default:
        recycleLeft = true;
    }
    return m_revertKeepItemPriority ? not recycleLeft : recycleLeft;
  }

  auto rowCount(const QModelIndex& /*parent*/ = {}) const -> int override { return m_conflict.m_fromPathItems.size(); }
  auto columnCount(const QModelIndex& /*parent*/ = {}) const -> int override { return HORIZONTAL_HEADER_NAMES.size(); }

  auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;
  auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override;

  QString filePath(const QModelIndex& index) const;

  QString displayCommands() const;
  FileOperatorType::BATCH_COMMAND_LIST_TYPE getCommands() const { return m_cmds; }
  void updateCommands();
  bool isCommandsAvail() const { return not m_cmds.isEmpty(); }

  void setDeleteOrRecycle(const QModelIndexList& indLst, bool isSetDelete = true);

  QMap<int, ConflictItemsProperty> getConflictMap() const { return m_recycleMap; }

 private:
  static const QStringList HORIZONTAL_HEADER_NAMES;

  QFileIconProvider m_iconProvider;
  ConflictsItemHelper m_conflict;

  QMap<int, ConflictItemsProperty> m_recycleMap;

  KEEP_PRIORITY m_keepItemPriority;
  bool m_revertKeepItemPriority;

  FileOperatorType::BATCH_COMMAND_LIST_TYPE m_cmds;
};

#endif  // CONFLICTSFILESYSTEMMODEL_H
