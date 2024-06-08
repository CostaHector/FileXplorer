#include "ViewHelper.h"
#include "Component/ConflictsRecycle.h"
#include "MyQFileSystemModel.h"

#include <QFileIconProvider>
bool View::onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& to) {
  const unsigned URLS_SIZE = data->urls().size();
  if (URLS_SIZE == 0) {
    return true;
  }
  qDebug() << action << QString("%1 item(s) and dropped in [%2]").arg(URLS_SIZE).arg(to);
  QStringList selectedItems;
  selectedItems.reserve(URLS_SIZE);
  for (const QUrl& url : data->urls()) {
    if (url.isLocalFile()) {
      selectedItems.append(url.toLocalFile());
    }
  }
  CCMMode opMode = CCMMode::ERROR_OP;
  switch (action) {
    case Qt::DropAction::CopyAction:
      opMode = CCMMode::COPY_OP;
      break;
    case Qt::DropAction::MoveAction:
      opMode = CCMMode::CUT_OP;
      break;
    case Qt::DropAction::LinkAction:
      opMode = CCMMode::LINK_OP;
      break;  // should not conflict
    default:
      qWarning() << "[Err] Unknown action:" << action;
      return false;
  }
  ConflictsItemHelper conflictIF(selectedItems, to, opMode);
  auto* tfm = new ConflictsRecycle(conflictIF);
  if (to == conflictIF.l and opMode != CCMMode::LINK_OP) {
    return false;
  }
  if (not conflictIF) {  // conflict
    tfm->on_completeMerge();
  } else {
    tfm->exec();
  }
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
    painter.drawText(QRect(0, 0, DRGA_PIXMAP_SIDE_LEN * 2, DRGA_PIXMAP_SIDE_LEN * 2), Qt::AlignRight | Qt::AlignBottom,
                     QString("x%1").arg(selectedCnt));
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
  if (not(m_fsm->canItemsBeDragged(ind) or m_fsm->canItemsDroppedHere(ind))) {
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
  if (not(m_fsm->canItemsDroppedHere(ind) and not view->selectionModel()->selectedIndexes().contains(ind))) {
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
  if (not(m_fsm->canItemsDroppedHere(ind) and not view->selectionModel()->selectedIndexes().contains(ind))) {
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
