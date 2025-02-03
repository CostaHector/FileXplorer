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
  int SaveSelectionFilesTo() const;
  bool PreviewImage() const;

  void updateWindowsSize();
  void closeEvent(QCloseEvent* event) override;
 private:
  QString GetWinTitleStr(const QString& harFile={}) const;
  QString mHarAbsPath;
  bool mShowImagePreview;
  HarModel* mHarModel;
  QSortFilterProxyModel *mSortProxyModel;
  QMenu *mMenu {nullptr};
  QAction *mEXPORT_TO {nullptr};
  QAction *mQUICK_PREVIEW{nullptr};
};

#endif // HARTABLEVIEW_H
