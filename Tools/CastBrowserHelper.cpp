#include "CastBrowserHelper.h"
#include "DisplayEnhancement.h"
#include "TableFields.h"
#include "VideoDurationGetter.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "StyleSheet.h"
#include "StringTool.h"
#include <QBuffer>
#include <QDir>
#include <QFileIconProvider>
#include <QSqlField>
#include <QImageReader>

QString CastHtmlParts::fullHtml(bool castVideosVisisble, bool castImagesVisisble) const {
  QString fullHtmlContents;
  fullHtmlContents.reserve(12 + length() + 14);
  fullHtmlContents += "<html><body>"; // 12
  fullHtmlContents += R"(<font size="+2">)";
  fullHtmlContents += body;
  fullHtmlContents += vidPart[0].arg(castVideosVisisble ? "▼" : "▶");
  fullHtmlContents += castVideosVisisble ? vidPart[1] : "";
  fullHtmlContents += imgPart[0].arg(castImagesVisisble ? "▼" : "▶");
  fullHtmlContents += castImagesVisisble ? imgPart[1] : "";
  fullHtmlContents += R"(</font>)";
  fullHtmlContents += "</body></html>"; // 14
  return fullHtmlContents;
}

namespace CastBrowserHelper {
const QString HTML_H1_TEMPLATE{R"(<a href="file:///%1">%2</a>)"};
const QString HTML_IMG_WIDTH_TEMPLATE{R"(<a href="file:///%1"><img src="%1" alt="%2" width="%3"></a>)"};
const QString HTML_IMG_HEIGHT_TEMPLATE{R"(<a href="file:///%1"><img src="%1" alt="%2" height="%3"></a>)"};
const QString VID_LINK_TEMPLATE{R"(<a href="file:///%1" title="%1" style="cursor:pointer">&#9654;%2</a>)"};

QSize GetImageSize(const QString& filePath) {
  QImageReader reader{filePath};
  if (!reader.canRead()) {
    return {};
  }
  return reader.size();
}

QString GenerateSingleImageInHtml(const QString& imagePath, const QString& altText, const QSize& ICON_SIZE) {
  const auto realSize = GetImageSize(imagePath);
  if (realSize.isNull()) {
    return HTML_IMG_WIDTH_TEMPLATE.arg("").arg(altText).arg(0);
  }
  if (realSize.width() * ICON_SIZE.height() >= realSize.height() * ICON_SIZE.width()) {
    int actualWidth = ICON_SIZE.width();
    return HTML_IMG_WIDTH_TEMPLATE.arg(imagePath).arg(altText).arg(actualWidth);
  } else {
    int actualHeight = ICON_SIZE.height();
    return HTML_IMG_HEIGHT_TEMPLATE.arg(imagePath).arg(altText).arg(actualHeight);
  }
}

QString GetDetailDescription(const QString& fileAbsPath) {
  QString fileName, extension;
  std::tie(fileName, extension) = PathTool::GetBaseNameExt(fileAbsPath);
  QString starDotExtensionLowerCase = '*' + extension.toLower();

  QString detail;
  detail.reserve(200);
  detail += QString(R"(<body>)");
  detail += QString(R"(<h1>%1</h1>)").arg(fileName);
  detail += QString(R"(<h2><font color="gray">%1</font></h2>)").arg(extension);
  const bool isFileAVideo{TYPE_FILTER::VIDEO_TYPE_SET.contains("*" + extension)};
  if (isFileAVideo) {
    VideoDurationGetter mi;
    if (!mi.StartToGet()) {
      return {};
    }
    int dur = mi.GetLengthQuick(fileAbsPath);
    detail += QString(R"(<h3>Length: %1</h3>)").arg(FILE_PROPERTY_DSP::durationToHumanReadFriendly(dur));
  }

  QString imgStr;
  if (TYPE_FILTER::IMAGE_TYPE_SET.contains(starDotExtensionLowerCase)) {
    imgStr = QString(R"(<img src="%1" width="480" alt="%1" />)").arg(fileAbsPath);
  } else {
    static QMap<QString, QString> fileTypeImgIcons;
    auto it = fileTypeImgIcons.find(starDotExtensionLowerCase);
    if (it == fileTypeImgIcons.end()) {
      static QFileIconProvider iconProv;
      const QIcon& ic = iconProv.icon(starDotExtensionLowerCase);
      const QPixmap pm{ic.pixmap(64, 64)};
      QByteArray bArray;
      QBuffer buffer(&bArray);
      buffer.open(QIODevice::WriteOnly);
      pm.save(&buffer, "PNG");
      imgStr = R"(<img src="data:image/png;base64,)" + bArray.toBase64() + QString(R"(" width="64">)");
      fileTypeImgIcons[starDotExtensionLowerCase] = imgStr;
    } else {
      imgStr = it.value();
    }
  }
  const QFileInfo fi{fileAbsPath};
  detail += QString(R"(<h3><a href="file:///%1">%2</a></h3>)").arg(fileAbsPath, imgStr);
  detail += QString(R"(<font size="+2">)");
  detail += QString(R"(Size: %1<br/>)").arg(FILE_PROPERTY_DSP::sizeToFileSizeDetail(fi.size()));
  detail += QString(R"(Date created: %1<br/>)").arg(fi.lastModified().toString(Qt::ISODate));
  detail += QString(R"(Date modified: %1<br/>)").arg(fi.birthTime().toString(Qt::ISODate));
  detail += QString(R"(Full path: %1<br/>)").arg(fileAbsPath);
  detail += QString(R"(</font>)");
  detail += QString(R"(</body>)");
  return detail;
}

CastHtmlParts GetCastHtmlParts(const QSqlRecord& record, const QString& imgHost, const QSize& ICON_SIZE) {
  const QString castName {record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString()};
  const QString orientation {record.field(PERFORMER_DB_HEADER_KEY::Ori).value().toString()};
  const QStringList& vidsLst {StringTool::GetImgsVidsListFromField(record.field(PERFORMER_DB_HEADER_KEY::Vids).value().toString())};
  const QStringList& imgsLst {StringTool::GetImgsVidsListFromField(record.field(PERFORMER_DB_HEADER_KEY::Imgs).value().toString())};

  static const QString CAST_BRIEF_INTRODUCTION_TEMPLATE//
      { R"(
  <table width="100%" border="0" cellspacing="0" cellpadding="10">
    <h1 style="margin:0;">%1 | %2 | %3</h1>
    <hr style="border-top:1px solid #ccc;margin:5px 0">
    <tr><td><b>Tags:</b>%4 | %5 | %6</td></tr>
  </table>

)"};
  QString htmlSrc;
  htmlSrc.reserve(CAST_BRIEF_INTRODUCTION_TEMPLATE.size() + 200);
  htmlSrc += CAST_BRIEF_INTRODUCTION_TEMPLATE                                         //
                 .arg(castName)                                                       //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::Rate).value().toInt())    //
                 .arg(orientation)                                                    //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::Tags).value().toString()) //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::AKA).value().toString())  //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::Detail).value().toString());

  // Videos here
  const int vidCnt{vidsLst.size()};
  QString vidsPartHead {R"(<h3 style="margin:10px 0 5px 0;"><a href="hideRelatedVideos"> %1 )" + QString::number(vidCnt) + R"( Related Videos</a></h3>)" "\n"};
  QString vidsPartBody;
  vidsPartBody.reserve(50 * vidCnt);
  vidsPartBody += R"(<div style="margin-top:15px;">)" "\n";
  foreach (const QString vidPath, vidsLst) {
    vidsPartBody += VID_LINK_TEMPLATE.arg(vidPath, PathTool::forSearchPath(vidPath));
    vidsPartBody += "<br/>";
  }
  vidsPartBody += R"(</div>)" "\n";

  // Images here
  const QDir imgDir {imgHost + '/' + orientation + '/' + castName};
  const int imgsCnt{imgsLst.size()};
  QString imgsPartHead {R"(<h3 style="margin:10px 0 5px 0;"><a href="hideRelatedImages"> %1 )" + QString::number(imgsCnt) + R"( Related Images</a></h3>)" "\n"};
  QString imgsPartBody;
  imgsPartBody.reserve(50 * imgsCnt);
  imgsPartBody += R"(<div style="margin-top:20px;">)" "\n";
  foreach (const QString imgRelPath, imgsLst) {
    imgsPartBody += GenerateSingleImageInHtml(imgDir.absoluteFilePath(imgRelPath), imgRelPath, ICON_SIZE);
  }
  imgsPartBody += "</div>" "\n";
  return {htmlSrc, {vidsPartHead, vidsPartBody}, {imgsPartHead, imgsPartBody}};
}
}
