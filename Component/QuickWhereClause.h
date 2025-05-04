#ifndef QUICKWHERECLAUSE_H
#define QUICKWHERECLAUSE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFormLayout>

class QuickWhereClause : public QDialog {
 public:
  explicit QuickWhereClause(QWidget* parent = nullptr);

  QSize sizeHint() const override { return QSize(600, 200); }
  virtual void accept() override;

  void onClauseSave();
  void onClauseChanged();

  QString GetWhereString() const { return m_whereLineEdit->text(); }

 private:
  QLineEdit* m_Name{nullptr};
  QLineEdit* m_Size{nullptr};
  QLineEdit* m_Duration{nullptr};
  QLineEdit* m_Studio{nullptr};
  QLineEdit* m_Cast{nullptr};
  QLineEdit* m_Tags{nullptr};
  QLineEdit* m_whereLineEdit{nullptr};
  QDialogButtonBox* dbb{nullptr};

  QFormLayout* lo{nullptr};
  QStringList m_whereAndClause;
};

#endif  // QUICKWHERECLAUSE_H
