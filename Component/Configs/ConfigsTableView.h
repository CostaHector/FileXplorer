#ifndef CONFIGSTABLEVIEW_H
#define CONFIGSTABLEVIEW_H

#include "CustomTableView.h"
#include "StyleSheetEditDelegate.h"
#include <QSortFilterProxyModel>

class ConfigsModel;

class ConfigsTableView : public CustomTableView {
  Q_OBJECT
public:
  explicit ConfigsTableView(const QString& instName, QWidget* parent = nullptr);
  int GetFailedCnt() const;
  void setFilter(const QString& filter);

signals:
  void modelCfgFailedCountChanged(int newFailedCnt);

protected:
  void initExclusivePreferenceSetting() override;

private:
  void subscribe();
  bool on_cellDoubleClicked(const QModelIndex& proxyIndex) const;

  ConfigsModel* m_cfgModel{nullptr};
  QSortFilterProxyModel* mSortFilterProxy{nullptr};
  StyleSheetEditDelegate* mStyleSheetEditDelegate{nullptr};
};

#endif // CONFIGSTABLEVIEW_H
