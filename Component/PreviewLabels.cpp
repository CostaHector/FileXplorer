#include "PreviewLabels.h"
#include "PublicVariable.h"

constexpr int PreviewLabels::NEXT_FOLDER_TIME_INTERVAL;        // ms
constexpr int PreviewLabels::SLIDE_TO_NEXT_IMG_TIME_INTERVAL;  // ms
constexpr int PreviewLabels::MAX_LABEL_CNT;

bool PreviewLabels::getImgsPathAndVidsCount(const QString& path, QStringList& imgs, int& vidsCnt) const {
  imgs.clear();
  vidsCnt = 0;

  const QString& suffix = "*." + QFileInfo(path).suffix().toLower();
  QFileInfo pathFi(path);
  if (pathFi.isFile()) {
    if (TYPE_FILTER::IMAGE_TYPE_SET.contains(suffix)) {
      imgs = QStringList{path};
    }
    if (TYPE_FILTER::VIDEO_TYPE_SET.contains(suffix)) {
      vidsCnt = 1;
    }
    return true;
  } else if (not pathFi.isDir()) {
    imgs.clear();
    vidsCnt = 0;
    return true;
  }

  QDir dir(path, "*", QDir::SortFlag::NoSort, QDir::Filter::Files);
  dir.setNameFilters(TYPE_FILTER::IMAGE_TYPE_SET);
  for (const QString& imgName : dir.entryList()) {
    imgs.append(dir.absoluteFilePath(imgName));
  }
  dir.setNameFilters(TYPE_FILTER::VIDEO_TYPE_SET);
  vidsCnt = dir.entryList().size();
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
    QPixmap pm(m_imgsUnderAPath[m_inFolderImgIndex++]);
    m_imgLabelsList[labelCnt]->setPixmap(pm.scaledToWidth(width()));
    m_isLabelDirty = true;
  }
}

auto PreviewLabels::display() -> void {
  QString lastPath = m_folderPathsStack.back();
  // list will be cleared. dont use reference to its element
  clear();

  m_inFolderImgIndex = 0;
  getImgsPathAndVidsCount(lastPath, m_imgsUnderAPath, m_vidsCountUnderAPath);
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
