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
  void onClauseClear();
  void onAkaHint(const bool autoCompleteAkaSwitch);

  static QString PlainLogicSentence2FuzzySqlWhere(const QString& tokens,
                                                  const QString& keyName = DB_HEADER_KEY::Name,
                                                  const bool autoCompleteAka = false,
                                                  const QString& binaryCondition = "%1 like \"%%2%\"");
  static QString PlainRelation2SqlWhere(QString plainText);

  QString GetWhereString() const { return m_whereLineEdit->text(); }

 private:
  QAction* RESET_CONDITION;
  QAction* SAVE_WHERE;
  QToolButton* HIST_WHERE;
  QAction* SHOW_HISTORY;
  QAction* CLEAR_ALL_WHERE;
  QAction* AUTO_COMPLETE_AKA_SWITCH;
  QAction* APPLY_AND_CLOSE;

  QMenu* m_histMenu;

  QToolBar* m_helperTB;
  QLineEdit* m_name;
  QLineEdit* m_perf;
  QLineEdit* m_size;
  QLineEdit* m_type;
  QLineEdit* m_dateModified;
  QLineEdit* m_tags;
  QLineEdit* m_rate;
  QLineEdit* m_prePath;
  QStringList m_whereAndClause;
  QLineEdit* m_whereLineEdit;
  QDialogButtonBox* dbb;

  static QHash<QString, QString> akaPerf;
  static QHash<QString, QString> GetAkaPerf();
};

#endif  // QUICKWHERECLAUSE_H
