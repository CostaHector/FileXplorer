#include "MovieDbHelper.h"
#include "MovieDBModelField.h"
#include "PathTool.h"
#include "DataFormatter.h"
#include "ImageTool.h"
#include <QSqlRecord>
#include <QVariant>

namespace MovieDbHelper {

QString GetWindowTitleName(const QSqlRecord& record) {
  return record.value(MovieDBModelField::FIELD_E::Name).toString() + '|' + record.value(MovieDBModelField::PathHash).toString();
}

QString GetDetailHtml(const QSqlRecord& record) {
  const PathTool::RMFComponent rmf{record.value(MovieDBModelField::FIELD_E::PrePathLeft).toString(),
                                   record.value(MovieDBModelField::FIELD_E::PrePathRight).toString(),
                                   record.value(MovieDBModelField::FIELD_E::Name).toString()};
  const QString fileAbsPath{rmf.joinItself()};
  QString noExtFileName, extension;
  std::tie(noExtFileName, extension) = PathTool::GetBaseNameExt(rmf.fileName);
  extension = extension.toLower();

  QString detail;
  detail.reserve(200);
  detail += QString(R"(<body>)");

  detail += QString(R"(<h1>%1</h1>)").arg(noExtFileName);
  detail += QString(R"(<h2><font color="gray">%1</font></h2>)").arg(extension);
  detail += QString(R"(<h3>Length: %1</h3>)").arg(DataFormatter::formatDurationISOMs(record.value(MovieDBModelField::FIELD_E::Duration).toInt()));

  const QString starDotExtensionLowerCase = '*' + extension;
  QString imgStr = ImageTool::GetBase64PixmapForHtml(starDotExtensionLowerCase);
  detail += QString(R"(<h3><a href="file:///%1">%2</a></h3>)").arg(fileAbsPath, imgStr);

  const qint64 filesz = record.value(MovieDBModelField::FIELD_E::Size).toLongLong();
  detail += QString(R"(<h3>Size: %1</h3>)").arg(DataFormatter::formatFileSizeWithBytes(filesz));
  detail += QString(R"(<h3>Rate: %1</h3>)").arg(record.value(MovieDBModelField::FIELD_E::Rate).toInt());
  detail += QString(R"(<h3>InLocal: %1</h3>)").arg(record.value(MovieDBModelField::FIELD_E::InLocal).toInt());

  detail += R"(<h3>Full Path:</h3>)";
  detail += fileAbsPath;

  detail += R"(<h3>SampleMD5:</h3>)";
  detail += record.value(MovieDBModelField::FIELD_E::SampleMD5).toString();

  detail += R"(<h3>Studio:</h3>)";
  detail += record.value(MovieDBModelField::FIELD_E::Studio).toString();

  detail += R"(<h3>Cast:</h3>)";
  detail += record.value(MovieDBModelField::FIELD_E::Cast).toString();

  detail += R"(<h3>Tags:</h3>)";
  detail += record.value(MovieDBModelField::FIELD_E::Tags).toString();

  detail += R"(<h3>Detail:</h3>)";
  detail += record.value(MovieDBModelField::FIELD_E::Detail).toString();

  detail += QString(R"(</body>)");
  return detail;
}

}
