#include "FloatingPreview.h"
#include "PublicVariable.h"
#include "NotificatorFrame.h"
#include <QDir>
#include <QHeaderView>
#include <QDesktopServices>

QVariant ImgsModel::data(const QModelIndex& index, int role) const {
  const int rw = index.row();
  if (rw < 0 || rw >= mImgsLst.size()) {
    return {};
  }
  if (role == Qt::DecorationRole) {
    const QPixmap pm{mImgsLst[rw]};
    // w/h > 480/280 = 48 / 28 = 12 / 7
    if (pm.width() * 7 >= pm.height() * 12) {
      return pm.scaledToWidth(480);
    }
    return pm.scaledToHeight(280);
  }
  return {};
}

void ImgsModel::UpdateImgs(const QStringList& newImgsLst) {  // dont use newImgsLst after
  RowsCountBeginChange(mImgsLst.size(), newImgsLst.size());
  mImgsLst = newImgsLst;
  RowsCountEndChange();
}

QString ImgsModel::filePath(const QModelIndex& index) const{
  const int rw = index.row();
  if (rw < 0 || rw >= mImgsLst.size()) {
    return {};
  }
  return mImgsLst[rw];
}

// -----------------

QPushButton* FloatingPreview::CreateBtn(const QString& tag, QWidget* parent) {
  QPushButton* btn = new (std::nothrow) QPushButton{"> " + tag, parent};
  if (btn == nullptr) {
    qCritical("btn is nullptr");
    return nullptr;
  }
  btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  btn->setStyleSheet(R"(
QPushButton{
  border: 1px solid #ccc;
  border-radius: 0px;
  background-color: #f0f0f0;
  padding: 0px;
  margin: 0px 0;
  text-align: left;
}
QPushButton:hover{
  background-color: #FFA500;
}
QPushButton:checked{
  background-color: #d0d0d0;
}
)");
  btn->setCheckable(true);
  return btn;
}

FloatingPreview::FloatingPreview(QWidget* parent) : QWidget{parent} {
  mImgBtn = CreateBtn("Images", this);
  mVidsBtn = CreateBtn("Videos", this);
  mOthersBtn = CreateBtn("Others", this);

  mImgTv = new QListView{this};
  mImgModel = new ImgsModel{mImgTv};
  mImgTv->setModel(mImgModel);

  mImgTv->setViewMode(QListView::ViewMode::ListMode);
  mImgTv->setFlow(QListView::Flow::LeftToRight);
  mImgTv->setTextElideMode(Qt::TextElideMode::ElideMiddle);
  mImgTv->setUniformItemSizes(false);

  mImgTv->setResizeMode(QListView::ResizeMode::Adjust);
  mImgTv->setMovement(QListView::Movement::Free);
  mImgTv->setWrapping(true);
  mImgTv->setMinimumHeight(380);

  mVLo = new QVBoxLayout;
  mVLo->addWidget(mImgTv);
  mVLo->addWidget(mImgBtn);
  mVLo->addWidget(mVidsBtn);
  mVLo->addWidget(mOthersBtn);
  setLayout(mVLo);

  mImgBtn->setChecked(true);
  onImgBtnClicked(true);

  subscribe();

  ReadSettings();
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

bool FloatingPreview::NeedUpdate(const QString& lastName) const {
  return mLastName.isEmpty() || mLastName != lastName;
}

bool FloatingPreview::operator()(const QString& pth) {  // file system
  if (!NeedUpdate(pth)) {
    return true;
  }
  mLastName = pth;
  setWindowTitle(mLastName);
  return true;
}

bool FloatingPreview::operator()(const QString& name, const QString& pth) {  // scene
  mLastName = name;
  setWindowTitle(mLastName);

  QDir dir{pth, "", QDir::SortFlag::Name, QDir::Filter::Files};
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  mImgModel->UpdateImgs(dir.entryList());
  return true;
}

bool FloatingPreview::operator()(const QString& name, const QStringList& imgPthLst) {
  mLastName = name;
  setWindowTitle(mLastName);
  mImgModel->UpdateImgs(imgPthLst);
  return true;
}

void FloatingPreview::subscribe() {
  connect(mImgBtn, &QPushButton::clicked, this, &FloatingPreview::onImgBtnClicked);
  connect(mImgTv, &QListView::doubleClicked, this, &FloatingPreview::on_cellDoubleClicked);
//  connect(mVidsBtn, &QPushButton::clicked, this, nullptr);
//  connect(mOthersBtn, &QPushButton::clicked, this, nullptr);
}

bool FloatingPreview::onImgBtnClicked(bool checked) {
  if (checked){
    mImgBtn->setText("v Images");
    mImgBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mVidsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mOthersBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mImgTv->show();
  }else{
    mImgBtn->setText("> Images");
    mImgBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mVidsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mOthersBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mImgTv->hide();
  }
  return true;
}

void FloatingPreview::on_cellDoubleClicked(const QModelIndex& clickedIndex) const {
  const QString& path = mImgModel->filePath(clickedIndex);
  const bool ret = QDesktopServices::openUrl(QUrl::fromLocalFile(path));
  Notificator::information(QString("Try open [%1]:").arg(path), QString::number(ret));
}
