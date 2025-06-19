#include "ViewHelper.h"
#include "public/MemoryKey.h"
#include "Model/MyQFileSystemModel.h"
#include "Component/Notificator.h"
#include "FileOperation/ComplexOperation.h"
#include "Actions/AddressBarActions.h"

#include <QFileIconProvider>
#include <QApplication>
#include <QDrag>
#include <QPainter>

constexpr int View::START_DRAG_DIST; // QApplication::startDragDistance()
constexpr int View::START_DRAG_DIST_MIN;

bool View::onMouseSidekeyBackwardForward(Qt::MouseButton mousebutton) {
  switch (mousebutton) {
    case Qt::MouseButton::BackButton:
      g_addressBarActions()._BACK_TO->triggered(false);
      return true;
    case Qt::MouseButton::ForwardButton:
      g_addressBarActions()._FORWARD_TO->triggered(false);
      return true;
    default:
      return false;
  }
}

void View::UpdateItemViewFontSizeCore(QAbstractItemView* view) {
  if (view == nullptr) {
    qDebug("UpdateItemViewFontSizeCore view* pointer is nullptr");
    return;
  }
  const auto fontSize = PreferenceSettings().value(MemoryKey::ITEM_VIEW_FONT_SIZE.name, MemoryKey::ITEM_VIEW_FONT_SIZE.v).toInt();
  QFont defaultFont(view->font());
  defaultFont.setPointSize(fontSize);
  view->setFont(defaultFont);
}

bool View::onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& dest) {
  if (!data->hasUrls()) {
    return true;
  }
  const QList<QUrl>& urls = data->urls();
  qWarning("DropAction[%d] %d item(s) will be dropped in path[%s].", action, urls.size(), qPrintable(dest));
  using namespace ComplexOperation;
  int ret = DoDropAction(action, urls, dest, ComplexOperation::FILE_STRUCTURE_MODE::PRESERVE);
  if (ret < 0) {
    LOG_WARN("Drop into partial failed", dest);
    return false;
  }
  LOG_GOOD("Drop into all succeed", dest);
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
  auto* m_fsm = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    qDebug("m_fsm is nullptr");
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (!(m_fsm->canItemsBeDragged(ind) || m_fsm->canItemsDroppedHere(ind))) {
    qDebug("reject drag/drop not allowed.");
    m_fsm->DragRelease();
    event->ignore();
    return;
  }
  m_fsm->DragHover(ind);
  View::changeDropAction(event);
  event->accept();
}

void View::dragMoveEventCore(QAbstractItemView* view, QDragMoveEvent* event) {
  auto* m_fsm = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    qDebug("m_fsm is nullptr");
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (!(m_fsm->canItemsDroppedHere(ind) && !view->selectionModel()->selectedIndexes().contains(ind))) {
    qDebug("reject drag and move. not allowed or self drop");
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
  auto* m_fsm = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    qDebug("m_fsm is nullptr");
    return;
  }
  m_fsm->DragRelease();
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  // allowed dropped and not contains
  if (!(m_fsm->canItemsDroppedHere(ind) && !view->selectionModel()->selectedIndexes().contains(ind))) {
    qDebug("reject drop here. not allowed or self drop");
    event->ignore();
    return;
  }
  event->accept();
  // ignore here and False return here to allow further processing
  // otherwise. accept() and True return here
  View::changeDropAction(event);
  onDropMimeData(event->mimeData(), event->dropAction(), ind.isValid() ? m_fsm->filePath(ind) : m_fsm->rootPath());
  qDebug() << event->dropAction() << QString("DropEvent Finished with %1 url(s)").arg(event->mimeData()->urls().size());
}

void View::dragLeaveEventCore(QAbstractItemView* view, QDragLeaveEvent* event) {
  auto* m_fsm = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    qDebug("m_fsm is nullptr");
    return;
  }
  m_fsm->DragRelease();
  event->accept();
}

void View::mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event) {
  event->accept();
  const QModelIndexList& rows = View::selectedIndexes(view);
  if (rows.isEmpty()) {
    return;
  }
  auto* m_fsm = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (m_fsm == nullptr) {
    qDebug("[mouseMove] m_fsm is nullptr");
    return;
  }
  event->accept();
  QList<QUrl> urls;
  urls.reserve(rows.size());
  for (const auto& ind : rows) {
    urls.append(QUrl::fromLocalFile(m_fsm->filePath(ind)));
  }
  qDebug("drags %d rows", urls.size());

  QMimeData* mime = new QMimeData;
  mime->setUrls(urls);

  QDrag drag(view);
  drag.setMimeData(mime);
  const QPixmap dragPixmap = View::PaintDraggedFilesFolders(urls[0].toLocalFile(), urls.size());
  drag.setPixmap(dragPixmap);
  drag.exec(Qt::DropAction::LinkAction | Qt::DropAction::CopyAction | Qt::DropAction::MoveAction);
}
