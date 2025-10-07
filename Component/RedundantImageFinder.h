#ifndef REDUNDANTIMAGEFINDER_H
#define REDUNDANTIMAGEFINDER_H

#include "CustomTableView.h"
#include "RedundantImageModel.h"
#include <QMainWindow>
#include <QToolButton>
#include <QToolBar>

class RedundantImageFinder : public QMainWindow {
public:
  explicit RedundantImageFinder(QWidget* parent = nullptr);
  void ReadSetting();
  void showEvent(QShowEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  bool operator()(const QString& folderPath);

  void subscribe();

  void RecycleSelection();

  void ChangeWindowTitle(const QString& rootPath);

private:
  void whenModeChanged();
  void UpdateDisplayWhenRecycled();
  QToolButton* mFindImgByTb{nullptr};
  QToolBar* m_toolBar{nullptr};
  DuplicateImageMetaInfo::RedundantImagesList m_imgsBunch;

  RedundantImageModel* m_imgModel{nullptr};
  CustomTableView* m_table{nullptr};
  QString mCurrentPath;
  void setResultAlsoContainEmptyImage(bool alsoContain) { mResultAlsoContainEmptyImage = alsoContain; }
  bool mResultAlsoContainEmptyImage {true};
  static constexpr char GEOMETRY_KEY[]{"RedunImgFinderKey/GEOMETRY"};
};

#endif  // REDUNDANTIMAGEFINDER_H
