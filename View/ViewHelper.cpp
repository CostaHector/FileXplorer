#include "ViewHelper.h"
#include "MemoryKey.h"
#include "Configuration.h"
#include "FileSystemModel.h"
#include "NotificatorMacro.h"
#include "ComplexOperation.h"
#include "AddressBarActions.h"
#include "ViewActions.h"
#include "ImageTool.h"
#include <QApplication>
#include <QDrag>
#include <QPainter>

namespace View {
bool onMouseSidekeyBackwardForward(Qt::KeyboardModifiers mods, Qt::MouseButton mousebutton) {
  // when return true, event should not be populated to its parent
  if (mods == Qt::KeyboardModifier::NoModifier) {
    static auto& addressInst = g_addressBarActions();
    switch (mousebutton) {
      case Qt::MouseButton::BackButton:
        emit addressInst._BACK_TO->triggered(false);
        return true;
      case Qt::MouseButton::ForwardButton:
        emit addressInst._FORWARD_TO->triggered(false);
        return true;
      default:
        return false;
    }
  } else if (mods == Qt::KeyboardModifier::ControlModifier) {
    static auto& viewInst = g_viewActions();
    switch (mousebutton) {
      case Qt::MouseButton::BackButton:
        emit viewInst._VIEW_BACK_TO->triggered(false);
        return true;
      case Qt::MouseButton::ForwardButton:
        emit viewInst._VIEW_FORWARD_TO->triggered(false);
        return true;
      default:
        return false;
    }
  }
  return false;
}

bool onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& dest) {
  CHECK_NULLPTR_RETURN_FALSE(data);
  if (!data->hasUrls()) {
    return true;
  }
  const QList<QUrl>& urls = data->urls();
  LOG_W("DropAction[%d] %d item(s) will be dropped in path[%s].", action, urls.size(), qPrintable(dest));
  using namespace ComplexOperation;
  int ret = DoDropAction(action, urls, dest, ComplexOperation::FileStuctureModeE::PRESERVE);
  if (ret < 0) {
    LOG_WARN_NP("[Failed] Drop into partial", dest);
    return false;
  }
  LOG_OK_NP("[Ok] Drop into all", dest);
  return true;
}

void changeDropAction(QDropEvent* event) {
  if (event->keyboardModifiers().testFlag(Qt::AltModifier)) {
    event->setDropAction(Qt::DropAction::LinkAction);
  } else if (event->keyboardModifiers() & Qt::ControlModifier) {
    event->setDropAction(Qt::DropAction::CopyAction);
  } else {
    event->setDropAction(Qt::DropAction::MoveAction);
  }
}

bool validateDrop(QAbstractItemView* view,
                  FileSystemModel* m_fsm,
                  const QMimeData* mimeData,
                  Qt::DropAction action,
                  const QPoint& pos,
                  QModelIndex& outIndex) {
  outIndex = view->indexAt(pos);
  if (!m_fsm->canDropMimeData(mimeData, action, -1, -1, outIndex)) {
    LOG_D("[reject] drop at event position index not allowed");
    return false;
  }

  const QModelIndexList& selectedRows = view->selectionModel()->selectedRows();
  if (selectedRows.contains(outIndex)) {
    LOG_D("[reject] self drop not allowed");
    return false;
  }
  return true;
}

void dragEnterEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragEnterEvent* event) {
  const auto* pMimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(pMimeData);
  if (!pMimeData->hasUrls()) {
    LOG_D("[reject] no urls not allowed to drag");
    m_fsm->DragRelease();
    event->ignore();
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  m_fsm->DragHover(ind);
  changeDropAction(event);
  event->accept();
}

void dragMoveEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragMoveEvent* event) {
  const auto* pMimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(pMimeData);

  QModelIndex ind;
  if (!validateDrop(view, m_fsm, pMimeData, event->proposedAction(), event->pos(), ind)) {
    event->ignore();
    return;
  }
  m_fsm->DragHover(ind);
  changeDropAction(event);
  event->accept();
}

void dropEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDropEvent* event) {
  const auto* pMimeData = event->mimeData();
  CHECK_NULLPTR_RETURN_VOID(pMimeData);

  m_fsm->DragRelease();
  QModelIndex ind;
  if (!validateDrop(view, m_fsm, pMimeData, event->proposedAction(), event->pos(), ind)) {
    event->ignore();
    return;
  }
  changeDropAction(event);
  const QString dropToDestPath{ind.isValid() ? m_fsm->filePath(ind) : m_fsm->rootPath()};
  onDropMimeData(event->mimeData(), event->dropAction(), dropToDestPath);
  LOG_D("DropEvent[%d] finished with %d url(s)", event->dropAction(), event->mimeData()->urls().size());
  event->accept();
}

void dragLeaveEventCore(FileSystemModel* m_fsm, QDragLeaveEvent* event) {
  m_fsm->DragRelease();
  event->accept();
}

}  // namespace View
