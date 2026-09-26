#include "DetailBrowserHelper.h"
#include "FdBasedDb.h"
#include "FileTool.h"
#include "PathTool.h"
#include "DataFormatter.h"
#include "SystemPath.h"
#include "PublicVariable.h"
#include <QSqlField>

namespace DetailBrowserHelper {
const QString WHEN_SEARCH_RETURN_EMPTY_LIST_HINT_TEXT = "[%1] Not in database";

QString GetHideRelatedVideosHref(int count) {
  return QString{R"(<a href="%1">)"}.arg(UrlAnchorTemplate::HIDE_RELATED_VIDEOS)
  + " %1 "
      + QString{"%1 Related Videos</a>"}.arg(count);
}
QString GetHideRelatedImagesHref(int count) {
  return QString{R"(<a href="%1">)"}.arg(UrlAnchorTemplate::HIDE_RELATED_IMAGES)
  + " %1 "
      + QString{"%1 Related Images</a>"}.arg(count);
}

QString GetCopyLineHref(int lineIndex) {
  return QString{R"(<a href="%1">cp</a>)"}.arg(UrlAnchorTemplate::COPY_LINE_INDEX).arg(lineIndex);
}

QString GetCopyLineHref(const QString& base64UrlPath) {
  return QString{R"(<a href="%1">cp</a>)"}.arg(UrlAnchorTemplate::COPY_LINE_INDEX).arg(base64UrlPath);
}

bool AppendSqlRecordToClipboard(const QString& base64UrlPath) {
  FileTool::CopyTextToSystemClipboard(FromBase64Url(base64UrlPath), true);
  return true;
}

QString GetSearchResultParagraphDisplay(const QString& whereText) {
  QList<QSqlRecord> records;
#ifdef RUNNING_UNIT_TESTS
  records = UserSpecifiedBrowerInteractMock::mockSqlRecordList();
#else
  FdBasedDb movieDbManager{SystemPath::VIDS_DATABASE(), "EXIST_THEN_QRY_MOVIE_DB"};
  QString qryCmd = FdBasedDb::QUERY_KEY_INFO_TEMPLATE.arg(DB_TABLE::MOVIES, whereText);
  if (!movieDbManager.QueryForTest(qryCmd, records)) {
    return QString{"<b>Query command failed</b>[%1]"}.arg(qryCmd);
  }
#endif
  if (records.isEmpty()) {
    return "<b>" + WHEN_SEARCH_RETURN_EMPTY_LIST_HINT_TEXT.arg(whereText) + "</b>";
  }

  QString searchResult;
  searchResult.reserve(512);
  searchResult += QString{"<b>%1 record(s) found</b> by key[%2]. They are:"}.arg(records.size()).arg(whereText);
  searchResult += "<table border='1' cellpadding='4' style='border-collapse: collapse;'>";
  searchResult += "<thead><tr><th>cp</th><th>Size</th><th>Name</th><th>Duration</th><th>MD5Sample</th><th>Path</th></tr></thead>";
  searchResult += "<tbody>";
  for (int i = 0; i < records.size(); ++i) {
    const QSqlRecord& record = records[i];
    const PathTool::RMFComponent rmf{record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::PrePathLeft).value().toString(),
                                     record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::PrePathRight).value().toString(),
                                     record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Name).value().toString()};
    QString copiedContent = rmf.joinItself();
    copiedContent += '\t';
    copiedContent += DataFormatter::formatFileSizeGMKB(record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Size).value().toLongLong());
    copiedContent += '\t';
    copiedContent += DataFormatter::formatDurationISOMs(record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Duration).value().toInt());
    copiedContent += '\t';
    copiedContent += record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::SampleMD5).value().toString();
    const QString base64UrlPath = DetailBrowserHelper::ToBase64Url(copiedContent);

    searchResult += "<tr>";
    searchResult += QString{"<td>%1</td>"}.arg(DetailBrowserHelper::GetCopyLineHref(base64UrlPath));
    searchResult += QString{"<td>%1</td>"}.arg(DataFormatter::formatFileSizeGMKB(record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Size).value().toLongLong()));
    searchResult += QString{"<td>%1</td>"}.arg(rmf.fileName);
    searchResult += QString{"<td>%1</td>"}.arg(DataFormatter::formatDurationISOMs(record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::Duration).value().toInt()));
    searchResult += QString{"<td>%1</td>"}.arg(record.field((int)FdBasedDb::QUERY_KEY_INFO_FIELD::SampleMD5).value().toString());
    searchResult += QString{"<td>%1</td>"}.arg(rmf.joinParentPathItself());
    searchResult += "</tr>";
  }
  searchResult += "</tbody></table>";
  return searchResult;
}

}
