#include "ViewHelper.h"
#include "MemoryKey.h"
#include "FileSystemModel.h"
#include "NotificatorMacro.h"
#include "ComplexOperation.h"
#include "AddressBarActions.h"
#include "ViewActions.h"
#include <QFileIconProvider>
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

void UpdateItemViewFontSizeCore(QAbstractItemView* view) {
  if (view == nullptr) {
    LOG_D("UpdateItemViewFontSizeCore view* pointer is nullptr");
    return;
  }
  const auto fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(view->font());
  defaultFont.setPointSize(fontSize);
  view->setFont(defaultFont);
}

bool onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& dest) {
  if (!data->hasUrls()) {
    return true;
  }
  const QList<QUrl>& urls = data->urls();
  LOG_W("DropAction[%d] %d item(s) will be dropped in path[%s].", action, urls.size(), qPrintable(dest));
  using namespace ComplexOperation;
  int ret = DoDropAction(action, urls, dest, ComplexOperation::FILE_STRUCTURE_MODE::PRESERVE);
  if (ret < 0) {
    LOG_WARN_NP("[Failed] Drop into partial", dest);
    return false;
  }
  LOG_OK_NP("[Ok] Drop into all", dest);
  return true;
}

QPixmap PaintDraggedFilesFolders(const QString& firstSelectedAbsPath, const int selectedCnt) {
  static QFileIconProvider iconPro;
  QIcon ico = iconPro.icon(firstSelectedAbsPath);
  constexpr int DRGA_PIXMAP_SIDE_LEN = 128;
  QPixmap pixmap = ico.pixmap(DRGA_PIXMAP_SIDE_LEN, DRGA_PIXMAP_SIDE_LEN);
  if (selectedCnt > 1) {
    QPainter painter(&pixmap);
    static QFont font("arial", 18, QFont::Weight::ExtraBold, true);
    painter.setFont(font);
#ifdef _WIN32
    painter.drawText(QRect(0, 0, DRGA_PIXMAP_SIDE_LEN * 2, DRGA_PIXMAP_SIDE_LEN * 2), Qt::AlignRight | Qt::AlignBottom,
                     QString("x%1").arg(selectedCnt));
#else
    painter.drawText(QRect(0, 0, DRGA_PIXMAP_SIDE_LEN, DRGA_PIXMAP_SIDE_LEN), Qt::AlignRight | Qt::AlignBottom, QString("x%1").arg(selectedCnt));
#endif
    painter.end();
  }
  return pixmap;
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

void mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event) {
  event->accept();
  const QModelIndexList rows{view->selectionModel()->selectedRows()};
  if (rows.isEmpty()) {
    return;
  }
  const FileSystemModel* m_fsm = dynamic_cast<FileSystemModel*>(view->model());
  CHECK_NULLPTR_RETURN_VOID(m_fsm)
  QList<QUrl> urls;
  urls.reserve(rows.size());
  for (const QModelIndex& ind : rows) {
    urls.append(QUrl::fromLocalFile(m_fsm->filePath(ind)));
  }
  LOG_D("drags %d rows", urls.size());
  QMimeData* mime = new (std::nothrow) QMimeData;
  mime->setUrls(urls);

  QDrag drag(view);
  drag.setMimeData(mime);
  const QPixmap dragPixmap = PaintDraggedFilesFolders(urls[0].toLocalFile(), urls.size());
  drag.setPixmap(dragPixmap);
  drag.exec(Qt::DropAction::LinkAction | Qt::DropAction::CopyAction | Qt::DropAction::MoveAction);
}

}  // namespace View
