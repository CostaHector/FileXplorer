#ifndef COLUMNVISIBILITYDIALOG_H
#define COLUMNVISIBILITYDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QToolButton>

class ColumnVisibilityDialog : public QDialog {
public:
  explicit ColumnVisibilityDialog(const QStringList& headers,
                                  const QString& initSwitches,
                                  const QString& name = "",
                                  QWidget* parent = nullptr);
  void setAllCheckboxes(bool checked);
  void toggleAllCheckboxes();
  void revertCheckboxes(const QString& initSwitches);
  QString getSwitches() const;
  void showEvent(QShowEvent* event) override;

private:
  QToolButton* mSelectToolButton{nullptr};
  QFormLayout *m_layout{nullptr};
  QDialogButtonBox* buttons{nullptr};
  QList<QCheckBox*> m_checkboxes;
};

#endif // COLUMNVISIBILITYDIALOG_H
