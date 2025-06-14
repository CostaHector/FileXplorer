#include "FloatingPreview.h"
#include "public/MemoryKey.h"
#include "public/PathTool.h"
#include "public/DisplayEnhancement.h"
#include "Tools/QMediaInfo.h"
#include "public/PublicVariable.h"
#include "public/StyleSheet.h"
#include <QDir>
#include <QHeaderView>
#include <QMenu>
#include <QScrollBar>
#include <QLayout>
#include <QIcon>
#include <QFileIconProvider>
#include <QBuffer>

FloatingPreview::FloatingPreview(const QString& memoryName, QWidget* parent)
    : QSplitter{parent},
      mMemoryName{memoryName}  //
{
  setOrientation(Qt::Orientation::Vertical);

  _IMG_ENABLED = new (std::nothrow) QAction(QIcon(":img/IMAGE"), "Images", this);
  CHECK_NULLPTR_RETURN_VOID(_IMG_ENABLED)
  _IMG_ENABLED->setCheckable(true);
  _IMG_ENABLED->setChecked(PreferenceSettings().value("FLOATING_IMAGE_VIEW_SHOW", true).toBool());

  _VID_ENABLED = new (std::nothrow) QAction(QIcon(":img/VIDEO"), "Videos", this);
  CHECK_NULLPTR_RETURN_VOID(_VID_ENABLED)
  _VID_ENABLED->setCheckable(true);
  _VID_ENABLED->setChecked(PreferenceSettings().value("FLOATING_VIDEO_VIEW_SHOW", false).toBool());

  _OTH_ENABLED = new (std::nothrow) QAction(QIcon(":img/FILE"), "Others", this);
  CHECK_NULLPTR_RETURN_VOID(_OTH_ENABLED)
  _OTH_ENABLED->setCheckable(true);
  _OTH_ENABLED->setChecked(PreferenceSettings().value("FLOATING_OTHER_VIEW_SHOW", false).toBool());

  mTypeToDisplayTB = new (std::nothrow) QToolBar{"Type To Display", this};
  CHECK_NULLPTR_RETURN_VOID(mTypeToDisplayTB)
  mTypeToDisplayTB->addAction(_IMG_ENABLED);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(_VID_ENABLED);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(_OTH_ENABLED);
  mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mTypeToDisplayTB->setMaximumHeight(35);
  insertWidget(m_insertIndex(TYPE_DISPLAY_CONTROL), mTypeToDisplayTB);

  onImgBtnClicked(_IMG_ENABLED->isChecked());
  onVidBtnClicked(_VID_ENABLED->isChecked());
  onOthBtnClicked(_OTH_ENABLED->isChecked());

  subscribe();

  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
}

void FloatingPreview::showEvent(QShowEvent* event) {
  QSplitter::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void FloatingPreview::ReadSettings() {
  if (PreferenceSettings().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
    restoreState(PreferenceSettings().value("FLOATING_PREVIEW_STATE").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FloatingPreview::SaveSettings() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

void FloatingPreview::SaveState() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_STATE", saveState());
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
  if (record.isEmpty()) {
    mDetailsPane->setHtml("NULL");
    return;
  }

  setWindowTitle(mLastName);
  BeforeDisplayAFileDetail();
  if (mDetailsPane == nullptr) {
    mDetailsPane = new (std::nothrow) ClickableTextBrowser(this);
    CHECK_NULLPTR_RETURN_VOID(mDetailsPane)
    insertWidget(m_insertIndex(DETAIL), mDetailsPane);
  }
  const QString& castHtml = GetCastHtml(record, imgHost, imgHeight);
  mDetailsPane->setHtml(castHtml);
  return;
}

QString GetDetailDescription(const QString& fileAbsPath) {
  QString fileName, extension;
  std::tie(fileName, extension) = PathTool::GetBaseNameExt(fileAbsPath);
  const QFileInfo fi{fileAbsPath};
  QString detail;
  detail += QString(R"(<h1>%1</h1>)").arg(fileName);
  detail += QString(R"(<h2><font color="gray">%1</font></h2>)").arg(extension);
  if (TYPE_FILTER::VIDEO_TYPE_SET.contains("*" + extension)) {
    QMediaInfo mi;
    if (!mi.StartToGet()) {
      qWarning("Start to Get failed");
      return {};
    }
    const int dur = mi.VidDurationLengthQuick(fileAbsPath);
    detail += QString(R"(<h3>Length: %1</h3><br/>)").arg(FILE_PROPERTY_DSP::durationToHumanReadFriendly(dur));
  }

  QString imgStr;
  if (TYPE_FILTER::IMAGE_TYPE_SET.contains("*" + extension)) {
    imgStr = QString(R"(<img src="%1" width="480" alt="%1" />)").arg(fileAbsPath);
  } else {
    static QFileIconProvider iconProv;
    const QIcon& ic = iconProv.icon(fi);
    const QPixmap pm{ic.pixmap(64, 64)};
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    pm.save(&buffer, "PNG");
    imgStr = R"(<img src="data:image/png;base64,)" + bArray.toBase64() + QString(R"(" width="128" alt="%1">)").arg(fileAbsPath);
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

void FloatingPreview::operator()(const QString& pth) {  // file system
  if (!NeedUpdate(pth)) {
    return;
  }

  mLastName = pth;
  setWindowTitle(mLastName);
  if (!QDir(pth).exists()) {  // a file
    BeforeDisplayAFileDetail();
    if (mDetailsPane == nullptr) {
      mDetailsPane = new (std::nothrow) ClickableTextBrowser(this);
      insertWidget(m_insertIndex(DETAIL), mDetailsPane);
    }
    mDetailsPane->setHtml(GetDetailDescription(pth));
    return;
  }
  BeforeDisplayAFolder();
  if (NeedUpdateImgs()) {
    const int imgCnt = mImgModel->setDirPath(pth, TYPE_FILTER::IMAGE_TYPE_SET, false);
    _IMG_ENABLED->setText(QString("%1 Images").arg(imgCnt, 3, 10));
  }
  if (NeedUpdateVids()) {
    const int vidCnt = mVidsModel->setDirPath(pth, TYPE_FILTER::VIDEO_TYPE_SET, true);
    _VID_ENABLED->setText(QString("%1 Videos").arg(vidCnt, 3, 10));
  }
  if (NeedUpdateOthers()) {
    const int othCnt = mOthModel->setDirPath(pth, TYPE_FILTER::TEXT_TYPE_SET, true);
    _OTH_ENABLED->setText(QString("%1 Others").arg(othCnt, 3, 10));
  }
}

void FloatingPreview::operator()(const QString& name, const QString& pth) {  // scene
  mLastName = name;
  setWindowTitle(mLastName);

  QDir dir{pth, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  mImgModel->UpdateData(dir.entryList());
}

void FloatingPreview::UpdateImgs(const QString& name, const QStringList& imgPthLst) {
  mLastName = name;
  setWindowTitle(mLastName);
  mImgModel->UpdateData(imgPthLst);
}

void FloatingPreview::UpdateVids(const QStringList& dataLst) {
  if (mVidsModel == nullptr) {
    return;
  }
  mVidsModel->UpdateData(dataLst);
}

void FloatingPreview::UpdateOthers(const QStringList& dataLst) {
  if (mOthModel == nullptr) {
    return;
  }
  mOthModel->UpdateData(dataLst);
}

void FloatingPreview::subscribe() {
  connect(_IMG_ENABLED, &QAction::triggered, this, &FloatingPreview::onImgBtnClicked);
  connect(_VID_ENABLED, &QAction::triggered, this, &FloatingPreview::onVidBtnClicked);
  connect(_OTH_ENABLED, &QAction::triggered, this, &FloatingPreview::onOthBtnClicked);
  connect(this, &QSplitter::splitterMoved, this, &FloatingPreview::SaveState);
}

void FloatingPreview::onImgBtnClicked(bool checked) {
  if (mImgTv == nullptr) {
    mImgModel = new (std::nothrow) ImgsModel;
    CHECK_NULLPTR_RETURN_VOID(mImgModel)
    mImgTv = new (std::nothrow) ItemView{mMemoryName + "_IMAGE", this};
    CHECK_NULLPTR_RETURN_VOID(mImgTv)
    mImgTv->SetCurrentModel(mImgModel);
    mImgTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    insertWidget(m_insertIndex(IMAGE), mImgTv);
  }
  PreferenceSettings().setValue("FLOATING_IMAGE_VIEW_SHOW", checked);
  mImgTv->setVisible(checked);
}

void FloatingPreview::onVidBtnClicked(bool checked) {
  if (mVidTv == nullptr) {
    mVidsModel = new (std::nothrow) VidsModel;
    CHECK_NULLPTR_RETURN_VOID(mVidsModel)
    mVidTv = new (std::nothrow) ItemView{mMemoryName + "_VIDEO", this};
    CHECK_NULLPTR_RETURN_VOID(mVidTv)
    mVidTv->SetCurrentModel(mVidsModel);
    mVidTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    insertWidget(m_insertIndex(VIDEO), mVidTv);
  }
  PreferenceSettings().setValue("FLOATING_VIDEO_VIEW_SHOW", checked);
  mVidTv->setVisible(checked);
}

void FloatingPreview::onOthBtnClicked(bool checked) {
  if (mOthTv == nullptr) {
    mOthModel = new (std::nothrow) OthersModel;
    CHECK_NULLPTR_RETURN_VOID(mOthModel)
    mOthTv = new (std::nothrow) ItemView{mMemoryName + "_OTHER", this};
    CHECK_NULLPTR_RETURN_VOID(mOthTv)
    mOthTv->SetCurrentModel(mOthModel);
    mOthTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    insertWidget(m_insertIndex(OTHER), mOthTv);
  }
  PreferenceSettings().setValue("FLOATING_OTHER_VIEW_SHOW", checked);
  mOthTv->setVisible(checked);
}
