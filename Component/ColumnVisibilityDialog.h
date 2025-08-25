#ifndef COLUMNVISIBILITYDIALOG_H
#define COLUMNVISIBILITYDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QToolButton>

class ColumnVisibilityDialog : public QDialog {
public:
  explicit ColumnVisibilityDialog(const QStringList& headers,
                                  const QString& currentSwitches,
                                  QWidget* parent = nullptr);
  QString getSwitches() const;

private:
  QToolButton* mSelectToolButton{nullptr};

  QList<QCheckBox*> m_checkboxes;
  QVBoxLayout *m_layout;
  QDialogButtonBox* buttons;
};

#endif // COLUMNVISIBILITYDIALOG_H
