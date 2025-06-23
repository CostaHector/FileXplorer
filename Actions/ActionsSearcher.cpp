#include "ActionsSearcher.h"
#include "public/PublicMacro.h"
#include "ActionsRecorder.h"
#include <QCompleter>

ActionsSearcher::ActionsSearcher(QWidget* parent)  //
    : QWidget{parent}                              //
{
  mActionTb = new (std::nothrow) QToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(mActionTb)
  mActionTb->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonIconOnly);
  mActionsCb = new (std::nothrow) QComboBox{this};
  CHECK_NULLPTR_RETURN_VOID(mActionsCb)
  mActionsCb->setEditable(true);
  mLo = new (std::nothrow) QHBoxLayout;
  CHECK_NULLPTR_RETURN_VOID(mLo)

  mLo->addWidget(mActionTb);
  mLo->addWidget(mActionsCb);
  setLayout(mLo);

  static const auto& inst = ActionsRecorder::GetInst();
  mActionsTextModel = new (std::nothrow) QStringListModel{inst.GetKeys(), mActionsCb};
  CHECK_NULLPTR_RETURN_VOID(mActionsTextModel)
  mActionsCb->setModel(mActionsTextModel);

  QCompleter* pCompleter = new (std::nothrow) QCompleter{mActionsCb};
  CHECK_NULLPTR_RETURN_VOID(pCompleter)
  pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  pCompleter->setCompletionMode(QCompleter::PopupCompletion);
  pCompleter->setFilterMode(Qt::MatchContains);
  pCompleter->setModel(mActionsTextModel);
  mActionsCb->setCompleter(pCompleter);

  layout()->setSpacing(0);
  layout()->setContentsMargins(0, 0, 0, 0);
  subscribe();
}

void ActionsSearcher::subscribe() {
  connect(mActionsCb, &QComboBox::currentTextChanged, this, &ActionsSearcher::onActionSearchTextEdit);
}

void ActionsSearcher::onActionSearchTextEdit(const QString& text) {
  static const auto& inst = ActionsRecorder::GetInst();
  const auto it = inst.mTextToActionMap.constFind(text);
  if (it == inst.mTextToActionMap.constEnd()) {
    qDebug("QAction[%s] not exist in map", qPrintable(text));
    if (mActionTb->defaultAction() != nullptr) {
      mActionTb->setDefaultAction(nullptr);
      mActionTb->setIcon(QIcon());
      mActionTb->setText("");
    }
    return;
  }
  QAction* action = it.value();
  mActionTb->setDefaultAction(action);
}
