#include "ImagesInFolderSlider.h"
#include "PublicVariable.h"
#include "ArchiveFiles.h"

constexpr int ImagesInFolderSlider::SLIDE_TO_NEXT_IMG_TIME_INTERVAL;  // ms
constexpr int ImagesInFolderSlider::MAX_LABEL_CNT;
constexpr int ImagesInFolderSlider::mWidth, ImagesInFolderSlider::mHeight;

ImagesInFolderSlider::ImagesInFolderSlider(QWidget* parent)
  : QScrollArea{parent},//
  m_labelsLayout{new (std::nothrow) QVBoxLayout{this}}
{
  m_imgLabelsList.reserve(MAX_LABEL_CNT);
  for (int imgCnt = 0; imgCnt < MAX_LABEL_CNT; ++imgCnt) {
    m_imgLabelsList.append(new (std::nothrow) QLabel{this});
    m_labelsLayout->addWidget(m_imgLabelsList[imgCnt]);
  }
  setWidgetResizable(true);
  setLayout(m_labelsLayout);

  m_nextImgTimer->setInterval(SLIDE_TO_NEXT_IMG_TIME_INTERVAL);
  m_nextImgTimer->setSingleShot(false);
  connect(m_nextImgTimer, &QTimer::timeout, this, &ImagesInFolderSlider::nxtImgInFolder);
}

void ImagesInFolderSlider::operator()(const QString& folderPath) {
  if (m_nextImgTimer->isActive()) {
    m_nextImgTimer->stop();
  }
  m_inFolderImgIndex = 0;
  getImgsPathAndVidsCount(folderPath);
  if (m_imgsUnderAPath != nullptr && m_imgsUnderAPath->isEmpty()) {
    clearLabelContents();
    return;
  }

  nxtImgInFolder();
  if (m_imgsUnderAPath != nullptr && m_imgsUnderAPath->size() > MAX_LABEL_CNT) {
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

bool ImagesInFolderSlider::getImgsPathAndVidsCount(const QString& path) {
  if (m_imgsUnderAPath != nullptr) {
    m_imgsUnderAPath->clear();
  }
  m_vidsCountUnderAPath = 0;

  const QFileInfo pathFi{path};
  if (pathFi.isFile()) {
    const QString& suffix = "*." + QFileInfo(path).suffix().toLower();
    if (TYPE_FILTER::IMAGE_TYPE_SET.contains(suffix)) {
      ResetImgsList(new PlainStringList(QStringList{path}));
    } else if (TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET.contains(suffix)) {
      ResetImgsList(new ArchiveFiles{path});
    }
    if (TYPE_FILTER::VIDEO_TYPE_SET.contains(suffix)) {
      m_vidsCountUnderAPath = 1;
    }
    return true;
  } else if (pathFi.isDir()) {
    QDir dir{path, "", QDir::SortFlag::NoSort, QDir::Filter::Files};
    dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
    QStringList imgs;
    for (const QString& imgName : dir.entryList()) {
      imgs.append(dir.absoluteFilePath(imgName));
    }

    // show .qz as image preview only when images not exist
    if (!imgs.isEmpty()) {
      ResetImgsList(new (std::nothrow) PlainStringList(imgs));
    } else {
      dir.setNameFilters(TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET);
      const QStringList& qzNamesList = dir.entryList();
      if (not qzNamesList.isEmpty()) {
        // only show first qz file
        const QString& qzFilePath = dir.absoluteFilePath(qzNamesList.front());
        ResetImgsList(new (std::nothrow) ArchiveFiles{qzFilePath});
      }
    }
    dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    m_vidsCountUnderAPath = dir.entryList().size();
    return true;
  }
  return true;
}

void ImagesInFolderSlider::nxtImgInFolder() {
  if (m_imgsUnderAPath == nullptr) {
    return;
  }
  for (int labelCnt = 0; labelCnt < MAX_LABEL_CNT; ++labelCnt) {
    if (m_inFolderImgIndex >= m_imgsUnderAPath->size()) {
      if (m_nextImgTimer->isActive()) {
        m_nextImgTimer->stop();
      }
      m_imgLabelsList[labelCnt]->setText("placeholder");
      continue;
    }
    const QVariant& imgEle = (*m_imgsUnderAPath)[m_inFolderImgIndex];
    ++m_inFolderImgIndex;

    m_isLabelDirty = true;
    QPixmap pm;
    if (imgEle.type() == QVariant::String) {
      pm.load(imgEle.toString());
    } else if (imgEle.type() == QVariant::ByteArray) {
      pm.loadFromData(imgEle.toByteArray());
    } else {
      qWarning("imgEle type is invalid %d", int(imgEle.type()));
      return;
    }
    if (pm.width() * mHeight >= pm.height() * mWidth) {
      pm = pm.scaledToWidth(mWidth, Qt::SmoothTransformation);
    } else {
      pm = pm.scaledToHeight(mHeight, Qt::SmoothTransformation);
    }
    m_imgLabelsList[labelCnt]->setPixmap(pm);
  }
}

// #define __NAME__EQ__MAIN__ 1
#ifdef __NAME__EQ__MAIN__
#include <QApplication>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QMainWindow>
#include <QTableView>
#include <QWidget>

#include <QItemSelection>
#include <QItemSelectionModel>
#include <QItemSelectionRange>
#include <QModelIndex>
#include <Qt>

class PreviewLabelTest : public QMainWindow {
public:
  QTableView* m_tv{new QTableView{this}};
  QFileSystemModel* m_fsm{new QFileSystemModel{this}};
  PreviewLabel* m_dc{new PreviewLabel{this}};
  QDockWidget* m_docker{new QDockWidget};
  PreviewLabelTest(QWidget* parent = nullptr) : QMainWindow{parent} {
    m_tv->setModel(m_fsm);
    m_tv->setRootIndex(m_fsm->setRootPath("E:/115/0419"));
    connect(m_tv->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PreviewLabelTest::onChanged);

    m_docker->setWidget(m_dc);
    setCentralWidget(m_tv);
    addDockWidget(Qt::DockWidgetArea::RightDockWidgetArea, m_docker);
    setMinimumSize(1024, 768);
    m_docker->setFixedWidth(400);
  }
  void onChanged(const QItemSelection& newSelection, const QItemSelection& oldSelection) {
    const QModelIndex& lastIndex = m_tv->currentIndex();
    if (not lastIndex.isValid()) {
      return;
    }
    const QString& path = m_fsm->filePath(lastIndex);
    m_dc->operator()(path);
  }
};

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);
  PreviewLabelTest wid;
  wid.show();
  return a.exec();
}
#endif
