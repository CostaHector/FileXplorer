#ifndef IMGREORDERLISTVIEW_H
#define IMGREORDERLISTVIEW_H

#include "CustomListView.h"
#include "ImgReorderListModel.h"

class ImgReorderListView : public CustomListView {
  Q_OBJECT
 public:
  explicit ImgReorderListView(QWidget* parent = nullptr);
  bool setImagesToReorder(const QStringList& imgs, const QString& baseName, int startIndex = 0, const QString& namePattern = " %1");
  QStringList getOrderedNames() const;

 protected:
  void dropEvent(QDropEvent* event) override;

 private:
  void subscribe();
  int calculateInsertionRow(const QPoint& pos) const;
  void initExclusivePreferenceSetting() override;

  bool onBatchShiftSelectedRowsByStep(int step = 100);
  bool onBatchShiftCustomUnit();
  bool onNormalizeKeepRelativeOrder();
  bool onOpenCurrentIndexInSystemApplication() const;

  QAction *mBatchShiftRight100{nullptr}, *mBatchShiftLeft100{nullptr}, *mBatchShiftCustomUnit{nullptr};
  QAction* mNormalizeKeepRelativeOrder{nullptr};
  QAction* mOpenInSystemApplication{nullptr};
  ImgReorderListModel* mImgReorderListModel{nullptr};
};

#endif  // IMGREORDERLISTVIEW_H
