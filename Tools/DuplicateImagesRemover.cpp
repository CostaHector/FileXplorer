#include "DuplicateImagesRemover.h"
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include "public/PublicVariable.h"
#include "public/UndoRedo.h"

using namespace FileOperatorType;

QStringList DuplicateImagesRemover::GetDuplicateImagesFiles(const QStringList& imgs) {
  QHash<QString, QStringList> duplicateImg;
  for (const auto& nm : imgs) {
    QString stdName = nm;
    stdName.remove(JSON_RENAME_REGEX::RESOLUTION_COMP);
    auto it = duplicateImg.find(stdName);
    if (it == duplicateImg.end()) {
      it = duplicateImg.insert(stdName, {});
    }
    (*it) << nm;
  }
  const auto resSorterLesser = [](const QString& lhs, const QString& rhs) -> bool {
    if (lhs.size() != rhs.size()) {
      return lhs.size() < rhs.size();
    }
    return lhs.toLower() < rhs.toLower();
  };

  QStringList imgsToBeDeleted;
  for (auto& nms : duplicateImg.values()) {
    if (nms.size() < 2) {
      continue;
    }
    std::sort(nms.begin(), nms.end(), resSorterLesser);
    nms.pop_back();
    imgsToBeDeleted += nms;
  }
  return imgsToBeDeleted;
}

int DuplicateImagesRemover::operator()(const QString& imgPath) {
  QFileInfo fi(imgPath);
  if (not fi.isDir()) {
    return -1;
  }
  QDir dir(imgPath);
  dir.setFilter(QDir::Filter::Files);
  const auto& imgsToBeDelete = GetDuplicateImagesFiles(dir.entryList());
  if (imgsToBeDelete.isEmpty()) {
    return 0;
  }

  BATCH_COMMAND_LIST_TYPE removeCmds;
  removeCmds.reserve(imgsToBeDelete.size());
  for (const auto& nm : imgsToBeDelete) {
    removeCmds.append(ACMD{MOVETOTRASH, {imgPath, nm}});
  }
  bool bAllSucceed = g_undoRedo.Do(removeCmds);
  qDebug("delete %d items, bAllSucceed[%d]", imgsToBeDelete.size(), bAllSucceed);
  return imgsToBeDelete.size();
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
int main(int argc, char* argv[]) {
  DuplicateImagesRemover dIR;
  const auto& cmds = dIR("D:/MEN Pages/P3000");
}
#endif
