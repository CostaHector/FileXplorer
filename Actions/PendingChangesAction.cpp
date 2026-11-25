#include "PendingChangesAction.h"
#include "PublicCheckMacro.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>

class TriStateWidget : public QWidget {
public:
  explicit TriStateWidget(QAction* submitAct, QAction* revertAct, QWidget* parent = nullptr);
  QLabel* label() const { return mStatesLabel; }

private:
  QLabel* mStatesLabel{nullptr};
};

#define LABEL_TEXT_DOT_PENDING "● Pending"
#define LABEL_TEXT_NO_CHANGES  "No Changes"
TriStateWidget::TriStateWidget(QAction *submitAct, QAction *revertAct, QWidget *parent) : QWidget{parent} {
  // 左侧提交和状态
  auto* pSubmitBtn = new QToolButton{this}; // 2*2
  pSubmitBtn->setDefaultAction(submitAct);
  pSubmitBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mStatesLabel = new QLabel{this}; // 1*2
  mStatesLabel->setTextFormat(Qt::RichText);
  // 关键：用两种状态文本的最大宽度固定 label，避免切换时宽度变化
  ensurePolished();                                      // 让样式/字体生效，font() 才准确
  const QFontMetrics fm{mStatesLabel->font()};
  const int dirtyW = fm.horizontalAdvance(QStringLiteral(LABEL_TEXT_DOT_PENDING));
  const int cleanW = fm.horizontalAdvance(QStringLiteral(LABEL_TEXT_NO_CHANGES));
  mStatesLabel->setFixedWidth(qMax(dirtyW, cleanW) + 8); // +8 为左右 padding

  QVBoxLayout* submitAndIndicatorLayout = new QVBoxLayout;
  submitAndIndicatorLayout->setContentsMargins(0, 0, 0, 0);
  submitAndIndicatorLayout->setSpacing(0);
  submitAndIndicatorLayout->addWidget(pSubmitBtn, 2);
  submitAndIndicatorLayout->addWidget(mStatesLabel, 1);

  // 右侧撤销
  auto* pRevertAct = new QToolButton{this}; // 2*1
  pRevertAct->setDefaultAction(revertAct);
  pRevertAct->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  QHBoxLayout* mainLayout = new QHBoxLayout{this};
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);
  mainLayout->addLayout(submitAndIndicatorLayout, 2);
  mainLayout->addWidget(pRevertAct, 1);

  setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
}

QAction* NewQActionFromQActionConstructorArgs(PendingChangesAction::QActionConstructorArgs actArgs, QObject* parent) {
  CHECK_NULLPTR_RETURN_NULLPTR(parent);
  auto* pAct = new QAction{actArgs.icon,  actArgs.text, parent};
  pAct->setShortcut(actArgs.ks);
  pAct->setShortcutVisibleInContextMenu(true);
  pAct->setToolTip(QString{"<b>%1 (%2)</b><br/>%3"}.arg(pAct->text(), pAct->shortcut().toString(), actArgs.tooltip));
  return pAct;
}

PendingChangesAction* PendingChangesAction::GetDefault(QObject* parent) {
  auto* _MODEL_SUBMIT_ALL = new (std::nothrow) QAction{QIcon(":img/MANUAL_SUBMIT"), tr("Submit All"), parent};
  CHECK_NULLPTR_RETURN_NULLPTR(_MODEL_SUBMIT_ALL);
  _MODEL_SUBMIT_ALL->setShortcut(QKeySequence(Qt::ControlModifier | Qt::Key_S));
  _MODEL_SUBMIT_ALL->setShortcutVisibleInContextMenu(true);
  _MODEL_SUBMIT_ALL->setToolTip(QString{"<b>%1 (%2)</b><br/>Submits all pending changes. Model will be repopulated."}.arg(
      _MODEL_SUBMIT_ALL->text(), _MODEL_SUBMIT_ALL->shortcut().toString()));

  auto* _MODEL_REVERT_ALL = new (std::nothrow) QAction{QIcon(":img/REVERT"), tr("Revert all"), parent};
  CHECK_NULLPTR_RETURN_NULLPTR(_MODEL_REVERT_ALL);
  _MODEL_REVERT_ALL->setShortcutVisibleInContextMenu(true);
  _MODEL_REVERT_ALL->setToolTip(QString{"<b>%1 (%2)</b><br/>Warning: Discard all pending edits and restore the last loaded data from the database. "
                                        "Uncommitted inserts, updates and deletions will be lost."}.arg(
                                        _MODEL_REVERT_ALL->text(), _MODEL_REVERT_ALL->shortcut().toString()));

  return new (std::nothrow) PendingChangesAction{_MODEL_SUBMIT_ALL, _MODEL_REVERT_ALL, parent};
}

PendingChangesAction::PendingChangesAction(QAction* pSubmitAct, QAction* pRevert, QObject* parent)
  : QWidgetAction(parent), mSubmitAct {pSubmitAct}, mRevertAct{pRevert} {
  CHECK_NULLPTR_RETURN_VOID(pSubmitAct);
  CHECK_NULLPTR_RETURN_VOID(pRevert);
  CHECK_NULLPTR_RETURN_VOID(pSubmitAct->parent());
  CHECK_NULLPTR_RETURN_VOID(pRevert->parent());
  CHECK_NULLPTR_RETURN_VOID(parent);
  // pSubmitAct和pRevert作为到静态对象类中的成员时， 必须要设定好了parent为静态对象自身
  // 若没有设定好了parent， 把QAction加到QToolBar中时会移交所有权出去， QToolBar析构后， 导致静态对象类中持有悬垂指针
}

PendingChangesAction::PendingChangesAction(QActionConstructorArgs submitActArgs, QActionConstructorArgs revertActArgs, QObject *parent)
  : PendingChangesAction{parent != nullptr ? NewQActionFromQActionConstructorArgs(submitActArgs, parent) : nullptr,
                         parent != nullptr ? NewQActionFromQActionConstructorArgs(revertActArgs, parent) : nullptr,
                         parent} {
}

void PendingChangesAction::setDirty(bool dirty) {
  mDirty = dirty;
  refreshAll();
}

QWidget *PendingChangesAction::createWidget(QWidget *parent) {
  TriStateWidget* pWid = new TriStateWidget(mSubmitAct, mRevertAct, parent);
  mWidgets.append(pWid);   // QPointer，widget 被删后自动变 null
  applyState(pWid);
  return pWid;
}

void PendingChangesAction::deleteWidget(QWidget *widget) {
  mWidgets.removeAll(dynamic_cast<TriStateWidget*>(widget));
  QWidgetAction::deleteWidget(widget);
}

void PendingChangesAction::applyState(TriStateWidget *pWid) {
  auto* label = pWid->label();
  if (mDirty) {
    label->setText(QStringLiteral("<span style='color:#FF0000;'>" LABEL_TEXT_DOT_PENDING "</span>"));
    label->setToolTip(tr("There are uncommitted changes in this table. "
                         "Use Submit All to save them, or Revert All to discard them."));
  } else {
    label->setText(QStringLiteral(LABEL_TEXT_NO_CHANGES));
    label->setToolTip(tr("All changes are saved. There are no pending edits to submit."));
  }
}

void PendingChangesAction::refreshAll() {
  for (TriStateWidget* pWid : mWidgets) {
    if (pWid != nullptr) {
      applyState(pWid);
    }
  }
}
#undef LABEL_TEXT_NO_CHANGES
#undef LABEL_TEXT_DOT_PENDING
