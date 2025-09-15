#include "ImagesInFolderSlider.h"
#include "PublicVariable.h"
#include "ArchiveFiles.h"
#include "PublicMacro.h"
#include <QPixmap>
#include <QDir>
#include <QFileInfo>

constexpr int ImagesInFolderSlider::SLIDE_TO_NEXT_IMG_TIME_INTERVAL;  // ms
constexpr int ImagesInFolderSlider::MAX_LABEL_CNT;

ImagesInFolderSlider::ImagesInFolderSlider(QWidget* parent)
  : QScrollArea{parent} {
  m_imgLabelsList.reserve(MAX_LABEL_CNT);
  for (int imgCnt = 0; imgCnt < MAX_LABEL_CNT; ++imgCnt) {
    auto* pLabel = new (std::nothrow) QLabel{this};
    CHECK_NULLPTR_RETURN_VOID(pLabel);
    m_imgLabelsList.append(pLabel);
  }

  m_labelsLayout = new (std::nothrow) QVBoxLayout{this};
  CHECK_NULLPTR_RETURN_VOID(m_labelsLayout);
  foreach (auto* pLabel, m_imgLabelsList) {
    m_labelsLayout->addWidget(pLabel);
  }
  // setWidgetResizable(true);
  setLayout(m_labelsLayout);
  m_labelsLayout->setSpacing(0);
  m_labelsLayout->setContentsMargins(0, 0, 0, 0);

  m_nextImgTimer = new (std::nothrow) QTimer{this};
  CHECK_NULLPTR_RETURN_VOID(m_nextImgTimer);
  m_nextImgTimer->setInterval(SLIDE_TO_NEXT_IMG_TIME_INTERVAL);
  m_nextImgTimer->setSingleShot(false);

#ifndef RUNNING_UNIT_TESTS
  connect(m_nextImgTimer, &QTimer::timeout, this, &ImagesInFolderSlider::nxtImgInFolder);
#endif
}

void ImagesInFolderSlider::operator()(const QString& folderPath) {
  disableTimer();
  m_inFolderImgIndex = 0;
  getImgsPathAndVidsCount(folderPath);
  if (m_imgsUnderAPath == nullptr || m_imgsUnderAPath->isEmpty()) {
    clearLabelContents();
    return;
  }
  nxtImgInFolder();
  if (hasNextImgs()) {
    m_nextImgTimer->start();
  }
}

void ImagesInFolderSlider::ResetImgsList(FilesListBase* pImgsList) {
  if (m_imgsUnderAPath == nullptr) {
    m_imgsUnderAPath = pImgsList;
    return;
  }
  delete m_imgsUnderAPath;
  m_imgsUnderAPath = pImgsList;
}

void ImagesInFolderSlider::getImgsPathAndVidsCount(const QString& path) {
  if (m_imgsUnderAPath != nullptr) {
    m_imgsUnderAPath->clear();
  }
  const QFileInfo pathFi{path};
  if (pathFi.isFile()) {
    const QString& suffix = "*." + pathFi.suffix().toLower();
    if (TYPE_FILTER::IMAGE_TYPE_SET.contains(suffix)) {
      ResetImgsList(new PlainStringList(QStringList{path}));
    } else if (TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET.contains(suffix)) {
      ResetImgsList(new ArchiveFiles{path});
    } else {
      ResetImgsList(nullptr);
    }
    return;
  }
  if (pathFi.isDir()) {
    QDir dir{path, "", QDir::SortFlag::Name, QDir::Filter::Files};
    dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
    QStringList imgs;
    foreach (const QString& imgName, dir.entryList()) {
      imgs.append(dir.absoluteFilePath(imgName));
    }
    if (imgs.isEmpty()) {
      ResetImgsList(nullptr);
      return;
    }
    ResetImgsList(new (std::nothrow) PlainStringList(imgs));
    return;
  }
}

void ImagesInFolderSlider::nxtImgInFolder() {
  if (!hasNextImgs()) {
    return;
  }

  const int mWidth = width();
  const int mHeight = height() / MAX_LABEL_CNT;

  for (int labelCnt = 0; labelCnt < MAX_LABEL_CNT; ++labelCnt) {
    if (m_inFolderImgIndex >= m_imgsUnderAPath->size()) {
      m_imgLabelsList[labelCnt]->clear();
      break;
    }

    const QVariant& imgEle = (*m_imgsUnderAPath)[m_inFolderImgIndex];
    ++m_inFolderImgIndex;

    QPixmap pm;
    if (imgEle.type() == QVariant::String) {
      pm.load(imgEle.toString());
    } else if (imgEle.type() == QVariant::ByteArray) {
      pm.loadFromData(imgEle.toByteArray());
    } else {
      LOG_W("imgEle type is invalid %d", int(imgEle.type()));
      return;
    }

    if (pm.width() * mHeight >= pm.height() * mWidth) {
      pm = pm.scaledToWidth(mWidth, Qt::FastTransformation);
    } else {
      pm = pm.scaledToHeight(mHeight, Qt::FastTransformation);
    }

    m_imgLabelsList[labelCnt]->setPixmap(pm);
  }

  if (m_inFolderImgIndex >= m_imgsUnderAPath->size()) {
    disableTimer();
  }
}

bool ImagesInFolderSlider::hasNextImgs() const {
  return m_imgsUnderAPath != nullptr && m_inFolderImgIndex < m_imgsUnderAPath->size();
}
