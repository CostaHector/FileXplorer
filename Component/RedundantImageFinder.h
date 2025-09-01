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

  void operator()(const QString& folderPath);

  void subscribe();

  void RecycleSelection();

  void ChangeWindowTitle(const QString& rootPath);

private:
  void whenModeChanged();
  void UpdateDisplayWhenRecycled();
  QToolButton* mFindImgByTb{nullptr};
  QToolBar* m_toolBar{nullptr};
  REDUNDANT_IMG_BUNCH m_imgsBunch;

  RedundantImageModel* m_imgModel{nullptr};
  CustomTableView* m_table{nullptr};
  QString mCurrentPath;
  void setResultAlsoContainEmptyImage(bool alsoContain) { mResultAlsoContainEmptyImage = alsoContain; }
  bool mResultAlsoContainEmptyImage;
};

#endif  // REDUNDANTIMAGEFINDER_H
