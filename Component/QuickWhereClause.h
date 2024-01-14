#ifndef QUICKWHERECLAUSE_H
#define QUICKWHERECLAUSE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>

class QuickWhereClause : public QDialog {
 public:
  explicit QuickWhereClause(QWidget* parent = nullptr);

  QSize sizeHint() const override { return QSize(600, 200); }
  virtual void accept() override;

  void onClauseSave();
  void onClauseChanged();

  QString GetWhereString() const { return m_whereLineEdit->text(); }

 private:
  QLineEdit* m_forSearch;
  QLineEdit* m_name;
  QLineEdit* m_perf;
  QLineEdit* m_size;
  QLineEdit* m_type;
  QLineEdit* m_dateModified;
  QLineEdit* m_tags;
  QLineEdit* m_rate;
  QStringList m_whereAndClause;
  QLineEdit* m_whereLineEdit;
  QDialogButtonBox* dbb;
};

#endif  // QUICKWHERECLAUSE_H
