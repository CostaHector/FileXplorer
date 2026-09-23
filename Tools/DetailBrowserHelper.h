#ifndef DETAILBROWSERHELPER_H
#define DETAILBROWSERHELPER_H

#include <QString>
#include <QList>
#include <QSqlRecord>

namespace UserSpecifiedBrowerInteractMock {
inline QList<QSqlRecord>& mockSqlRecordList() {
  static QList<QSqlRecord> sqlRecordsList;
  return sqlRecordsList;
}
}

namespace UrlSchema {
constexpr const char* HIDE_RELATED = "hide-related";
constexpr const char* COPY_LINE = "copy-line";
}

namespace UrlAnchorTemplate {
constexpr const char* HIDE_RELATED_IMAGES = "hide-related:Images";
constexpr const char* HIDE_RELATED_VIDEOS = "hide-related:Videos";
constexpr const char* COPY_LINE_INDEX = "copy-line:%1";
}

namespace DetailBrowserHelper {
QString GetHideRelatedImagesHref(int count);
QString GetHideRelatedVideosHref(int count);
QString GetCopyLineHref(int lineIndex);
bool AppendSqlRecordToClipboard(const QList<QSqlRecord>& recordsLst, const QString& lineIndexStr);
QString GetSearchResultParagraphDisplay(const QString& searchText, QList<QSqlRecord>* pSqlRecordList = nullptr);
};

#endif // DETAILBROWSERHELPER_H
