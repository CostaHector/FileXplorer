#ifndef IMAGESINFOLDERSLIDER_H
#define IMAGESINFOLDERSLIDER_H

#include <QLabel>
#include <QScrollBar>

#include <QDir>
#include <QFileInfo>
#include <QPixmap>
#include <QScrollArea>
#include <QSet>
#include <QStack>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

class FilesListBase;

class ImagesInFolderSlider : public QScrollArea {
public:
  explicit ImagesInFolderSlider(QWidget* parent = nullptr);
  ~ImagesInFolderSlider() { ResetImgsList(); }

  void operator()(const QString& folderPath);

  void ResetImgsList(FilesListBase* pImgsList = nullptr);

  void clearLabelContents() {
    if (!m_isLabelDirty) {
      return;
    }
    for (auto* pLabel : m_imgLabelsList) {
      pLabel->clear();
    }
  }

  bool getImgsPathAndVidsCount(const QString& path);

  auto nxtImgInFolder() -> void;

private:
  QList<QLabel*> m_imgLabelsList;
  QVBoxLayout* m_labelsLayout;

  int m_vidsCountUnderAPath = 0;
  FilesListBase* m_imgsUnderAPath{nullptr}; // release when destructor

  QTimer* m_nextImgTimer = new QTimer{this};

  int m_inFolderImgIndex = 0;
  bool m_isLabelDirty = false;

  static constexpr int SLIDE_TO_NEXT_IMG_TIME_INTERVAL = 2 * 1000;  // ms
  static constexpr int MAX_LABEL_CNT = 2;
};

#endif  // IMAGESINFOLDERSLIDER_H
