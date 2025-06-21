#include "FloatingPreview.h"
#include "public/MemoryKey.h"
#include "public/PathTool.h"
#include "public/DisplayEnhancement.h"
#include "public/PublicVariable.h"
#include "public/StyleSheet.h"
#include <QDir>
#include <QLayout>
#include <QIcon>
#include <QFileIconProvider>
#include <QBuffer>

#ifdef _WIN32
#include "Tools/QMediaInfo.h"
#endif

constexpr FloatingPreview::MediaBtnHandlerFunc FloatingPreview::MEDIA_HANDLERS_MAP[];

FloatingPreview::FloatingPreview(const QString& memoryName, QWidget* parent)
    : QStackedWidget{parent},
      mMemoryName{memoryName}  //
{
  mDetailsPane = new (std::nothrow) ClickableTextBrowser(this);

  _IMG_ACT = new (std::nothrow) QAction(QIcon(":img/IMAGE"), "Images", this);
  CHECK_NULLPTR_RETURN_VOID(_IMG_ACT)
  _VID_ACT = new (std::nothrow) QAction(QIcon(":img/VIDEO"), "Videos", this);
  CHECK_NULLPTR_RETURN_VOID(_VID_ACT)
  _OTH_ACT = new (std::nothrow) QAction(QIcon(":img/FILE"), "Others", this);
  CHECK_NULLPTR_RETURN_VOID(_OTH_ACT)

  _IMG_ACT->setCheckable(true);
  _VID_ACT->setCheckable(true);
  _OTH_ACT->setCheckable(true);

  m_bImgVisible = PreferenceSettings().value("FLOATING_IMAGE_VIEW_SHOW", true).toBool();
  m_bVidVisible = PreferenceSettings().value("FLOATING_VIDEO_VIEW_SHOW", false).toBool();
  m_bOthVisible = PreferenceSettings().value("FLOATING_OTHER_VIEW_SHOW", false).toBool();

  _IMG_ACT->setChecked(m_bImgVisible);
  _VID_ACT->setChecked(m_bVidVisible);
  _OTH_ACT->setChecked(m_bOthVisible);

  mImgVidOtherSplitter = new (std::nothrow) QSplitter{this};
  CHECK_NULLPTR_RETURN_VOID(mImgVidOtherSplitter)
  mImgVidOtherSplitter->setOrientation(Qt::Orientation::Vertical);

  const QString& defaultMediaTypeSeq = MediaTypeSeqStr(mMediaSequence);
  const QString& seqStr = PreferenceSettings().value("FLOATING_MEDIA_TYPE_SEQ", defaultMediaTypeSeq).toString();
  decltype(mMediaSequence) mediaSequenceMemory;
  if (IsValidMediaTypeSeq(seqStr, mediaSequenceMemory) && mediaSequenceMemory.size() == mMediaSequence.size()) {
    mMediaSequence.swap(mediaSequenceMemory);
  }
  const bool visibility[(int)PREVIEW_ITEM_TYPE::BUTT] = {m_bImgVisible, m_bVidVisible, m_bOthVisible};
  for (int mediaTypeInd : mMediaSequence) {
    (this->*MEDIA_HANDLERS_MAP[mediaTypeInd])(visibility[mediaTypeInd]);
  }

  onImgBtnClicked(m_bImgVisible);
  onVidBtnClicked(m_bVidVisible);
  onOthBtnClicked(m_bOthVisible);
  onTypesBtnClicked(true);

  addWidget(mDetailsPane);
  addWidget(mImgVidOtherSplitter);
  if (currentIndex() != (int)m_curIndex) {
    setCurrentIndex((int)m_curIndex);
  }

  subscribe();
  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
}

void FloatingPreview::showEvent(QShowEvent* event) {
  QStackedWidget::showEvent(event);
  StyleSheet::UpdateTitleBar(this);
}

void FloatingPreview::ReadSettings() {
  if (PreferenceSettings().contains("FLOATING_PREVIEW_GEOMETRY")) {
    restoreGeometry(PreferenceSettings().value("FLOATING_PREVIEW_GEOMETRY").toByteArray());
    mImgVidOtherSplitter->restoreState(PreferenceSettings().value("FLOATING_PREVIEW_STATE").toByteArray());
  } else {
    setGeometry(QRect(0, 0, 480, 1080));
  }
}

void FloatingPreview::SaveSettings() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_GEOMETRY", saveGeometry());
}

void FloatingPreview::SaveState() {
  PreferenceSettings().setValue("FLOATING_PREVIEW_STATE", mImgVidOtherSplitter->saveState());
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
  if (TYPE_FILTER::VIDEO_TYPE_SET.contains("*" + extension)) {
    int dur = 0;
#ifdef _WIN32
    QMediaInfo mi;
    if (!mi.StartToGet()) {
      qWarning("Start to Get failed");
      return {};
    }
    dur = mi.VidDurationLengthQuick(fileAbsPath);
#endif
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
  if (QFileInfo{pth}.isFile()) {  // a file
    BeforeDisplayAFileDetail();
    mDetailsPane->setHtml(GetDetailDescription(pth));
    return;
  }
  BeforeDisplayAFolder();
  if (NeedUpdateImgs()) {
    const int imgCnt = mImgModel->setDirPath(pth, TYPE_FILTER::IMAGE_TYPE_SET, false);
    _IMG_ACT->setText(QString("%1 Images").arg(imgCnt, 3, 10));
  }
  if (NeedUpdateVids()) {
    const int vidCnt = mVidsModel->setDirPath(pth, TYPE_FILTER::VIDEO_TYPE_SET, true);
    _VID_ACT->setText(QString("%1 Videos").arg(vidCnt, 3, 10));
  }
  if (NeedUpdateOthers()) {
    const int othCnt = mOthModel->setDirPath(pth, TYPE_FILTER::TEXT_TYPE_SET, true);
    _OTH_ACT->setText(QString("%1 Others").arg(othCnt, 3, 10));
  }
}

void FloatingPreview::operator()(const QString& name, const QString& pth) {  // scene
  mLastName = name;
  setWindowTitle(mLastName);
  CHECK_NULLPTR_RETURN_VOID(mImgModel)
  QDir dir{pth, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  mImgModel->UpdateData(dir.entryList());
}

void FloatingPreview::UpdateImgs(const QString& name, const QStringList& imgPthLst) {
  mLastName = name;
  setWindowTitle(mLastName);
  CHECK_NULLPTR_RETURN_VOID(mImgModel)
  mImgModel->UpdateData(imgPthLst);
}

void FloatingPreview::UpdateVids(const QStringList& dataLst) {
  CHECK_NULLPTR_RETURN_VOID(mVidsModel)
  mVidsModel->UpdateData(dataLst);
}

void FloatingPreview::UpdateOthers(const QStringList& dataLst) {
  CHECK_NULLPTR_RETURN_VOID(mOthModel)
  mOthModel->UpdateData(dataLst);
}

void FloatingPreview::subscribe() {
  connect(mTypeToDisplayTB, &ReorderableToolBar::widgetMoved, this, &FloatingPreview::onReorder);
  connect(mTypeToDisplayTB->mCollectPathAgs, &QActionGroup::triggered, this, &FloatingPreview::onImgVidOthActTriggered);
  connect(mImgVidOtherSplitter, &QSplitter::splitterMoved, this, &FloatingPreview::SaveState);
}

bool FloatingPreview::onReorder(int fromIndex, int destIndex) {
  CHECK_NULLPTR_RETURN_FALSE(mImgVidOtherSplitter);
  if (!MoveElementFrontOf(mMediaSequence, fromIndex, destIndex)) {
    qWarning("failed, move widget at index[%d] in front of widget at[%d]", fromIndex, destIndex);
    return false;
  }
  const QString& newMediaTypeSeq = MediaTypeSeqStr(mMediaSequence);
  PreferenceSettings().setValue("FLOATING_MEDIA_TYPE_SEQ", newMediaTypeSeq);
  qDebug("New media type seq[%s]", qPrintable(newMediaTypeSeq));
  return MoveWidgetAtFromIndexInFrontOfDestIndex(fromIndex, destIndex, *mImgVidOtherSplitter);
}

void FloatingPreview::onImgBtnClicked(bool checked) {
  m_bImgVisible = checked;
  if (mImgTv == nullptr) {
    mImgModel = new (std::nothrow) ImgsModel;
    CHECK_NULLPTR_RETURN_VOID(mImgModel)
    mImgTv = new (std::nothrow) ItemView{mMemoryName + "_IMAGE", this};
    CHECK_NULLPTR_RETURN_VOID(mImgTv)
    mImgTv->SetCurrentModel(mImgModel);
    mImgTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
    mImgVidOtherSplitter->addWidget(mImgTv);
  }
  PreferenceSettings().setValue("FLOATING_IMAGE_VIEW_SHOW", checked);
  if (mImgTv->isVisible() != checked) {
    mImgTv->setVisible(checked);
  }
}

void FloatingPreview::onVidBtnClicked(bool checked) {
  m_bVidVisible = checked;
  if (mVidTv == nullptr) {
    mVidsModel = new (std::nothrow) VidsModel;
    CHECK_NULLPTR_RETURN_VOID(mVidsModel)
    mVidTv = new (std::nothrow) ItemView{mMemoryName + "_VIDEO", this};
    CHECK_NULLPTR_RETURN_VOID(mVidTv)
    mVidTv->SetCurrentModel(mVidsModel);
    mVidTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    mImgVidOtherSplitter->addWidget(mVidTv);
  }
  PreferenceSettings().setValue("FLOATING_VIDEO_VIEW_SHOW", checked);
  mVidTv->setVisible(checked);
}

void FloatingPreview::onOthBtnClicked(bool checked) {
  m_bOthVisible = checked;
  if (mOthTv == nullptr) {
    mOthModel = new (std::nothrow) OthersModel;
    CHECK_NULLPTR_RETURN_VOID(mOthModel)
    mOthTv = new (std::nothrow) ItemView{mMemoryName + "_OTHER", this};
    CHECK_NULLPTR_RETURN_VOID(mOthTv)
    mOthTv->SetCurrentModel(mOthModel);
    mOthTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    mImgVidOtherSplitter->addWidget(mOthTv);
  }
  PreferenceSettings().setValue("FLOATING_OTHER_VIEW_SHOW", checked);
  mOthTv->setVisible(checked);
}

void FloatingPreview::onTypesBtnClicked(bool checked) {
  if (mTypeToDisplayTB == nullptr) {
    mTypeToDisplayTB = new (std::nothrow) ReorderableToolBar{"Type To Display", this};
    CHECK_NULLPTR_RETURN_VOID(mTypeToDisplayTB)
    mTypeToDisplayTB->setOrientation(Qt::Orientation::Horizontal);
    mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
    static QAction* MEDIA_TYPE_2_ACTS[(int)PREVIEW_ITEM_TYPE::BUTT] = {_IMG_ACT, _VID_ACT, _OTH_ACT};
    for (int mediaTypeInd : mMediaSequence) {
      mTypeToDisplayTB->addAction(MEDIA_TYPE_2_ACTS[mediaTypeInd]);
    }
    mTypeToDisplayTB->setMaximumHeight(35);
    mImgVidOtherSplitter->addWidget(mTypeToDisplayTB);
  }
  mTypeToDisplayTB->setVisible(checked);
}

void FloatingPreview::onImgVidOthActTriggered(const QAction* pAct) {
  CHECK_NULLPTR_RETURN_VOID(pAct);
  const bool checked{pAct->isChecked()};
  if (pAct == _IMG_ACT) {
    onImgBtnClicked(checked);
  } else if (pAct == _VID_ACT) {
    onVidBtnClicked(checked);
  } else if (pAct == _OTH_ACT) {
    onOthBtnClicked(checked);
  } else {
    qWarning("Action[%s] not supported", qPrintable(pAct->text()));
  }
}
