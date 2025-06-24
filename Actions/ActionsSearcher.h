#ifndef ACTIONSSEARCHER_H
#define ACTIONSSEARCHER_H

#include <QComboBox>
#include <QStringListModel>

class ActionsSearcher : public QComboBox {
 public:
  ActionsSearcher(QWidget* parent = nullptr);
  void subscribe();
 private:
  void onActionSearchTextEdit(const QString& text);
  QLineEdit* actionKeyLineEdit{nullptr};
  QStringListModel* mActionsTextModel{nullptr};
  QAction* mLastValidAct{nullptr};
};

#endif  // ACTIONSSEARCHER_H
