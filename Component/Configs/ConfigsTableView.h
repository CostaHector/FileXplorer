#ifndef CONFIGSTABLEVIEW_H
#define CONFIGSTABLEVIEW_H

#include "CustomTableView.h"
#include "ConfigsModel.h"
#include "StyleSheetEditDelegate.h"

class ConfigsTableView : public CustomTableView {
public:
  explicit ConfigsTableView(const QString& instName, QWidget* parent = nullptr);

  const QAbstractTableModel* GetModel() const { return m_alertModel; }
  QAbstractTableModel* GetModel() { return m_alertModel; }

  std::pair<int, int> GetStatistics() const {
    const int failsCnt{m_alertModel->failCount()};
    const int totalCnt{m_alertModel->rowCount()};
    return {failsCnt, totalCnt};
  }

protected:
  void initExclusivePreferenceSetting() override;

private:
  void subscribe();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  ConfigsModel* m_alertModel{nullptr};
  StyleSheetEditDelegate* mStyleSheetEditDelegate{nullptr};
};

#endif // CONFIGSTABLEVIEW_H
