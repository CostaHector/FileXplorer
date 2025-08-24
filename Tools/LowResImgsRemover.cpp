#include "LowResImgsRemover.h"
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include "PublicTool.h"
#include "PublicVariable.h"
#include "StringTool.h"
#include "UndoRedo.h"

using namespace FileOperatorType;

QStringList LowResImgsRemover::GetLowResImgsToDel(const QStringList& imgs) const {
  QHash<QString, QStringList> imgsResGrp;
  QString stdName;
  for (const auto& imgName : imgs) {
    stdName = imgName;
    stdName.remove(JSON_RENAME_REGEX::RESOLUTION_COMP);
    auto it = imgsResGrp.find(stdName);
    if (it != imgsResGrp.end()) {
      it->push_back(imgName);
    } else {
      imgsResGrp[stdName] = QStringList{imgName};
    }
  }

  using namespace StringTool;
  QStringList imgsToBeDel;
  for (auto& imgs : imgsResGrp.values()) {
    if (imgs.size() < 2) {  // only 1 image; no other resolution images
      continue;
    }
    ImgsSortFileSizeFirst(imgs);  // last image size is largest should keep, so skip
    imgs.pop_back();
    imgsToBeDel += imgs;
  }
  return imgsToBeDel;
}

int LowResImgsRemover::operator()(const QString& imgPath) {
  if (!QFileInfo{imgPath}.isDir()) {
    qWarning("Path[%s] is not a directory", qPrintable(imgPath));
    return -1;
  }
  QDir dir{imgPath};
  dir.setFilter(QDir::Filter::Files);
  const auto& imgsToDel = GetLowResImgsToDel(dir.entryList());
  if (imgsToDel.isEmpty()) {
    qDebug("No image differ by resolution need delete");
    return 0;
  }

  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(imgsToDel.size());
  for (const auto& nm : imgsToDel) {
    removeCmds.append(ACMD::GetInstMOVETOTRASH(imgPath, nm));
  }
  bool bAllSucceed = g_undoRedo.Do(removeCmds);
  qDebug("delete %d images items, bAllSucceed[%d]", imgsToDel.size(), bAllSucceed);
  return imgsToDel.size();
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  LowResImgsRemover dIR;
  const auto& cmds = dIR("D:/MEN Pages/P3000");
}
#endif
