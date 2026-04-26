#ifndef CONFIGSTABLEVIEW_H
#define CONFIGSTABLEVIEW_H

#include "CustomTableView.h"
#include "StyleSheetEditDelegate.h"

class ConfigsModel;

class ConfigsTableView : public CustomTableView {
  Q_OBJECT
public:
  explicit ConfigsTableView(const QString& instName, QWidget* parent = nullptr);

  const ConfigsModel* GetModel() const { return m_alertModel; }
  ConfigsModel* GetModel() { return m_alertModel; }

  std::pair<int, int> GetStatistics() const;

signals:
  void modelDataChanged();

protected:
  void initExclusivePreferenceSetting() override;

private:
  void subscribe();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  ConfigsModel* m_alertModel{nullptr};
  StyleSheetEditDelegate* mStyleSheetEditDelegate{nullptr};
};

#endif // CONFIGSTABLEVIEW_H
