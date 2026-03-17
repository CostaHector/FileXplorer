#ifndef IMGREORDERLISTVIEW_H
#define IMGREORDERLISTVIEW_H

#include "CustomListView.h"
#include "ImgReorderListModel.h"

class ImgReorderListView : public CustomListView {
 public:
  explicit ImgReorderListView(QWidget* parent = nullptr);
  bool setImagesToReorder(const QStringList& imgs, const QString& baseName, int startIndex = 0, const QString& namePattern = " %1");
  QStringList getOrderedNames() const;

 protected:
  void dropEvent(QDropEvent* event) override;

 private:
  int calculateInsertionRow(const QPoint& pos) const;

  ImgReorderListModel* mImgReorderListModel{nullptr};
};

#endif  // IMGREORDERLISTVIEW_H
