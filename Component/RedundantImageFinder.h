#ifndef REDUNDANTIMAGEFINDER_H
#define REDUNDANTIMAGEFINDER_H

#include "Model/RedundantImageModel.h"
#include "View/CustomTableView.h"
#include <QMainWindow>

class RedundantImageFinder : public QMainWindow {
 public:
  explicit RedundantImageFinder(QWidget* parent = nullptr);
  ~RedundantImageFinder();
  void ReadSetting();
  void closeEvent(QCloseEvent* event) override;

  void operator()(const QString& folderPath);

  void subscribe();

  void RecycleSelection();

  void ChangeWindowTitle(const QString& rootPath);

 private:
  void UpdateDisplayWhenRecycled();

  REDUNDANT_IMG_BUNCH m_imgsBunch;

  RedundantImageModel* m_imgModel{nullptr};
  CustomTableView* m_table{nullptr};

  static RedunImgLibs mRedunLibs;
};

#endif  // REDUNDANTIMAGEFINDER_H
