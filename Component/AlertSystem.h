#ifndef ALERTSYSTEM_H
#define ALERTSYSTEM_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTableWidget>
#include <QWidget>

class AlertSystem : public QDialog {
 public:
  explicit AlertSystem(QWidget* parent = nullptr);

  auto sizeHint() const -> QSize override { return QSize(1024, 768); }

  void RefreshWindowIcon();

  bool isRowItemPass(const int row) const;
  bool InitLineColor(const int row);
  bool RefreshLineColor(const int row);
  bool on_cellChanged(const int row, const int column);

  void onEditPreferenceSetting() const;

 signals:

 private:
  QMap<QString, bool> m_checkItemStatus;
  QLabel* m_failItemCnt;
  QTableWidget* m_alertsTable;
  QDialogButtonBox* m_recheckButtonBox;
};

#endif  // ALERTSYSTEM_H
