#ifndef CONFIGSMGR_H
#define CONFIGSMGR_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QToolBar>
#include <QLineEdit>

#include "DialogWithSearchLine.h"
#include "ConfigsTableView.h"

class ConfigsMgr : public DialogWithSearchLine {
 public:
  explicit ConfigsMgr(QWidget* parent = nullptr);
  void onRecheck();

  bool onEditPreferenceSetting() const;

 protected:
  void hideEvent(QHideEvent* event) override;

 private:
  void subscribe();
  void RefreshFailedCountLabel();
  void UpdateFailedCountLabel(int newFailedCount);
  void onStartFilter(const QString& searchText) override;

  QToolBar* m_helpToolBar{nullptr};
  QLabel* m_failedCountLabel{nullptr};
  QAction* m_recheckAction{nullptr};

  ConfigsTableView* m_cfgsTable{nullptr};
  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif  // CONFIGSMGR_H
