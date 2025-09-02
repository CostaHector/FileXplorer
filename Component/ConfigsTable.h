#ifndef CONFIGSTABLE_H
#define CONFIGSTABLE_H

#include <QDialog>
#include <QLabel>
#include "ConfigsModel.h"
#include "CustomTableView.h"

class ConfigsTable : public QDialog {
 public:
  explicit ConfigsTable(QWidget* parent = nullptr);
  void ReadSettings();

  void showEvent(QShowEvent* event) override;
  virtual void hideEvent(QHideEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  void RefreshWindowIcon();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  void onEditPreferenceSetting() const;

 private:
  QLabel* m_failItemCnt{nullptr};
  ConfigsModel* m_alertModel{nullptr};
  CustomTableView* m_alertsTable{nullptr};
};

#endif  // CONFIGSTABLE_H
