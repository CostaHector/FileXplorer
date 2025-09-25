#ifndef MIMEDATAHELPER_H
#define MIMEDATAHELPER_H
#include <QMimeData>
#include <QStringList>
#include <QUrl>
#include <QModelIndexList>

class QFileSystemModel;
class FileSystemModel;
class AdvanceSearchModel;
class SearchProxyModel;
class FdBasedDbModel;

namespace MimeDataHelper {

struct MimeDataMember {
  QStringList texts;
  QList<QUrl> urls;
  QModelIndexList srcIndexes;
};

template <typename TSrcModel>
MimeDataMember GetMimeDataMemberFromSourceModel(const TSrcModel& srcModel, const QModelIndexList& indexes);

MimeDataMember GetMimeDataMemberFromSearchModel(const AdvanceSearchModel& searchSrcModel, const SearchProxyModel& searchProxyModel, const QModelIndexList& proIndexes);


template <typename TSrcModel>
bool FillCutCopySomething(TSrcModel& fsModel, const QModelIndexList& srcIndexes, const Qt::DropAction dropAct);

bool SetMimeDataCutCopy(QMimeData& mimeData, const Qt::DropAction dropAction);
int WriteIntoSystemClipboard(const MimeDataMember& mimeDataMember, Qt::DropAction dropAct);

extern template MimeDataMember GetMimeDataMemberFromSourceModel(const QFileSystemModel& srcModel, const QModelIndexList& indexes);
extern template MimeDataMember GetMimeDataMemberFromSourceModel(const FileSystemModel& srcModel, const QModelIndexList& indexes);
extern template MimeDataMember GetMimeDataMemberFromSourceModel(const FdBasedDbModel& srcModel, const QModelIndexList& indexes);

extern template bool FillCutCopySomething(FileSystemModel& fsModel, const QModelIndexList& indexes, const Qt::DropAction dropAct);
extern template bool FillCutCopySomething(AdvanceSearchModel& fsModel, const QModelIndexList& indexes, const Qt::DropAction dropAct);
}  // namespace MimeDataHelper

#endif  // MIMEDATAHELPER_H
