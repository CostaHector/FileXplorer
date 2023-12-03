#ifndef QUICKWHERECLAUSE_H
#define QUICKWHERECLAUSE_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>

class QuickWhereClause : public QDialog {
 public:
  explicit QuickWhereClause(QWidget* parent = nullptr);

  QSize sizeHint() const override { return QSize(600, 300); }
  virtual void accept() override;
  void onNameClause();

  QLineEdit* m_whereLineEdit;
  QDialogButtonBox* dbb;

 private:
  QLineEdit* m_name;
  QStringList m_whereClause;
  QStringList m_whereAndClause;
};

#endif  // QUICKWHERECLAUSE_H
