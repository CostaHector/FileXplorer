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
inline QString ToBase64Url(const QString& s) {
  return QString::fromLatin1(
      s.toUtf8().toBase64(QByteArray::Base64UrlEncoding
                          | QByteArray::OmitTrailingEquals));
}

inline QString FromBase64Url(const QString& s) {
  return QString::fromUtf8(
      QByteArray::fromBase64(s.toLatin1(), QByteArray::Base64UrlEncoding));
}
QString GetHideRelatedImagesHref(int count);
QString GetHideRelatedVideosHref(int count);
QString GetCopyLineHref(int lineIndex);
QString GetCopyLineHref(const QString& base64UrlPath);
bool AppendSqlRecordToClipboard(const QString& base64UrlPath);
QString GetSearchResultParagraphDisplay(const QString& searchText);
};

#endif // DETAILBROWSERHELPER_H
