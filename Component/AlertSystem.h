#ifndef ALERTSYSTEM_H
#define ALERTSYSTEM_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTableWidget>
#include <QWidget>
#include "View/CustomTableView.h"
#include "Model/PreferenceModel.h"

class AlertSystem : public QDialog {
 public:
  explicit AlertSystem(QWidget* parent = nullptr);

  auto sizeHint() const -> QSize override { return QSize(1024, 768); }

  void RefreshWindowIcon();

  bool isRowItemPass(const int row) const;
  bool InitLineColor(const int row);
  bool RefreshLineColor(const int row);
  bool on_cellChanged(const int row, const int column);
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  void onEditPreferenceSetting() const;

 signals:

 private:
  QMap<QString, bool> m_checkItemStatus;
  QLabel* m_failItemCnt;
  PreferenceModel* m_alertModel{new PreferenceModel{this}};
  CustomTableView* m_alertsTable{new CustomTableView{"ALERT_SYSTEM", this}};
  QDialogButtonBox* m_recheckButtonBox;
};

#endif  // ALERTSYSTEM_H
