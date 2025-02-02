#ifndef HARTABLEVIEW_H
#define HARTABLEVIEW_H

#include "View/CustomTableView.h"
#include "Model/HarModel.h"
#include <QSortFilterProxyModel>
#include <QMenu>

class HarTableView : public CustomTableView {
 public:
  explicit HarTableView(QWidget* parent = nullptr);
  int operator()(const QString& harAbsPath);
  void subscribe();
  QSize sizeHint() const { return QSize(1024, 768); }
  int SaveSelectionFilesTo();
  bool PreviewImage() const;

 private:
  QString GetWinTitleStr(const QString& harFile={}) const;
  QString mHarAbsPath;
  HarModel* mHarModel;
  QSortFilterProxyModel *mSortProxyModel;
  QMenu *mMenu {nullptr};
  QAction *mEXPORT_TO {nullptr};
};

#endif // HARTABLEVIEW_H
