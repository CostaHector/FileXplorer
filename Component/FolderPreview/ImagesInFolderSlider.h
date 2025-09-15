#ifndef IMAGESINFOLDERSLIDER_H
#define IMAGESINFOLDERSLIDER_H

#include <QWidget>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>

class FilesListBase;

class ImagesInFolderSlider : public QScrollArea {
public:
  explicit ImagesInFolderSlider(QWidget* parent = nullptr);
  ~ImagesInFolderSlider() { ResetImgsList(nullptr); }

  void operator()(const QString& folderPath);

  void ResetImgsList(FilesListBase* pImgsList = nullptr);

  void clearLabelContents() {
    foreach (auto* pLabel, m_imgLabelsList) {
      if(pLabel != nullptr) {pLabel->clear();}
    }
  }

  void getImgsPathAndVidsCount(const QString& path);
  void nxtImgInFolder();
  bool hasNextImgs() const;
  void disableTimer() {
    if (m_nextImgTimer != nullptr && m_nextImgTimer->isActive()) {
      m_nextImgTimer->stop();
    }
  }

private:
  QList<QLabel*> m_imgLabelsList;
  QVBoxLayout* m_labelsLayout{nullptr};
  FilesListBase* m_imgsUnderAPath{nullptr}; // release in destructor

  QTimer* m_nextImgTimer{nullptr};

  int m_inFolderImgIndex = 0;
  bool m_isLabelDirty = false;

  static constexpr int SLIDE_TO_NEXT_IMG_TIME_INTERVAL = 2 * 1000;  // ms
  static constexpr int MAX_LABEL_CNT = 2;
};

#endif  // IMAGESINFOLDERSLIDER_H
