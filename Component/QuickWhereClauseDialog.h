#ifndef QUICKWHERECLAUSEDIALOG_H
#define QUICKWHERECLAUSEDIALOG_H

#include "ColumnFilterLineEdit.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QAction>
#include <QMenu>
#include <QToolButton>
#include <QComboBox>
#include <QFormLayout>
#include <QStringListModel>

#ifdef RUNNING_UNIT_TESTS
namespace QuickWhereClauseDialogMock {
inline QStringList& mockWhereHistsList() {
  static QStringList newWhereHistsList;
  return newWhereHistsList;
}
} // namespace QuickWhereClauseDialogMock
#endif

class QuickWhereClauseDialog : public QDialog {
public:
  explicit QuickWhereClauseDialog(QWidget* parent = nullptr);
  ~QuickWhereClauseDialog();
  void Init();

  int WriteUniqueHistoryToQSetting();
  QString GetWhereString() const { return m_whereLineEdit->text(); }

protected:
  void onConditionsChanged();
  QFormLayout* m_Layout{nullptr};
  QLineEdit* m_whereLineEdit{nullptr};

  void AppendColumnFilterLineEdit(ColumnFilterLineEdit* lineEdit) {
    mColumnEditors.push_back(lineEdit);
  }

private:
  virtual void CreatePrivateWidget() {}
  void InitPrivateLayout();

  void subscribe();

  bool onRemoveAHistory();
  int onClearHistory();
  bool onAddAHistory();
  int onEditHistory();

  void ClearLineEditsListText();

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

  QList<ColumnFilterLineEdit*> mColumnEditors;
  static constexpr char WHERE_HIST_SPLIT_CHAR{'\n'};
};

#endif // QUICKWHERECLAUSEDIALOG_H
