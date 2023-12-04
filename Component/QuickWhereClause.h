#ifndef QUICKWHERECLAUSE_H
#define QUICKWHERECLAUSE_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QPushButton>
#include "PublicVariable.h"
#include <QToolBar>
#include <QToolButton>

class QuickWhereClause : public QDialog {
 public:
  explicit QuickWhereClause(QWidget* parent = nullptr);

  QSize sizeHint() const override { return QSize(600, 200); }
  virtual void accept() override;

  void onClauseSave();
  void onClauseChanged();
  void onClauseClear();
  void onAkaHint();

  static QString PlainLogicSentence2FuzzySqlWhere(const QString& tokens, const QString& keyName = DB_HEADER_KEY::Name);
  static QString PlainRelation2SqlWhere(const QString& plainText);

  QString GetWhereString() const { return m_whereLineEdit->text(); }

 private:
  QAction* RESET_CONDITION;
  QAction* SAVE_WHERE;
  QToolButton* HIST_WHERE;
  QAction* SHOW_HISTORY;
  QAction* CLEAR_ALL_WHERE;
  QAction* AKA_COMPLETER;
  QAction* APPLY_AND_CLOSE;

  QMenu* m_histMenu;

  QToolBar* m_helperTB;
  QLineEdit* m_name;
  QLineEdit* m_perf;
  QLineEdit* m_size;
  QLineEdit* m_type;
  QLineEdit* m_dateModified;
  QLineEdit* m_tags;
  QLineEdit* m_prePath;
  QStringList m_whereAndClause;
  QLineEdit* m_whereLineEdit;
  QDialogButtonBox* dbb;

  static QHash<QString, QString> akaPerf;
  static QHash<QString, QString> GetAkaPerf();
};

#endif  // QUICKWHERECLAUSE_H
