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

  void ReadSettings() {
    if (PreferenceSettings().contains("ALERT_SYSTEM_GEOMETRY")) {
      restoreGeometry(PreferenceSettings().value("ALERT_SYSTEM_GEOMETRY").toByteArray());
    } else {
      setGeometry(DEFAULT_GEOMETRY);
    }
    m_alertsTable->InitTableView();
  }

  void closeEvent(QCloseEvent* event) {
    PreferenceSettings().setValue("ALERT_SYSTEM_GEOMETRY", saveGeometry());
    return QDialog::closeEvent(event);
  }

  void RefreshWindowIcon();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  void onEditPreferenceSetting() const;

 signals:

 private:
  QLabel* m_failItemCnt;
  PreferenceModel* m_alertModel{new PreferenceModel{this}};
  CustomTableView* m_alertsTable{new CustomTableView{"ALERT_SYSTEM", this}};
  QDialogButtonBox* m_recheckButtonBox;
};

#endif  // ALERTSYSTEM_H
