#include "ActionsSearcher.h"
#include "public/PublicMacro.h"
#include "ActionsRecorder.h"
#include <QCompleter>
#include <QLayout>
#include <QLineEdit>

ActionsSearcher::ActionsSearcher(QWidget* parent)  //
    : QComboBox{parent}                            //
{
  mLastValidAct = new (std::nothrow) QAction{QIcon(":img/SEARCH"), "Search action by name here", this};
  CHECK_NULLPTR_RETURN_VOID(mLastValidAct)

  actionKeyLineEdit = new (std::nothrow) QLineEdit{this};
  CHECK_NULLPTR_RETURN_VOID(actionKeyLineEdit)
  actionKeyLineEdit->setPlaceholderText(mLastValidAct->text());
  actionKeyLineEdit->addAction(mLastValidAct, QLineEdit::ActionPosition::TrailingPosition);
  setLineEdit(actionKeyLineEdit);

  static const auto& inst = ActionsRecorder::GetInst();
  mActionsTextModel = new (std::nothrow) QStringListModel{inst.GetKeys(), this};
  CHECK_NULLPTR_RETURN_VOID(mActionsTextModel)
  setModel(mActionsTextModel);
  setCurrentIndex(-1);

  QCompleter* pCompleter = completer();
  CHECK_NULLPTR_RETURN_VOID(pCompleter)
  pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  pCompleter->setFilterMode(Qt::MatchContains);
  pCompleter->setModel(mActionsTextModel);
  setCompleter(pCompleter);

  setToolTip(mLastValidAct->text());
  subscribe();
}

void ActionsSearcher::subscribe() {
  connect(this, &QComboBox::currentTextChanged, this, &ActionsSearcher::onActionSearchTextEdit);
}

void ActionsSearcher::onActionSearchTextEdit(const QString& text) {
  static const auto& inst = ActionsRecorder::GetInst();
  if (mLastValidAct != nullptr) {
    actionKeyLineEdit->removeAction(mLastValidAct);
    mLastValidAct = nullptr;
  }
  const auto it = inst.mTextToActionMap.constFind(text);
  if (it == inst.mTextToActionMap.constEnd()) {
    qDebug("QAction[%s] not exist in map", qPrintable(text));
    return;
  }
  mLastValidAct = it.value();
  actionKeyLineEdit->addAction(mLastValidAct, QLineEdit::ActionPosition::TrailingPosition);
}
