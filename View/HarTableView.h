#ifndef HARTABLEVIEW_H
#define HARTABLEVIEW_H

#include "CustomTableView.h"
#include "HarModel.h"
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QLabel>

class HarTableView : public CustomTableView {
 public:
  explicit HarTableView(QWidget* parent = nullptr);
  int operator()(const QString& harAbsPath);
  void subscribe();
  int SaveSelectionFilesTo() const;
  bool PreviewImage();

  void updateWindowsSize();
  void showEvent(QShowEvent *event) override;
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
  QLabel *mPreviewLabel{nullptr};
};

#endif // HARTABLEVIEW_H
