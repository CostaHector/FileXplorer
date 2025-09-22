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

bool SetMimeDataCutCopy(QMimeData& mimeData, const Qt::DropAction dropAction) {
#ifdef _WIN32
  QByteArray preferred(4, 0x0);
  if (dropAction == Qt::DropAction::MoveAction) {  // # 2 for cut and 5 for copy
    preferred[0] = 0x2;
  } else if (dropAction == Qt::DropAction::CopyAction) {
    preferred[0] = 0x5;
  } else {
    LOG_W("cannot refill base DropEffect");
    return false;
  }
  mimeData.setData("Preferred DropEffect", preferred);
#else
  if (dropAction == Qt::DropAction::MoveAction) {
    mimeData.setData("XdndAction", "XdndActionMove");
  } else if (dropAction == Qt::DropAction::CopyAction) {
    mimeData.setData("XdndAction", "XdndActionCopy");
  } else {
    LOG_W("cannot refill base DropEffect");
    return false;
  }
#endif
  return true;
}

int WriteIntoSystemClipboard(const MimeDataMember& mimeDataMember, Qt::DropAction dropAct) {
  QMimeData* pMimeData = new (std::nothrow) QMimeData;
  CHECK_NULLPTR_RETURN_FALSE(pMimeData)
  pMimeData->setText(mimeDataMember.texts.join('\n'));
  pMimeData->setUrls(mimeDataMember.urls);
  if (!SetMimeDataCutCopy(*pMimeData, dropAct)) {
    LOG_E("[Abort] Set Copy/Move Action[%d] in QMimedata failed", (int)dropAct);
    return -1;
  }
  QClipboard* pClipboard = QApplication::clipboard();
  pClipboard->setMimeData(pMimeData);
  return mimeDataMember.texts.size();
}

}  // namespace MimeDataHelper
