#include "FloatingPreview.h"
#include "PublicVariable.h"
#include <QDir>
#include <QHeaderView>

QVariant ImgsModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return {};
  }
  if (role == Qt::DecorationRole) {
    const int column = index.column();
    const QPixmap pm{mImgsLst[column]};
    if (pm.width() * 280 >= pm.height() * 480) {
      return pm.scaledToWidth(480);
    }
    return pm.scaledToHeight(280);
  }
  return {};
}
void ImgsModel::UpdateImgs(const QStringList& newImgsLst) {  // dont use newImgsLst after
  ColumnsBeginChange(mImgsLst.size(), newImgsLst.size());
  mImgsLst = newImgsLst;
  ColumnsEndChange();
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
  border-radius: 5px;
  background-color: #f0f0f0;
  padding: 8px;
  margin: 5px 0;
  text-align: left;
}
QPushButton:hover{
  background-color: #e0e0e0;
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

  mImgTv = new QTableView{this};
  mImgModel = new ImgsModel{mImgTv};
  mImgTv->setModel(mImgModel);

  mImgTv->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
  mImgTv->verticalHeader()->hide();

  mImgTv->horizontalHeader()->setDefaultSectionSize(360);
  mImgTv->horizontalHeader()->setStretchLastSection(true);
  mImgTv->horizontalHeader()->hide();
  mImgTv->setMinimumHeight(380);

  mVLo = new QVBoxLayout;
  mVLo->addWidget(mImgBtn);
  mVLo->addWidget(mImgTv);
  mVLo->addWidget(mVidsBtn);
  mVLo->addWidget(mOthersBtn);
  setLayout(mVLo);

  mImgBtn->setChecked(true);
  onImgBtnClicked(true);
  subscribe();
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
//  connect(mVidsBtn, &QPushButton::clicked, this, nullptr);
//  connect(mOthersBtn, &QPushButton::clicked, this, nullptr);
}

bool FloatingPreview::onImgBtnClicked(bool checked) {
  if (checked){
    mImgBtn->setText("v Images");
    mImgBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    mImgTv->show();
  }else{
    mImgBtn->setText("> Images");
    mImgBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mImgTv->hide();
  }
  return true;
}
