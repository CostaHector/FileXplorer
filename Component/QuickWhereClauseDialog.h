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

class NoEnterLineEdit : public QLineEdit {
public:
  using QLineEdit::QLineEdit;
  void keyPressEvent(QKeyEvent* event) override;
};

class QuickWhereClauseDialog : public QDialog {
public:
  explicit QuickWhereClauseDialog(QWidget* parent = nullptr);
  ~QuickWhereClauseDialog();
  void Init();

  int WriteUniqueHistoryToQSetting();
  QString GetWhereString() const { return m_whereLineEdit->text(); }

protected:
  virtual void CreatePrivateWidget() {}
  virtual void InitPrivateLayout() {}
  virtual void PrivateSubscribe() {}
  virtual void onConditionsChanged() {}

  QFormLayout* m_Layout{nullptr};
  QComboBox* m_strFilterPatternCB{nullptr};
  QAction* AUTO_COMPLETE_AKA_SWITCH{nullptr};
  QLineEdit* m_whereLineEdit{nullptr};

#ifdef RUNNING_UNIT_TESTS
  inline void ClearLineEditsListText() {
    for (auto* pLE : mLineEditsList) {
      if (pLE != nullptr) {
        pLE->clear();
      }
    }
  }
  QList<QLineEdit*> mLineEditsList;
  QStringList newWhereHistsList;
#endif

private:
  void subscribe();

  void SetStrPatternCaseSensitive(Qt::CaseSensitivity caseSen);
  bool onRemoveAHistory();
  int onClearHistory();
  bool onAddAHistory();
  int onEditHistory();

  QAction* _RMV_WHERE_CLAUSE_FROM_HISTORY{nullptr};
  QAction* _CLEAR_WHERE_CLAUSE_FROM_HISTORY{nullptr};
  QToolButton* mWhereClauseHistoryDecTb{nullptr};
  QMenu* mWhereClauseHistoryDecMenu{nullptr};

  QAction* _ADD_WHERE_CLAUSE_TO_HISTORY{nullptr};
  QAction* _EDIT_WHERE_CLAUSE_HISTORY{nullptr};
  QToolButton* mWhereClauseHistoryIncTb{nullptr};
  QMenu* mWhereClauseHistoryIncMenu{nullptr};

  QComboBox* m_whereHistComboBox{nullptr};
  QDialogButtonBox* mDialogButtonBox{nullptr};

  QStringListModel* mStrListModel{nullptr};

  static constexpr char WHERE_HIST_SPLIT_CHAR{'\n'};
};

#endif // QUICKWHERECLAUSEDIALOG_H
