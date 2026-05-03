#ifndef HARTABLEVIEW_H
#define HARTABLEVIEW_H

#include "CustomTableView.h"
#include "HarModel.h"
#include <QSortFilterProxyModel>

class HarTableView : public CustomTableView {
  Q_OBJECT
public:
  explicit HarTableView(QWidget* parent = nullptr);
  int operator()(const QString& harAbsPath);
  void setFilter(const QString& filter);
  QString GetWinTitleStr(const QString& harFile) const;

signals:
  void pixmapByteArrayChanged(const QByteArray& pm, const QString& noDotFormatStr);

private:
  void subscribe();
  bool PreviewImage(const QModelIndex& current);
  int SaveSelectionFilesTo() const;

  QString mHarAbsPath;
  HarModel* mHarModel{nullptr};
  QSortFilterProxyModel* mSortFilterProxy{nullptr};

  QAction* mEXPORT_TO{nullptr};
};

#endif // HARTABLEVIEW_H
