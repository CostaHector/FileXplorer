#ifndef CONFIGSMGR_H
#define CONFIGSMGR_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include "ConfigsTableView.h"

class ConfigsMgr : public QDialog {
 public:
  explicit ConfigsMgr(QWidget* parent = nullptr);
  void RefreshWindowIcon();

  bool onEditPreferenceSetting() const;

 protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;

 private:
  void subscribe();

  QLabel* m_failItemCnt{nullptr};
  ConfigsTableView* m_alertsTable{nullptr};
  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif  // CONFIGSMGR_H
