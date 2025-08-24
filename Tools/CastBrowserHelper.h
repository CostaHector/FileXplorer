#ifndef CAST_BROWSER_HELPER_H
#define CAST_BROWSER_HELPER_H

#include <QSet>
#include <QString>
#include <QSqlRecord>

struct CastHtmlParts {
  QString body;
  QString vidPart[2];
  QString imgPart[2];
  int length() const {
    return body.size() + vidPart[0].size() + vidPart[1].size() + imgPart[0].size() + imgPart[1].size();
  }
  QString fullHtml(bool castVideosVisisble, bool castImagesVisisble) const;
};

namespace CastBrowserHelper {
extern const QString HTML_H1_TEMPLATE;
extern const QString HTML_IMG_TEMPLATE;
extern const QString VID_LINK_TEMPLATE;

QString GetDetailDescription(const QString& fileAbsPath);
CastHtmlParts GetCastHtmlParts(const QSqlRecord& record, const QString& imgHost, const int imgHeight);
}

#endif
