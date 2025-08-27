#ifndef QUICKWHERECLAUSEDIALOG_H
#define QUICKWHERECLAUSEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QAction>
#include <QMenu>
#include <QToolButton>
#include <QComboBox>
#include <QLineEdit>
#include <QFormLayout>
#include <QStringListModel>

class QuickWhereClauseDialog : public QDialog {
public:
  explicit QuickWhereClauseDialog(QWidget* parent = nullptr);
  ~QuickWhereClauseDialog();
  int WriteUniqueHistoryToQSetting();

  QSize sizeHint() const override { return QSize{600, 200}; }
  virtual void accept() override;

  void onConditionsChanged();

  QString GetWhereString() const { return m_whereLineEdit->text(); }
  void subscribe();
private:
  bool onRemoveAHistory();
  int onClearHistory();
  bool onAddAHistory();
  int onEditHistory();

  QAction* AUTO_COMPLETE_AKA_SWITCH{nullptr};

  QAction* _RMV_WHERE_CLAUSE_FROM_HISTORY{nullptr};
  QAction* _CLEAR_WHERE_CLAUSE_FROM_HISTORY{nullptr};
  QToolButton* mWhereClauseHistoryDecTb{nullptr};
  QMenu* mWhereClauseHistoryDecMenu{nullptr};

  QAction* _ADD_WHERE_CLAUSE_TO_HISTORY{nullptr};
  QAction* _EDIT_WHERE_CLAUSE_HISTORY{nullptr};
  QToolButton* mWhereClauseHistoryIncTb{nullptr};
  QMenu* mWhereClauseHistoryIncMenu{nullptr};

  /* DB_TABLE::MOVIES exclusive */
  QLineEdit* m_Name{nullptr}; // shared
  QLineEdit* m_Size{nullptr};
  QLineEdit* m_Duration{nullptr};
  QLineEdit* m_Studio{nullptr};
  QLineEdit* m_Cast{nullptr};
  QLineEdit* m_Tags{nullptr}; // shared
  /* DB_TABLE::PERFORMERS exclusive */
  QLineEdit* m_Rate{nullptr};
  QLineEdit* m_Ori{nullptr};

#ifdef RUNNING_UNIT_TESTS
  inline void ClearLineEditsListText() {
    for (auto* pLE: mLineEditsList) {
      if (pLE != nullptr) { pLE->clear(); }
    }
  }
  QList<QLineEdit*> mLineEditsList;
  QStringList newWhereHistsList;
#endif

  QComboBox* m_whereComboBox{nullptr};
  QLineEdit* m_whereLineEdit{nullptr};
  QDialogButtonBox* mDialogButtonBox{nullptr};

  QStringListModel* mStrListModel{nullptr};

  QFormLayout* m_Layout{nullptr};
  static constexpr char WHERE_HIST_SPLIT_CHAR{'\n'};
};

#endif  // QUICKWHERECLAUSEDIALOG_H
