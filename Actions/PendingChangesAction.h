#ifndef PENDINGCHANGESACTION_H
#define PENDINGCHANGESACTION_H

#include <QWidgetAction>
#include <QPointer>
#include <QList>

class TriStateWidget;
class PendingChangesAction : public QWidgetAction {
  Q_OBJECT
public:
  struct QActionConstructorArgs {
    const QIcon icon;
    const QString text;
    const QString tooltip;
    const QKeySequence ks;
  };
  explicit PendingChangesAction(QActionConstructorArgs submitActArgs, QActionConstructorArgs revertActArgs, QObject* parent = nullptr);
  explicit PendingChangesAction(QAction* pSubmitAct, QAction* pRevert, QObject* parent = nullptr);
  static PendingChangesAction* GetDefault(QObject* parent);

  void setDirty(bool dirty);

  bool isDirty() const { return mDirty; }
  QAction* mSubmitAct{nullptr};
  QAction* mRevertAct{nullptr};

protected:
  QWidget* createWidget(QWidget* parent) override;

  void deleteWidget(QWidget* widget) override;

private:

  void applyState(TriStateWidget* label);

  void refreshAll();

  bool mDirty{false};

  QList<QPointer<TriStateWidget>> mWidgets;
};

#endif // PENDINGCHANGESACTION_H
