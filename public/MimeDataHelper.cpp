#include "MimeDataHelper.h"
#include "FileSystemModel.h"
#include "AdvanceSearchModel.h"
#include "SearchProxyModel.h"
#include "FdBasedDbModel.h"
#include <QApplication>
#include <QClipboard>

namespace MimeDataHelper {
template MimeDataMember GetMimeDataMemberFromSourceModel(const QFileSystemModel& srcModel, const QModelIndexList& indexes);
template MimeDataMember GetMimeDataMemberFromSourceModel(const FileSystemModel& srcModel, const QModelIndexList& indexes);
template MimeDataMember GetMimeDataMemberFromSourceModel(const FdBasedDbModel& srcModel, const QModelIndexList& indexes);

template bool FillCutCopySomething(FileSystemModel& fsModel, const QModelIndexList& indexes, const Qt::DropAction dropAct);
template bool FillCutCopySomething(AdvanceSearchModel& fsModel, const QModelIndexList& indexes, const Qt::DropAction dropAct);

template <typename TSrcModel>
MimeDataMember GetMimeDataMemberFromSourceModel(const TSrcModel& srcModel, const QModelIndexList& indexes) {
  QStringList filePaths;
  filePaths.reserve(indexes.size());

  QList<QUrl> urls;
  urls.reserve(indexes.size());

  for (const auto& ind : indexes) {
    QString absFilePath{srcModel.filePath(ind)};
    filePaths.append(absFilePath);
    urls.append(QUrl::fromLocalFile(absFilePath));
  }
  return {filePaths, urls, indexes};
}

MimeDataMember GetMimeDataMemberFromSearchModel(const AdvanceSearchModel& searchSrcModel, const SearchProxyModel& searchProxyModel, const QModelIndexList& proIndexes) {
  QStringList filePaths;
  filePaths.reserve(proIndexes.size());

  QList<QUrl> urls;
  urls.reserve(proIndexes.size());

  QModelIndexList srcIndexes;
  srcIndexes.reserve(proIndexes.size());

  for (const auto& proInd : proIndexes) {
    const auto& srcind = searchProxyModel.mapToSource(proInd);
    const QString absFilePath = searchSrcModel.filePath(srcind);
    srcIndexes.append(srcind);
    filePaths.append(absFilePath);
    urls.append(QUrl::fromLocalFile(absFilePath));
  }
  return {filePaths, urls, srcIndexes};
}

template <typename TSrcModel>
bool FillCutCopySomething(TSrcModel& fsModel, const QModelIndexList& srcIndexes, const Qt::DropAction dropAct) {
  if (dropAct == Qt::CopyAction) {
    fsModel.CopiedSomething(srcIndexes);
  } else if (dropAct == Qt::MoveAction) {
    fsModel.CutSomething(srcIndexes);
  } else {
    LOG_D("dropAct[%d] not support", (int)dropAct);
    return false;
  }
  return true;
}

bool SetMimeDataCutCopy(QMimeData& mimeData, const Qt::DropAction dropAction) {
#ifdef _WIN32
  QByteArray preferred(4, 0x0);
  if (dropAction == Qt::DropAction::MoveAction) {  // # 2 for cut and 5 for copy
    preferred[0] = 0x2;
  } else if (dropAction == Qt::DropAction::CopyAction) {
    preferred[0] = 0x1;
  } else if (dropAction == Qt::DropAction::LinkAction) {
    preferred[0] = 0x4;
  } else {
    LOG_W("Unsupport DropEffect[%d]", (int)dropAction);
    return false;
  }
  mimeData.setData("Preferred DropEffect", preferred);
#else
  if (dropAction == Qt::DropAction::MoveAction) {
    mimeData.setData("XdndAction", "XdndActionMove");
  } else if (dropAction == Qt::DropAction::CopyAction) {
    mimeData.setData("XdndAction", "XdndActionCopy");
  } else if (dropAction == Qt::DropAction::LinkAction) {
    mimeData.setData("XdndAction", "XdndActionLink");
  } else {
    LOG_W("Unsupport DropEffect[%d]", (int)dropAction);
    return false;
  }
#endif
  return true;
}

int WriteIntoSystemClipboard(const MimeDataMember& mimeDataMember, Qt::DropAction dropAct) {
  QMimeData* pMimeData = new (std::nothrow) QMimeData;
  CHECK_NULLPTR_RETURN_FALSE(pMimeData);
  const int itemsCnt = mimeDataMember.texts.size();
  pMimeData->setText(mimeDataMember.texts.join('\n'));
  pMimeData->setUrls(mimeDataMember.urls);
  if (!SetMimeDataCutCopy(*pMimeData, dropAct)) {
    LOG_E("[Abort] Set Copy/Move Action[%d] in QMimedata failed", (int)dropAct);
    return -1;
  }
  QClipboard* pClipboard = QApplication::clipboard();
  CHECK_NULLPTR_RETURN_INT(pClipboard, -2);
  pClipboard->setMimeData(pMimeData);
  LOG_D("setMimeData with %d item(s) finished", itemsCnt);
  return itemsCnt;
}

}  // namespace MimeDataHelper
