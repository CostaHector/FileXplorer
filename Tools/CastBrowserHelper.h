#ifndef CAST_BROWSER_HELPER_H
#define CAST_BROWSER_HELPER_H

#include <QSet>
#include <QString>
#include <QSqlRecord>

struct stCastHtml {
  QString body;
  QString vidPart;
  QString imgPart;
};

namespace CastBrowserHelper {
extern const QString HTML_H1_TEMPLATE;
extern const QString HTML_IMG_TEMPLATE;
extern const QString VID_LINK_TEMPLATE;

QString GetDetailDescription(const QString& fileAbsPath);
stCastHtml GetCastHtml(const QSqlRecord& record, const QString& imgHost, const int imgHeight);
}

#endif
