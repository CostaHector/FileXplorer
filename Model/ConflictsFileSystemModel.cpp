#include "ConflictsFileSystemModel.h"
using namespace FileOperatorType;

const QStringList ConflictsFileSystemModel::HORIZONTAL_HEADER_NAMES = {"name", "Delete Incoming", "size", "date", "size", "date"};

ConflictsFileSystemModel::ConflictsFileSystemModel(const ConflictsItemHelper& conflictItemsHelper,
                                                   const KEEP_PRIORITY keepPriority,
                                                   const bool revertKeepItemPriority,
                                                   QObject* parent)
    : QAbstractTableModel{parent},
      m_conflict{conflictItemsHelper},
      m_recycleMap{InitMap()},
      m_keepItemPriority{keepPriority},
      m_revertKeepItemPriority{revertKeepItemPriority} {
  updateKeepChoice();
}

QMap<int, ConflictItemsProperty> ConflictsFileSystemModel::InitMap() const {
  QMap<int, ConflictItemsProperty> mp;
  QSet<QString> commonSet{m_conflict.commonList.cbegin(), m_conflict.commonList.cend()};
  for (int i = 0; i < m_conflict.m_fromPathItems.size(); ++i) {
    if (commonSet.contains(m_conflict.m_fromPathItems[i])) {
      const QFileInfo lhsFi{m_conflict.l + '/' + m_conflict.m_fromPathItems[i]};
      const QFileInfo rhsFi{m_conflict.r + '/' + m_conflict.m_fromPathItems[i]};
      mp[i] = ConflictItemsProperty{
          m_conflict.m_fromPathItems[i],       true, lhsFi.size(), lhsFi.lastModified(), rhsFi.size(), rhsFi.lastModified(), lhsFi.isDir(),
          (lhsFi.isDir() + rhsFi.isDir() == 1)  // 0+0, 1+1=>ok, 0+1=>nok
      };
    }
  }
  return mp;
}

auto ConflictsFileSystemModel::data(const QModelIndex& index, int role) const -> QVariant {
  if (not index.isValid()) {
    return QVariant();
  }

  if (not(0 <= index.row() and index.row() < rowCount())) {
    return QVariant();
  }
  if (role == Qt::DisplayRole) {
    if (index.column() == 0) {
      return m_conflict.m_fromPathItems[index.row()];
    }
    if (not m_recycleMap.contains(index.row())) {
      return QVariant();
    }
    switch (index.column()) {
      case 1:
        return m_recycleMap[index.row()].recycleLeft;  // bool
      case 2:
        return m_recycleMap[index.row()].lSize;
      case 3:
        return m_recycleMap[index.row()].lTime;
      case 4:
        return m_recycleMap[index.row()].rSize;
      case 5:
        return m_recycleMap[index.row()].rTime;
      default:
        return QVariant();
    }
  } else if (role == Qt::DecorationRole) {
    if (index.column() == 0) {
      return m_iconProvider.icon(QFileInfo(m_conflict.l + '/' + m_conflict.m_fromPathItems[index.row()]));
    }
    return QVariant();
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() != 0) {
      // Size column, data column
      return Qt::AlignRight;
    }
    return int(Qt::AlignLeft | Qt::AlignTop);
  } else if (role == Qt::BackgroundRole) {
    if (m_recycleMap.contains(index.row())) {
      if (m_recycleMap[index.row()].recycleLeft) {
        if (index.column() == 2 or index.column() == 3) {
          return QBrush(Qt::GlobalColor::red);
        }
      } else {
        if (index.column() == 4 or index.column() == 5) {
          return QBrush(Qt::GlobalColor::red);
        }
      }
      return QBrush(Qt::GlobalColor::transparent);
    }
    return QBrush(Qt::GlobalColor::lightGray);
  }
  return QVariant();
}

auto ConflictsFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Orientation::Horizontal) {
      return HORIZONTAL_HEADER_NAMES[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QString ConflictsFileSystemModel::filePath(const QModelIndex& index) const {
  if (not index.isValid())
    return {};
  int row = index.row();
  int col = index.column();
  if (not(0 <= index.row() and index.row() < rowCount())) {
    return {};
  }
  switch (col) {
    case 2:
    case 3:
      return QDir(m_conflict.l).absoluteFilePath(m_conflict.m_fromPathItems[row]);
    case 4:
    case 5:
      if (m_recycleMap.contains(row)) {
        return QDir(m_conflict.r).absoluteFilePath(m_conflict.m_fromPathItems[row]);
      } else {
        return m_conflict.r;
      }
    default:
      return {};
  }
}

QString ConflictsFileSystemModel::displayCommands() const {
  return BatchCommands2String(m_cmds);
}

void ConflictsFileSystemModel::updateCommands() {
  m_cmds.clear();

  const auto OP = m_conflict.m_mode;
  const QString& l = m_conflict.l;
  const QString& r = m_conflict.r;

  for (int i = 0; i < rowCount(); ++i) {
    const QString& nm = m_conflict.m_fromPathItems[i];
    if (m_recycleMap.contains(i)) {
      const auto& conflictItems = m_recycleMap[i];
      m_cmds.append(ACMD{MOVETOTRASH, {conflictItems.recycleLeft ? l : r, nm}});
      if (conflictItems.recycleLeft) {
        // don't need to do anything
        continue;
      }
    }
    switch (OP) {
      case CCMMode::CUT_OP:
      case CCMMode::MERGE_OP:
        m_cmds.append(ACMD{RENAME, {l, nm, r, nm}});
        break;
      case CCMMode::COPY_OP: {
        if (QFileInfo(l, nm).isDir()) {
          m_cmds.append(ACMD{MKPATH, {r, nm}});
        } else {
          m_cmds.append(ACMD{CPFILE, {l, nm, r}});
        }
        break;
      }
      case CCMMode::LINK_OP:
        m_cmds.append(ACMD{LINK, {l, nm, r}});
      default:
        break;
    }
  }
  if (OP == CCMMode::MERGE_OP) {
    m_cmds.append(ACMD{RMPATH, {"", l}});  // when merge A to B, folder A need to removed
  }
}

void ConflictsFileSystemModel::setDeleteOrRecycle(const QModelIndexList& indLst, bool isSetDelete) {
  // keep both side(not support now)
  // recyle both side(not support now)
  for (const auto& ind : indLst) {
    const int row = ind.row();
    const int col = ind.column();
    if (not m_recycleMap.contains(row)) {
      continue;
    }
    if (col < 2) {
      continue;  // ignore the first two column(name, and recycle Left)
    }
    const bool beforeRecycleSide = m_recycleMap[row].recycleLeft;
    switch (col) {
      case 2:
      case 3:
        m_recycleMap[row].recycleLeft = isSetDelete;
        break;
      case 4:
      case 5:
        m_recycleMap[row].recycleLeft = not isSetDelete;
        break;
      default:;
    }
    if (beforeRecycleSide != m_recycleMap[row].recycleLeft) {
      emit dataChanged(index(row, 1), index(row, 1), {Qt::DisplayRole});
      emit dataChanged(index(row, 2), index(row, 5), {Qt::BackgroundRole});
    }
  }
}
