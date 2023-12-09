#ifndef QUICKWHERECLAUSE_H
#define QUICKWHERECLAUSE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include "PublicVariable.h"

class QuickWhereClause : public QDialog {
 public:
  explicit QuickWhereClause(QWidget* parent = nullptr);

  QSize sizeHint() const override { return QSize(600, 200); }
  virtual void accept() override;

  void onClauseSave();
  void onClauseChanged();

  static QString PlainLogicSentence2FuzzySqlWhere(const QString& tokens,
                                                  const QString& keyName = DB_HEADER_KEY::Name,
                                                  const bool autoCompleteAka = false,
                                                  const QString& binaryCondition = "%1 like \"%%2%\"");
  static QString PlainRelation2SqlWhere(QString plainText);

  QString GetWhereString() const { return m_whereLineEdit->text(); }

 private:

  QLineEdit* m_name;
  QLineEdit* m_perf;
  QLineEdit* m_size;
  QLineEdit* m_type;
  QLineEdit* m_dateModified;
  QLineEdit* m_tags;
  QLineEdit* m_rate;
  QLineEdit* m_forSearch;
  QStringList m_whereAndClause;
  QLineEdit* m_whereLineEdit;
  QDialogButtonBox* dbb;

  static QHash<QString, QString> akaPerf;
  static int UpdateAKAHash(const bool isForce = false);
};

#endif  // QUICKWHERECLAUSE_H
