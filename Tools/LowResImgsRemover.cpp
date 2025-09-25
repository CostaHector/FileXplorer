#include "LowResImgsRemover.h"
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include "JsonRenameRegex.h"
#include "PublicVariable.h"
#include "UndoRedo.h"

using namespace FileOperatorType;

using FULL_NAME_2_RESOLUTION = std::pair<QString, QString>;
using FULL_NAME_2_RESOLUTION_LIST = QList<FULL_NAME_2_RESOLUTION>;

QStringList removeClearestResolution(const FULL_NAME_2_RESOLUTION_LIST& tobeDelete) {
  if (tobeDelete.size() <= 1) {
    return {};
  }

  // 查找最清晰元素
  QString clearest;
  int maxRank = -1;

  static const QMap<QString, int> resolutionRank = {
      // 定义清晰度到权重的映射（权重越高越清晰）
      {"2160p", 10}, // 最高清晰度
      {"4K", 10},    // 等同于2160p
      {"1080p", 9},
      {"FHD", 9}, // 等同于1080p
      {"810p", 8},
      {"720p", 7},
      {"HD", 7}, // 等同于720p
      {"480p", 6},
      {"360p", 5},
      {"SD", 5} // 等同于360p
  };

  QStringList tobeDeleteTemp;
  tobeDeleteTemp.reserve(tobeDelete.size());
  for (const FULL_NAME_2_RESOLUTION& res2Name : tobeDelete) {
    tobeDeleteTemp.push_back(res2Name.first);
    if (resolutionRank.contains(res2Name.second)) {
      int rank = resolutionRank[res2Name.second];
      if (rank > maxRank) {
        maxRank = rank;
        clearest = res2Name.first;
      }
    }
  }

  if (!clearest.isEmpty()) {
    tobeDeleteTemp.removeAll(clearest);
  }
  return tobeDeleteTemp;
}

QStringList LowResImgsRemover::GetLowResImgsToDel(const QStringList& imgs) const {
  QHash<QString, FULL_NAME_2_RESOLUTION_LIST> imgsResGrp; // extension to AbsPath

  for (const auto& imgName : imgs) {
    QRegularExpressionMatch match = JSON_RENAME_REGEX::RESOLUTION_COMP.match(imgName);
    if (!match.hasMatch()) {
      continue;
    }
    const QString resolutionStr = match.captured(0);
    const int startIndex = match.capturedStart(0);
    const int length = match.capturedLength(0);
    QString stdName = imgName;
    stdName.remove(startIndex, length);

    FULL_NAME_2_RESOLUTION name2Res{imgName, resolutionStr};
    auto it = imgsResGrp.find(stdName);
    if (it != imgsResGrp.end()) {
      it->push_back(name2Res);
    } else {
      imgsResGrp[stdName] = FULL_NAME_2_RESOLUTION_LIST{name2Res};
    }
  }

  QStringList imgsToBeDel;
  for (const FULL_NAME_2_RESOLUTION_LIST& fullname2ResList : imgsResGrp.values()) {
    imgsToBeDel += removeClearestResolution(fullname2ResList);
  }
  return imgsToBeDel;
}

int LowResImgsRemover::operator()(const QString& imgPath) {
  if (!QFileInfo{imgPath}.isDir()) {
    LOG_W("Path[%s] is not a directory", qPrintable(imgPath));
    return -1;
  }
  QDir dir{imgPath, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  const auto& imgsToDel = GetLowResImgsToDel(dir.entryList());
  if (imgsToDel.isEmpty()) {
    LOG_D("No image differ by resolution need delete");
    return 0;
  }

  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(imgsToDel.size());
  for (const auto& nm : imgsToDel) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH(imgPath, nm));
  }
  bool bAllSucceed = UndoRedo::GetInst().Do(removeCmds);
  LOG_D("delete %d images items, bAllSucceed[%d]", imgsToDel.size(), bAllSucceed);
  return imgsToDel.size();
}
