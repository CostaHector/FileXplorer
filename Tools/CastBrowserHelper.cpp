#include "CastBrowserHelper.h"
#include "DisplayEnhancement.h"
#include "PerformerJsonFileHelper.h"
#include "VideoDurationGetter.h"
#include "PathTool.h"
#include "PublicVariable.h"
#include "StyleSheet.h"
#include <QBuffer>
#include <QDir>
#include <QFileIconProvider>

namespace CastBrowserHelper {
const QString HTML_H1_TEMPLATE{R"(<a href="file:///%1">%2</a>)"};
const QString HTML_IMG_TEMPLATE{R"(<a href="file:///%1"><img src="%1" alt="%2" width="%3"></a>\n)"};
const QString VID_LINK_TEMPLATE{R"(<a href="file:///%1">&#9654;%2</a>)"};

QString GetDetailDescription(const QString& fileAbsPath) {
  QString fileName, extension;
  std::tie(fileName, extension) = PathTool::GetBaseNameExt(fileAbsPath);
  QString starDotExtensionLowerCase = '*' + extension.toLower();

  QString detail;
  detail.reserve(200);
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
  detail += QString(R"(<body>)");
  detail += QString(R"(<font size="+2">)");
  detail += QString(R"(Size: %1<br/>)").arg(FILE_PROPERTY_DSP::sizeToFileSizeDetail(fi.size()));
  detail += QString(R"(Date created: %1<br/>)").arg(fi.lastModified().toString(Qt::ISODate));
  detail += QString(R"(Date modified: %1<br/>)").arg(fi.birthTime().toString(Qt::ISODate));
  detail += QString(R"(Full path: %1<br/>)").arg(fileAbsPath);
  detail += QString(R"(</font>)");
  detail += QString(R"(</body>)");
  return detail;
}

stCastHtml GetCastHtml(const QSqlRecord& record, const QString& imgHost, const int imgHeight) {
  const QString castName {record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString()};
  const QString orientation {record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString()};
  const QString imgsStr {record.field(PERFORMER_DB_HEADER_KEY::Imgs).value().toString()};
  const QStringList imgsLst {PerformerJsonFileHelper::InitImgsList(imgsStr)};
  const QDir imgDir {imgHost + '/' + orientation + '/' + castName};
  const QString portraitImg {imgsLst.isEmpty() ? "" : imgDir.absoluteFilePath(imgsLst.front())};
  const QString details{record.field(PERFORMER_DB_HEADER_KEY::Detail).value().toString().replace(PerformerJsonFileHelper::IMG_VID_SEP_COMP, "<br/>")};

  const auto GetVidsLinks = [](const QString& vidsStr) -> QString {
    if (vidsStr.isEmpty()) {
      return "";
    }
    QString vidsLinks;
    vidsLinks.reserve(500);
    foreach (const QString vidPath, vidsStr.split(PerformerJsonFileHelper::IMG_VID_SEP_COMP)) {
      vidsLinks += VID_LINK_TEMPLATE.arg(vidPath, vidPath);
      vidsLinks += "<br/>";
    }
    return vidsLinks;
  };

  static const QString CAST_BRIEF_INTRODUCTION_TEMPLATE//
      { R"(
  <table width="100%" border="0" cellspacing="0" cellpadding="10">
    <tr>
      <!-- Left Brief Introduction -->
      <td width="70%" valign="top">
        <h1 style="margin:0;">%1</h1>
        <hr style="border-top:1px solid #ccc;margin:5px 0">
        <table cellpadding="5">
          <tr><td><b>Rate:</b></td><td>%4</td></tr>
          <tr><td><b>AKA:</b></td><td>%5</td></tr>
          <tr><td><b>Tags:</b></td><td>%6</td></tr>
          <tr><td><b>Orientation:</b></td><td>%7</td></tr>
          <tr><td><b>Details:</b></td><td>%8</td></tr>
        </table>
      </td>
      <!-- Right self portrait -->
      <td width="30%" align="right" valign="top">
        <a href="file:///%2">
          <img src="file:///%2" alt="Photo" width="%3" style="border:1px solid #ddd">
        </a>
      </td>
    </tr>
  </table>
)"};
  QString htmlSrc;
  htmlSrc.reserve(1500);
  htmlSrc += "<html><body>";
  htmlSrc += CAST_BRIEF_INTRODUCTION_TEMPLATE                                         //
                 .arg(castName)                                                       //
                 .arg(portraitImg)                                                    //
                 .arg(imgHeight)                                                      //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::Rate).value().toInt())    //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::AKA).value().toString())  //
                 .arg(record.field(PERFORMER_DB_HEADER_KEY::Tags).value().toString()) //
                 .arg(orientation)                                                    //
                 .arg(details);

  htmlSrc += R"(<h3 style="margin:10px 0 5px 0;"><a href="hideRelatedVideos">▼ Related Videos</a></h3>)" "\n";
  htmlSrc += R"(<h3 style="margin:10px 0 5px 0;"><a href="hideRelatedImages">▼ Related Images</a></h3>)" "\n";
  htmlSrc += "</body><html>";

  // Videos here
  QString vidsPart;
  vidsPart.reserve(200);
  vidsPart += R"(<div style="margin-top:15px">)" "\n";
  vidsPart += GetVidsLinks(record.field(PERFORMER_DB_HEADER_KEY::Vids).value().toString());
  vidsPart += R"(</div>)" "\n";

  // Images here
  QString imgsPart;
  imgsPart.reserve(50 * imgsLst.size());
  imgsPart += R"(<div style="margin-top:20px">)" "\n";
  for (int i = 1; i < imgsLst.size(); ++i) {
    imgsPart += HTML_IMG_TEMPLATE  //
                    .arg(imgDir.absoluteFilePath(imgsLst[i]))  //
                    .arg(imgsLst[i])                        //
                    .arg(IMAGE_SIZE::IMG_WIDTH);
  }
  imgsPart += "</div>";
  return {htmlSrc, vidsPart, imgsPart};
}
}
