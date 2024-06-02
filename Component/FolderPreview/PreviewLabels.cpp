#include "PreviewLabels.h"
#include "PublicVariable.h"

#include "Tools/ArchiveFiles.h"
constexpr int PreviewLabels::SLIDE_TO_NEXT_IMG_TIME_INTERVAL;  // ms
constexpr int PreviewLabels::MAX_LABEL_CNT;

void PreviewLabels::operator()(const QString& folderPath) {
  if (m_nextImgTimer->isActive()) {
    m_nextImgTimer->stop();
  }
  m_inFolderImgIndex = 0;
  getImgsPathAndVidsCount(folderPath);
  setDockerWindowTitle();
  if (m_imgsUnderAPath.isEmpty()) {
    clearLabelContents();
    return;
  }

  nxtImgInFolder();
  if (m_imgsUnderAPath.size() > MAX_LABEL_CNT) {
    m_nextImgTimer->start();
  }
}

bool PreviewLabels::getImgsPathAndVidsCount(const QString& path) {
  m_imgsUnderAPath.clear();
  m_vidsCountUnderAPath = 0;

  QFileInfo pathFi(path);
  if (pathFi.isFile()) {
    const QString& suffix = "*." + QFileInfo(path).suffix().toLower();
    if (TYPE_FILTER::IMAGE_TYPE_SET.contains(suffix)) {
      m_imgsUnderAPath = QVariantList{path};
    } else if (TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET.contains(suffix)) {
      ArchiveFiles af{path};
      m_imgsUnderAPath = af.PreviewFirstKItems(-1);
    }
    if (TYPE_FILTER::VIDEO_TYPE_SET.contains(suffix)) {
      m_vidsCountUnderAPath = 1;
    }
    return true;
  }
  if (pathFi.isDir()) {
    QDir dir(path, "*", QDir::SortFlag::NoSort, QDir::Filter::Files);
    dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
    for (const QString& imgName : dir.entryList()) {
      m_imgsUnderAPath.append(dir.absoluteFilePath(imgName));
    }
    // show .qz as image preview only when images not exist
    if (m_imgsUnderAPath.isEmpty()) {
      dir.setNameFilters(TYPE_FILTER::BUILTIN_COMPRESSED_TYPE_SET);
      const QStringList& qzNamesList = dir.entryList();
      if (not qzNamesList.isEmpty()) {
        // only show first qz file
        const QString& qzFilePath = dir.absoluteFilePath(qzNamesList.front());
        ArchiveFiles af{qzFilePath};
        m_imgsUnderAPath = af.PreviewFirstKItems(-1);
      }
    }
    dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
    m_vidsCountUnderAPath = dir.entryList().size();
    return true;
  }
  return true;
}

auto PreviewLabels::nxtImgInFolder() -> void {
  for (int labelCnt = 0; labelCnt < MAX_LABEL_CNT; ++labelCnt) {
    if (m_inFolderImgIndex >= m_imgsUnderAPath.size()) {
      if (m_nextImgTimer->isActive()) {
        m_nextImgTimer->stop();
      }
      m_imgLabelsList[labelCnt]->setText("placeholder");
      continue;
    }
    const QVariant& imgEle = m_imgsUnderAPath[m_inFolderImgIndex];
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
    m_imgLabelsList[labelCnt]->setPixmap(pm.scaledToWidth(width()));
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
