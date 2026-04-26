#ifndef CONFIGSMGR_H
#define CONFIGSMGR_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QToolBar>
#include <QLineEdit>

#include "ConfigsTableView.h"

class ConfigsMgr : public QDialog {
 public:
  explicit ConfigsMgr(QWidget* parent = nullptr);
  void onRecheck();

  bool onEditPreferenceSetting() const;

 protected:
  void showEvent(QShowEvent* event) override;
  void hideEvent(QHideEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

 private:
  void subscribe();
  void RefreshFailedCountLabel();
  void UpdateFailedCountLabel(int newFailedCount);
  void onStartFilter();

  static QWidget *focusWidgetCore(ConfigsMgr* self);

  QToolBar* m_helpToolBar{nullptr};
  QLabel* m_failedCountLabel{nullptr};
  QAction* m_recheckAction{nullptr};

  QLineEdit* m_searchLineEdit{nullptr};
  QAction* m_searchAction{nullptr};

  ConfigsTableView* m_cfgsTable{nullptr};
  QDialogButtonBox* m_dlgBtnBox{nullptr};
};

#endif  // CONFIGSMGR_H
