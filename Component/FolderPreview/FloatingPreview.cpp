#include "FloatingPreview.h"
#include "public/DisplayEnhancement.h"
#include "public/PublicMacro.h"
#include "public/PathTool.h"
#include "public/PublicVariable.h"
#include "public/MemoryKey.h"
#include "Tools/VideoDurationGetter.h"
#include <QBuffer>
#include <QIcon>
#include <QDir>
#include <QFileIconProvider>

FloatingPreview::FloatingPreview(const QString& memoryName, QWidget* parent)
  : QStackedWidget{parent}
{
  mDetailsPane = new (std::nothrow) ClickableTextBrowser{this};
  CHECK_NULLPTR_RETURN_VOID(mDetailsPane)

  mImgVidOtherPane = new (std::nothrow) ImgVidOthWid{memoryName, this};
  CHECK_NULLPTR_RETURN_VOID(mImgVidOtherPane)

  addWidget(mDetailsPane);
  addWidget(mImgVidOtherPane);
  if (currentIndex() != (int)m_curIndex) {
    setCurrentIndex((int)m_curIndex);
  }

  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
}

void FloatingPreview::ReadSettings() {
  if (PreferenceSettings().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FloatingPreview::SaveSettings() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

#include "Tools/PerformerJsonFileHelper.h"
QString GetCastHtml(const QSqlRecord& record, const QString& imgHost, const int imgHeight) {
  const QString& name = record.field(PERFORMER_DB_HEADER_KEY::Name).value().toString();
  const QString& ori = record.field(PERFORMER_DB_HEADER_KEY::Orientation).value().toString();
  const QString& imgs = record.field(PERFORMER_DB_HEADER_KEY::Imgs).value().toString();
  const QString& vids = record.field(PERFORMER_DB_HEADER_KEY::Vids).value().toString();
  QString details = record.field(PERFORMER_DB_HEADER_KEY::Detail).value().toString();  // for display in html. don't mix toString().replace() together
  details.replace(PerformerJsonFileHelper::IMG_VID_SEP_COMP, "<br/>");

  QString vidsLinks;
  if (!vids.isEmpty()) {
    for (const QString& vidPath : vids.split(PerformerJsonFileHelper::IMG_VID_SEP_COMP)) {
      vidsLinks += (ClickableTextBrowser::VID_LINK_TEMPLATE.arg(vidPath, vidPath) + "<br/>");
    }
  }

  const QString& dirPath{imgHost + '/' + ori + '/' + name};
  const QDir dir{dirPath};
  const QStringList& m_imgsLst = PerformerJsonFileHelper::InitImgsList(imgs);
  const QString& firstImgPath{m_imgsLst.isEmpty() ? "" : dir.absoluteFilePath(m_imgsLst.front())};
  static const QString PERFORMER_HTML_TEMPLATE{
                                               R"(
<html><head></head>
<style>
perf{
font-size:48px;
font-weight: bold;
}
p{
font-size:24px;
}
</style>

<body>
<perf>%1<a href="file:///%2\"><img width=%3 src="file:///%2" align=right hspace=12 v:shapes="Picture_x0020_2"></a></perf><br/>
<p> Rates: %4 </p>
<p> Aka: %5 </p>
<p> Tags: %6 </p>
<p> Ori: %7 </p><br/>
<p> Vids:<br/>%8 </p><br/>
<p> Details:<br/>%9 </p><br/>

</body>

</html>
)"};

  QString htmlSrc = PERFORMER_HTML_TEMPLATE                                                   //
                        .arg(name)                                                            //
                        .arg(firstImgPath)                                                    //
                        .arg(imgHeight)                                                       //
                        .arg(record.field(PERFORMER_DB_HEADER_KEY::Rate).value().toInt())     //
                        .arg(record.field(PERFORMER_DB_HEADER_KEY::AKA).value().toString())   //
                        .arg(record.field(PERFORMER_DB_HEADER_KEY::Tags).value().toString())  //
                        .arg(ori)                                                             //
                        .arg(vidsLinks)                                                       //
                        .arg(details);

  for (const QString& imgName : m_imgsLst) {
    htmlSrc += ClickableTextBrowser::HTML_IMG_TEMPLATE  //
                   .arg(dir.absoluteFilePath(imgName))  //
                   .arg(imgName)                        //
                   .arg(ClickableTextBrowser::HTML_IMG_FIXED_WIDTH);
  }
  return htmlSrc;
}

void FloatingPreview::operator()(const QSqlRecord& record, const QString& imgHost, const int imgHeight) {
  CHECK_NULLPTR_RETURN_VOID(mDetailsPane)
  if (record.isEmpty()) {
    mDetailsPane->setHtml("NULL");
    return;
  }

  setWindowTitle(mLastName);
  BeforeDisplayAFileDetail();
  const QString& castHtml = GetCastHtml(record, imgHost, imgHeight);
  mDetailsPane->setHtml(castHtml);
}

QString GetDetailDescription(const QString& fileAbsPath) {
  QString fileName, extension;
  std::tie(fileName, extension) = PathTool::GetBaseNameExt(fileAbsPath);
  const QFileInfo fi{fileAbsPath};
  QString detail;
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
  if (TYPE_FILTER::IMAGE_TYPE_SET.contains("*" + extension)) {
    imgStr = QString(R"(<img src="%1" width="480" alt="%1" />)").arg(fileAbsPath);
  } else {
    static QMap<QString, QString> fileTypeImgIcons;
    auto it = fileTypeImgIcons.find(extension);
    if (it == fileTypeImgIcons.end()) {
      static QFileIconProvider iconProv;
      const QIcon& ic = iconProv.icon(extension);
      const QPixmap pm{ic.pixmap(64, 64)};
      QByteArray bArray;
      QBuffer buffer(&bArray);
      buffer.open(QIODevice::WriteOnly);
      pm.save(&buffer, "PNG");
      imgStr = R"(<img src="data:image/png;base64,)" + bArray.toBase64() + QString(R"(" width="64">)");
      fileTypeImgIcons[extension] = imgStr;
    } else {
      imgStr = it.value();
    }
  }
  detail += QString(R"(<h3><a href="file:///%1">%2<br/>%1</a></h3>)").arg(fileAbsPath, imgStr);
  detail += QString(R"(<body>)");
  detail += QString(R"(<font size="+2">)");
  detail += QString(R"(Size: %1<br/>)").arg(FILE_PROPERTY_DSP::sizeToFileSizeDetail(fi.size()));
  detail += QString(R"(Date created: %1<br/>)").arg(fi.lastModified().toString(Qt::ISODate));
  detail += QString(R"(Date modified: %1<br/>)").arg(fi.birthTime().toString(Qt::ISODate));
  detail += QString(R"(</font>)");
  detail += QString(R"(</body>)");
  return detail;
}

void FloatingPreview::operator()(const QString& pth) {  // file system view
  if (!NeedUpdate(pth)) {
    return;
  }

  mLastName = pth;
  setWindowTitle(mLastName);
  if (QFileInfo{pth}.isFile()) {  // a file
    BeforeDisplayAFileDetail();
    mDetailsPane->setHtml(GetDetailDescription(pth));
    return;
  }
  BeforeDisplayAFolder();
  mImgVidOtherPane->operator()(pth);
}

void FloatingPreview::operator()(const QString& name, const QString& pth) {  // scene view
  mLastName = name;
  setWindowTitle(mLastName);
  mImgVidOtherPane->operator()(name, pth);
}

