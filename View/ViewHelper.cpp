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

constexpr int View::START_DRAG_DIST; // QApplication::startDragDistance()
constexpr int View::START_DRAG_DIST_MIN;
constexpr Qt::MouseButtons View::MOUSE_NAVI_BTN;

bool View::onMouseSidekeyBackwardForward(Qt::KeyboardModifiers mods, Qt::MouseButton mousebutton) {
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

void View::UpdateItemViewFontSizeCore(QAbstractItemView* view) {
  if (view == nullptr) {
    LOG_D("UpdateItemViewFontSizeCore view* pointer is nullptr");
    return;
  }
  const auto fontSize = Configuration().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(view->font());
  defaultFont.setPointSize(fontSize);
  view->setFont(defaultFont);
}

bool View::onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& dest) {
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

QPixmap View::PaintDraggedFilesFolders(const QString& firstSelectedAbsPath, const int selectedCnt) {
  static QFileIconProvider iconPro;
  QIcon ico = iconPro.icon(firstSelectedAbsPath);
  constexpr int DRGA_PIXMAP_SIDE_LEN = 128;
  QPixmap pixmap = ico.pixmap(DRGA_PIXMAP_SIDE_LEN, DRGA_PIXMAP_SIDE_LEN);
  if (selectedCnt > 1) {
    QPainter painter(&pixmap);
    static QFont font("arial", 18, QFont::Weight::ExtraBold, true);
    painter.setFont(font);
#ifdef _WIN32
    painter.drawText(QRect(0, 0, DRGA_PIXMAP_SIDE_LEN * 2, DRGA_PIXMAP_SIDE_LEN * 2), Qt::AlignRight | Qt::AlignBottom, QString("x%1").arg(selectedCnt));
#else
    painter.drawText(QRect(0, 0, DRGA_PIXMAP_SIDE_LEN, DRGA_PIXMAP_SIDE_LEN), Qt::AlignRight | Qt::AlignBottom, QString("x%1").arg(selectedCnt));
#endif
    painter.end();
  }
  return pixmap;
}

void View::changeDropAction(QDropEvent* event) {
  if (event->keyboardModifiers().testFlag(Qt::AltModifier)) {
    event->setDropAction(Qt::DropAction::LinkAction);
  } else if (event->keyboardModifiers() & Qt::ControlModifier) {
    event->setDropAction(Qt::DropAction::CopyAction);
  } else {
    event->setDropAction(Qt::DropAction::MoveAction);
  }
}

void View::dragEnterEventCore(QAbstractItemView* view, QDragEnterEvent* event) {
  auto* m_fsm = dynamic_cast<FileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    LOG_D("m_fsm is nullptr");
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (!(m_fsm->canItemsBeDragged(ind) || m_fsm->canItemsDroppedHere(ind))) {
    LOG_D("reject drag/drop not allowed.");
    m_fsm->DragRelease();
    event->ignore();
    return;
  }
  m_fsm->DragHover(ind);
  View::changeDropAction(event);
  event->accept();
}

void View::dragMoveEventCore(QAbstractItemView* view, QDragMoveEvent* event) {
  auto* m_fsm = dynamic_cast<FileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    LOG_D("m_fsm is nullptr");
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (!(m_fsm->canItemsDroppedHere(ind) && !view->selectionModel()->selectedRows().contains(ind))) {
    LOG_D("reject drag and move. not allowed or self drop");
    m_fsm->DragRelease();
    event->ignore();
    return;
  }
  m_fsm->DragHover(ind);
  View::changeDropAction(event);
  event->accept();
}

void View::dropEventCore(QAbstractItemView* view, QDropEvent* event) {
  // In mouse drag and move event, we can get DropAction directly.
  // So no need to get from mimedata.data("Preferred DropEffect").
  auto* m_fsm = dynamic_cast<FileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    LOG_D("m_fsm is nullptr");
    return;
  }
  m_fsm->DragRelease();
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  // allowed dropped and not contains
  if (!(m_fsm->canItemsDroppedHere(ind) && !view->selectionModel()->selectedRows().contains(ind))) {
    LOG_D("reject drop here. not allowed or self drop");
    event->ignore();
    return;
  }
  event->accept();
  // ignore here and False return here to allow further processing
  // otherwise. accept() and True return here
  View::changeDropAction(event);
  onDropMimeData(event->mimeData(), event->dropAction(), ind.isValid() ? m_fsm->filePath(ind) : m_fsm->rootPath());
  LOG_D("DropEvent[%d] finished with %d url(s)", event->dropAction(), event->mimeData()->urls().size());
}

void View::dragLeaveEventCore(QAbstractItemView* view, QDragLeaveEvent* event) {
  auto* m_fsm = dynamic_cast<FileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    LOG_D("m_fsm is nullptr");
    return;
  }
  m_fsm->DragRelease();
  event->accept();
}

void View::mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event) {
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
  const QPixmap dragPixmap = View::PaintDraggedFilesFolders(urls[0].toLocalFile(), urls.size());
  drag.setPixmap(dragPixmap);
  drag.exec(Qt::DropAction::LinkAction | Qt::DropAction::CopyAction | Qt::DropAction::MoveAction);
}
