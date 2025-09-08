#include "ReorderableToolBar.h"
#include "DraggableToolButton.h"
#include "PublicMacro.h"
#include "WidgetReorderHelper.h"
#include <QActionGroup>
#include <QMimeData>
#include <QLayout>
#include <QToolTip>
#include <QWidgetAction>

int GetStartPos(Qt::Orientation orientation, const QPoint& pos) {
  if (orientation == Qt::Orientation::Vertical) {
    return pos.y();
  }
  return pos.x();
}

bool IsCursorPosLessThenWidgetCenter(Qt::Orientation orientation, const int cursorPos, const QWidget& widget) {
  if (orientation == Qt::Orientation::Vertical) {
    return cursorPos <= widget.y() + widget.height() / 2;
  } else {
    return cursorPos <= widget.x() + widget.width() / 2;
  }
}

bool IsSourceValid(QObject* source, QWidget* parent) {
  if (source == nullptr) {
    LOG_W("source is nullptr, skip");
    return false;
  }
  if (source->parent() != parent) {
    LOG_W("parent is not self, skip");
    return false;
  }
  if (!source->isWidgetType()) {
    LOG_W("source is not widget, skip");
    return false;
  }
  return true;
}

ReorderableToolBar::ReorderableToolBar(const QString& title, QWidget* parent)  //
    : QToolBar{title, parent} {                                                //
  setAcceptDrops(true);
  mCollectPathAgs = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(mCollectPathAgs);
  mCollectPathAgs->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
}

ReorderableToolBar::ReorderableToolBar(QWidget* parent)  //
    : QToolBar{parent}                                   //
{
  setAcceptDrops(true);
  mCollectPathAgs = new (std::nothrow) QActionGroup{this};
  CHECK_NULLPTR_RETURN_VOID(mCollectPathAgs);
  mCollectPathAgs->setExclusionPolicy(QActionGroup::ExclusionPolicy::None);
}

void ReorderableToolBar::actionEvent(QActionEvent* event) {
  QToolBar::actionEvent(event);
  if (event->type() != QEvent::ActionAdded) {
    return;
  }
  auto* pAct = event->action();
  CHECK_NULLPTR_RETURN_VOID(pAct);
  mCollectPathAgs->addAction(pAct);
  QWidgetAction* widgetAction = qobject_cast<QWidgetAction*>(pAct);
  if (widgetAction != nullptr) {
    return;
  }
  // event action must be a simple action, not a QToolButton
  DraggableToolButton* btn = new (std::nothrow) DraggableToolButton{this};
  CHECK_NULLPTR_RETURN_VOID(btn)
  btn->setDefaultAction(pAct);
  btn->setToolButtonStyle(toolButtonStyle());

  removeAction(pAct);
  addWidget(btn);
}

void ReorderableToolBar::addDraggableAction(QAction* act) {
  if (act == nullptr) {
    return;
  }
  act->setCheckable(true);
  auto* btn = new (std::nothrow) DraggableToolButton(this);
  CHECK_NULLPTR_RETURN_VOID(btn)
  btn->setDefaultAction(act);
  addWidget(btn);
}

void ReorderableToolBar::dragMoveEvent(QDragMoveEvent* event) {
  QObject* source = event->source();
  if (!IsSourceValid(source, this)) {
    return;
  }
  const QPoint& pos{event->pos()};
  static const auto orien{orientation()};
  const int fromPos{GetStartPos(orien, pos)};
  const QPoint glbPos{mapToGlobal(pos)};
  const QString fromBtnText = event->mimeData()->text();
  bool isAppend = true;
  for (int i = 0; i < layout()->count(); ++i) {
    QWidget* widget = layout()->itemAt(i)->widget();
    if (widget == nullptr) {
      continue;
    }
    auto* tb = dynamic_cast<QToolButton*>(widget);
    if (tb == nullptr) {
      continue;
    }
    if (IsCursorPosLessThenWidgetCenter(orien, fromPos, *widget)) {
      isAppend = false;
      QToolTip::showText(glbPos, QString("Move btn[%1] in front of[%2]?").arg(fromBtnText, tb->text()));
      break;
    }
  }
  if (isAppend) {
    QToolTip::showText(glbPos, QString("Append btn[%1] to last one?").arg(fromBtnText));
  }
  event->acceptProposedAction();
}

void ReorderableToolBar::dragEnterEvent(QDragEnterEvent* event) {
  if (event->mimeData()->hasText()) {
    event->acceptProposedAction();
  }
}

void ReorderableToolBar::dropEvent(QDropEvent* event) {
  QObject* source = event->source();
  if (!IsSourceValid(source, this)) {
    return;
  }
  QLayout* layout = this->layout();
  if (layout == nullptr) {
    LOG_W("layout is None, skip");
    return;
  }
  //  LOG_D("type(source): %s", source->metaObject()->className());
  auto* pSrcWidget = qobject_cast<QWidget*>(source);
  if (pSrcWidget == nullptr) {
    LOG_W("pSrcWidget is nullptr, skip");
    return;
  }
  pSrcWidget->setEnabled(true);
  const int fromIndex{layout->indexOf(pSrcWidget)};
  if (fromIndex == -1) {
    LOG_D("from widget not find at all");
    return;
  }
  const QPoint& pos{event->pos()};
  static const auto orien = orientation();
  const int fromPos{GetStartPos(orien, pos)};
  int destIndex = 0;
  while (destIndex < layout->count()) {
    QWidget* widget = layout->itemAt(destIndex)->widget();
    if (!widget) {
      ++destIndex;
      continue;
    }
    if (IsCursorPosLessThenWidgetCenter(orien, fromPos, *widget)) {
      break;
    }
    ++destIndex;
  }
  event->acceptProposedAction();
  if (!MoveWidgetAtFromIndexInFrontOfDestIndex(fromIndex, destIndex, *this)) {
    return;
  }
  emit widgetMoved(fromIndex, destIndex);
}
