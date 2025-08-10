#ifndef ALERTSYSTEM_H
#define ALERTSYSTEM_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include "PreferenceModel.h"
#include "CustomTableView.h"

class AlertSystem : public QDialog {
 public:
  explicit AlertSystem(QWidget* parent = nullptr);

  void ReadSettings();
  void showEvent(QShowEvent* event) override;
  virtual void hideEvent(QHideEvent* event) override;
  void closeEvent(QCloseEvent* event) override;

  void RefreshWindowIcon();
  bool on_cellDoubleClicked(const QModelIndex& clickedIndex) const;

  void onEditPreferenceSetting() const;
  bool operator()(const QString& /**/) {return true;}

 private:
  QLabel* m_failItemCnt{nullptr};
  PreferenceModel* m_alertModel{nullptr};
  CustomTableView* m_alertsTable{nullptr};
  QDialogButtonBox* m_recheckButtonBox{nullptr};
};

#endif  // ALERTSYSTEM_H
