#ifndef PREVIEWLABELS_H
#define PREVIEWLABELS_H

#include <QLabel>
#include <QScrollBar>

#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QScrollArea>
#include <QSet>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QStack>

class PreviewLabels : public QScrollArea {
 public:
  explicit PreviewLabels(QWidget* parent = nullptr)
      : QScrollArea{parent}, m_parentDocker{parent}, m_labelsLayout{new QVBoxLayout{this}}, m_imgsBanner{new QWidget{this}} {
    m_imgLabelsList.reserve(MAX_LABEL_CNT);
    for (int imgCnt = 0; imgCnt < MAX_LABEL_CNT; ++imgCnt) {
      m_imgLabelsList.append(new QLabel{this});
      m_labelsLayout->addWidget(m_imgLabelsList[imgCnt]);
    }
    m_imgsBanner->setLayout(m_labelsLayout);
    setWidgetResizable(true);
    setWidget(m_imgsBanner);

    m_nextImgTimer->setInterval(SLIDE_TO_NEXT_IMG_TIME_INTERVAL);
    m_nextImgTimer->setSingleShot(false);
    connect(m_nextImgTimer, &QTimer::timeout, this, &PreviewLabels::nxtImgInFolder);
  }

  void setDockerWindowTitle() {
    if (m_parentDocker == nullptr) {
      return;
    }
    m_parentDocker->setWindowTitle(QString::number(m_vidsCountUnderAPath) + '|' + QString::number(m_imgsUnderAPath.size()));
  }

  void operator()(const QString& folderPath);

  void clearLabelContents() {
    if (not m_isLabelDirty)
      return;
    for (auto* pLabel : m_imgLabelsList) {
      pLabel->clear();
    }
  }

  bool getImgsPathAndVidsCount(const QString& path);

  auto nxtImgInFolder() -> void;

 private:
  QWidget* m_parentDocker;

  QList<QLabel*> m_imgLabelsList;
  QVBoxLayout* m_labelsLayout;
  QWidget* m_imgsBanner;

  int m_vidsCountUnderAPath = 0;
  QVariantList m_imgsUnderAPath;

  QTimer* m_nextImgTimer = new QTimer{this};

  int m_inFolderImgIndex = 0;
  bool m_isLabelDirty = false;

  static constexpr int SLIDE_TO_NEXT_IMG_TIME_INTERVAL = 2 * 1000;  // ms
  static constexpr int MAX_LABEL_CNT = 2;
};

#endif  // PREVIEWLABELS_H
