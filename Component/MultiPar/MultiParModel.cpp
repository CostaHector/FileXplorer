#include "MultiParModel.h"
#include "ImageTool.h"
#include "SystemPath.h"
#include "NotificatorMacro.h"
#include "MultiParTools.h"
#include "Par2Tools.h"
#include "PathTool.h"
#include "UndoRedo.h"
#include "FileOperatorPub.h"

MultiParModel::MultiParModel(ParVerifyInfomationList&& resultList, QObject* parent)
  : QAbstractTableModelPub{parent}
  , mVerifyInfoList{std::move(resultList)} {}

QVariant MultiParModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (role == Qt::TextAlignmentRole) {
    if (orientation == Qt::Vertical) {
      return Qt::AlignRight;
    }
  }
  if (role == Qt::DisplayRole) {
    if (0 <= section && section < columnCount() && orientation == Qt::Orientation::Horizontal) {
      return MultiParKey::TABLE_HOR_HEADERS[section];
    }
    return section + 1;
  }
  return QAbstractTableModel::headerData(section, orientation, role);
}

QVariant MultiParModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  const int r = index.row();
  if (r < 0 || r >= rowCount()) {
    return {};
  }
  using namespace MultiParTools;
  const ParVerifyInfomation& info = mVerifyInfoList[r];
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
#define MULTI_PAR_KEY_ITEM(enumName, enumValue, memberVariableDefValue, memberVariableName) \
  case MultiParKey::enumName: \
    return info.m_##memberVariableName;
      MULTI_PAR_KEY_MAPPING
#undef MULTI_PAR_KEY_ITEM
      default:
        return {};
    }
  } else if (role == Qt::DecorationRole) {
    switch (index.column()) {
      case MultiParKey::ColumnE::SOURCE_FILES:
        return ImageTool::GetIconFromCachedByFullPath(info.getFirstSrcFileAbsPath());
      case MultiParKey::ColumnE::PAR2_FILE:
        return ImageTool::GetIconFromCachedByFullPath(info.getPar2FileAbsPath());
      case MultiParKey::ColumnE::STATUS: {
        static const QIcon status2Icon[(int) ParVerifyInfomation::Par2StatusE::PAR_STATUS_BUTT]{
            QIcon{":img/SAVED"},             // Par2StatusE::ALL_FILES_COMPLETE
            QIcon{":img/REPAIR"},            // Par2StatusE::READY_TO_REPAIR
            QIcon{":img/RENAME"},            // Par2StatusE::READY_TO_RENAME
            QIcon{":img/NOT_SAVED"},         // Par2StatusE::NEED_MORE_TO_REPAIR
            QIcon{":img/REFRESH_THIS_PATH"}, // Par2StatusE::NEED_VERIFY_AGAIN
            QIcon{":img/QUESTION"},          // Par2StatusE::CANNOT_VERIFY
        };
        return status2Icon[(int) info.m_statusE];
      }
      default:
        return {};
    }
  } else if (role == Qt::ForegroundRole) {
    if (info.isNeedRepair()) {
      return QColor{Qt::GlobalColor::red};
    }
  }
  return {};
}

const QByteArray* MultiParModel::GetCliOutput(const QModelIndex& srcIndex) const {
  if (!srcIndex.isValid()) {
    return nullptr;
  }
  return &(mVerifyInfoList[srcIndex.row()].m_verifyOutput);
}

void MultiParModel::EmitInfoChanged(const QModelIndex& ind) {
  emit dataChanged(ind.siblingAtColumn(0),                    //
                   ind.siblingAtColumn(GetLastColumnIndex()), //
                   {Qt::DisplayRole, Qt::DecorationRole, Qt::ForegroundRole});
}

int MultiParModel::repairBrokenFile(const QModelIndexList& srcIndexes, const ParVerifyInfomation::Par2StatusE par2Status) {
  if (srcIndexes.isEmpty()) {
    return 0;
  }
  const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
  if (!MultiParTools::IsMultiPar2Available(multiParPath)) {
    return -1;
  }
  int succeedCnt{0};
  for (const QModelIndex& ind : srcIndexes) {
    if (!ind.isValid()) {
      LOG_W("index[%d, %d] out of range", ind.row(), ind.column());
      continue;
    }
    ParVerifyInfomation& info = mVerifyInfoList[ind.row()];
    if (info.m_statusE != par2Status) {
      continue;
    }
    const QString par2FileAbsPath{info.getPar2FileAbsPath()};
    QByteArray newCliOutput;
    const MultiParTools::RepairResultE repairResult = MultiParTools::RepairAFile(multiParPath, par2FileAbsPath, &newCliOutput);
    if (repairResult != MultiParTools::RepairResultE::CANNOT_REPAIR) {
      info.updateCliOutput(newCliOutput);
      EmitInfoChanged(ind);
    }
    if (!isRepairedOk(repairResult)) {
      LOG_ERR_NP("Failed when repair", par2FileAbsPath);
      return succeedCnt;
    }
    ++succeedCnt;
  }
  LOG_INFO_P("Repair", "%d in %d rows fixed.", succeedCnt, srcIndexes.size());
  return succeedCnt;
}

int MultiParModel::syncBuiltInSrcFileListInPar2(const QModelIndexList& srcIndexes) {
  if (srcIndexes.isEmpty()) {
    return 0;
  }
  int succeedCnt{0};
  for (const QModelIndex& ind : srcIndexes) {
    if (!ind.isValid()) {
      LOG_W("index[%d, %d] out of range", ind.row(), ind.column());
      continue;
    }
    ParVerifyInfomation& info = mVerifyInfoList[ind.row()];
    if (info.m_statusE != ParVerifyInfomation::Par2StatusE::READY_TO_RENAME) {
      continue;
    }
    info.getFirstSrcFileAbsPath();
    bool syncResult = Par2Tools::SyncBuiltInSrcFileListInPar2(info.m_prePath, info.m_par2FileName, info.m_volPar2FileNameList);
    if (!syncResult) {
      LOG_ERR_NP("Failed when sync built-in src file list", qPrintable(info.m_par2FileName));
      return syncResult;
    }
    info.m_statusE = ParVerifyInfomation::Par2StatusE::NEED_VERIFY_AGAIN;
    info.m_statusByteArray = "Verify it again";
    EmitInfoChanged(ind);
    ++succeedCnt;
  }
  return succeedCnt;
}

// return: need reverify index list
QModelIndexList MultiParModel::ProcessOldNewPar2Names(const QModelIndexList& srcIndexes) const {
  if (srcIndexes.isEmpty()) {
    return {};
  }

  const auto MoveOldToTrashAndRenameNewToOld = [](const QString& folderPath, const QString& oldPar2Name) -> FileOperatorType::BATCH_COMMAND_LIST_TYPE {
    QString newPar2Name = Par2Tools::GetSyncPar2FileAbsPath(oldPar2Name);
    if (!QFile::exists(PathTool::Path2Join(folderPath, oldPar2Name))) {
      return {};
    }
    if (!QFile::exists(PathTool::Path2Join(folderPath, newPar2Name))) {
      return {};
    }
    return {FileOperatorType::ACMD::GetInstMOVETOTRASH(folderPath, oldPar2Name), //
            FileOperatorType::ACMD::GetInstRENAME(folderPath, newPar2Name, oldPar2Name)};
  };

  FileOperatorType::BATCH_COMMAND_LIST_TYPE recycleThenRenameCmds;
  QModelIndexList needReverifyIndexes;
  for (const QModelIndex& ind : srcIndexes) {
    if (!ind.isValid()) {
      LOG_W("index[%d, %d] out of range", ind.row(), ind.column());
      continue;
    }
    const ParVerifyInfomation& info = mVerifyInfoList[ind.row()];
    if (!info.isNeedReverify()) {
      continue;
    }

    FileOperatorType::BATCH_COMMAND_LIST_TYPE batch;
    batch += MoveOldToTrashAndRenameNewToOld(info.m_prePath, info.m_par2FileName);
    for (const QString& oldVolPar2 : info.m_volPar2FileNameList) {
      batch += MoveOldToTrashAndRenameNewToOld(info.m_prePath, oldVolPar2);
    }
    if (batch.isEmpty()) {
      continue;
    }
    recycleThenRenameCmds += std::move(batch);

    needReverifyIndexes.push_back(ind);
  }

  if (recycleThenRenameCmds.isEmpty()) {
    LOG_WARN_NP("Skip", "No need recycle/rename at all");
    return {};
  }

  const bool bAllSucceed{UndoRedo::GetInst().Do(recycleThenRenameCmds)};
  if (!bAllSucceed) {
    LOG_ERR_NP("Recycle/Rename", "recycle old then rename new to old failed");
    return {};
  }
  return needReverifyIndexes;
}

int MultiParModel::ReverifyPar2File(const QModelIndexList& needReverifyIndexes) {
  if (needReverifyIndexes.isEmpty()) {
    return 0;
  }

  const QString multiParPath{SystemPath::MULTI_PAR_PROG_PATH()};
  if (!MultiParTools::IsMultiPar2Available(multiParPath)) {
    return -1;
  }

  int changedCnt{0};
  for (const QModelIndex& ind : needReverifyIndexes) {
    ParVerifyInfomation& info = mVerifyInfoList[ind.row()];
    const QString par2FileAbsPath = info.getPar2FileAbsPath();
    ParVerifyInfomation newInfo = MultiParTools::VerifyAFile(multiParPath, par2FileAbsPath);
    if (newInfo == info) {
      continue;
    }
    ++changedCnt;
    info.swap(newInfo);
    EmitInfoChanged(ind);
  }
  LOG_INFO_P("Reverify", "%d in %d rows data changed", changedCnt, needReverifyIndexes.size());
  return changedCnt;
}
