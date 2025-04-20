#include "FloatingPreview.h"
#include "public/PublicVariable.h"
#include "public/PathTool.h"
#include "public/DisplayEnhancement.h"
#include "Tools/QMediaInfo.h"
#include <QDir>
#include <QHeaderView>
#include <QMenu>
#include <QScrollBar>
#include <QLayout>
#include <QIcon>
#include <QFileIconProvider>
#include <QBuffer>

FloatingPreview::FloatingPreview(QWidget* parent) : QSplitter{parent} {
  setOrientation(Qt::Orientation::Vertical);
  _IMG_ENABLED = new (std::nothrow) QAction(QIcon(":img/IMAGE"), "Images", this);
  _VID_ENABLED = new (std::nothrow) QAction(QIcon(":img/VIDEO"), "Videos", this);
  _OTH_ENABLED = new (std::nothrow) QAction(QIcon(":img/FILE"), "Others", this);
  _IMG_ENABLED->setCheckable(true);
  _VID_ENABLED->setCheckable(true);
  _OTH_ENABLED->setCheckable(true);

  mTypeToDisplayTB = new QToolBar{"Type To Display", this};
  mTypeToDisplayTB->addAction(_IMG_ENABLED);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(_VID_ENABLED);
  mTypeToDisplayTB->addSeparator();
  mTypeToDisplayTB->addAction(_OTH_ENABLED);

  mTypeToDisplayTB->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonTextBesideIcon);
  mTypeToDisplayTB->setMaximumHeight(35);
  insertWidget(m_insertIndex(TYPE_DISPLAY_CONTROL), mTypeToDisplayTB);

  _IMG_ENABLED->setChecked(PreferenceSettings().value("FLOATING_IMAGE_VIEW_SHOW", true).toBool());
  _VID_ENABLED->setChecked(PreferenceSettings().value("FLOATING_VIDEO_VIEW_SHOW", false).toBool());
  _OTH_ENABLED->setChecked(PreferenceSettings().value("FLOATING_OTHER_VIEW_SHOW", false).toBool());

  onImgBtnClicked(_IMG_ENABLED->isChecked());
  onVidBtnClicked(_VID_ENABLED->isChecked());
  onOthBtnClicked(_OTH_ENABLED->isChecked());

  subscribe();

  ReadSettings();
  setWindowIcon(QIcon(":img/FLOATING_PREVIEW"));
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

QString GetDetailDescription(const QString& fileAbsPath) {
  QString fileName, extension;
  std::tie(fileName, extension) = PATHTOOL::GetBaseNameExt(fileAbsPath);
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
    mImgTv = new (std::nothrow) ItemView{"FLOATING_IMAGE_VIEW", this};
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
    mVidTv = new (std::nothrow) ItemView{"FLOATING_VIDEO_VIEW", this};
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
    mOthTv = new (std::nothrow) ItemView{"FLOATING_OTHER_VIEW", this};
    mOthTv->SetCurrentModel(mOthModel);
    mOthTv->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);
    insertWidget(m_insertIndex(OTHER), mOthTv);
  }
  PreferenceSettings().setValue("FLOATING_OTHER_VIEW_SHOW", checked);
  mOthTv->setVisible(checked);
}

// #define RUN_MAIN_FILE 1
// #ifdef RUN_MAIN_FILE
// #include <QApplication>

// int main(int argc, char* argv[]) {
//   QApplication a(argc, argv);

//  FloatingPreview fp;
//  fp.show();
//  fp(R"(E:/115/2022 M06 07/FunSizeBoys - Fetish for Daddy Issues - Mr. Steel, Marcus 1.jpg)");
//  a.exec();
//  return 0;
//}
// #endif
