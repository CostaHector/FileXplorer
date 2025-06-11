#ifndef ALERTSYSTEM_H
#define ALERTSYSTEM_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTableWidget>
#include <QWidget>
#include "Model/PreferenceModel.h"
#include "View/CustomTableView.h"

class AlertSystem : public QDialog {
 public:
  explicit AlertSystem(QWidget* parent = nullptr);

  void ReadSettings();
  virtual void hideEvent(QHideEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  void RefreshWindowIcon();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  void onEditPreferenceSetting() const;
  bool operator()(const QString& /**/) {return true;}

 signals:

 private:
  QLabel* m_failItemCnt;
  PreferenceModel* m_alertModel{new PreferenceModel{this}};
  CustomTableView* m_alertsTable{new CustomTableView{"ALERT_SYSTEM", this}};
  QDialogButtonBox* m_recheckButtonBox;
};

#endif  // ALERTSYSTEM_H
