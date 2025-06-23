#ifndef ACTIONSSEARCHER_H
#define ACTIONSSEARCHER_H

#include <QWidget>
#include <QHBoxLayout>
#include <QToolButton>
#include <QComboBox>
#include <QStringListModel>

class ActionsSearcher : public QWidget {
 public:
  ActionsSearcher(QWidget* parent = nullptr);
  void subscribe();
 private:
  void onActionSearchTextEdit(const QString& text);
  QHBoxLayout* mLo{nullptr};
  QToolButton* mActionTb{nullptr};
  QComboBox* mActionsCb{nullptr};
  QStringListModel* mActionsTextModel{nullptr};
};

#endif  // ACTIONSSEARCHER_H
