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
  bool onNormalizeKeepRelativeOrder();
  bool onOpenCurrentIndexInSystemApplication() const;

 protected:
  void dropEvent(QDropEvent* event) override;

 private:
  int calculateInsertionRow(const QPoint& pos) const;
  void initExclusivePreferenceSetting() override;

  QAction* mNormalizeKeepRelativeOrder{nullptr};
  QAction* mOpenInSystemApplication{nullptr};
  ImgReorderListModel* mImgReorderListModel{nullptr};
};

#endif  // IMGREORDERLISTVIEW_H
