#ifndef CONFIGSTABLE_H
#define CONFIGSTABLE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include "CustomTableView.h"
#include "ConfigsModel.h"

class ConfigsTable : public QDialog {
 public:
  explicit ConfigsTable(QWidget* parent = nullptr);
  void ReadSettings();
  void RefreshWindowIcon();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  bool onEditPreferenceSetting() const;

 protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private:
  QLabel* m_failItemCnt{nullptr};
  ConfigsModel* m_alertModel{nullptr};
  CustomTableView* m_alertsTable{nullptr};
  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif  // CONFIGSTABLE_H
