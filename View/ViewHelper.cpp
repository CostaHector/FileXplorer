#include "ViewHelper.h"

#include <QFileIconProvider>

QPixmap View::PaintDraggedFilesFolders(const QString& firstSelectedAbsPath, const int selectedCnt) {
  static QFileIconProvider iconPro;
  QIcon ico = iconPro.icon(firstSelectedAbsPath);
  constexpr int DRGA_PIXMAP_SIDE_LEN = 128;
  QPixmap pixmap = ico.pixmap(DRGA_PIXMAP_SIDE_LEN, DRGA_PIXMAP_SIDE_LEN);
  if (selectedCnt > 1) {
    static QFont font("arial", 18, QFont::Weight::ExtraBold, true);
    QPainter painter(&pixmap);
    painter.setFont(font);
    painter.drawText(QRect(0, 0, DRGA_PIXMAP_SIDE_LEN, DRGA_PIXMAP_SIDE_LEN), Qt::AlignRight | Qt::AlignBottom, QString("x%1").arg(selectedCnt));
    painter.end();
  }
  return pixmap;
}

void View::mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event) {
  if (event->buttons() != Qt::MouseButton::LeftButton) {
    return;
  }
  const QModelIndexList& mixed = View::selectedIndexes(view);
  if (mixed.isEmpty()) {
    event->ignore();
    return;
  }

  auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (_model == nullptr) {
    qDebug("[mouseMove] _model is nullptr");
    return;
  }
  if (_model->rootPath().isEmpty()) {
    qDebug("Ignore. Disk move is disabled[C:/,D:/,E:/]");
    return;
  }

  QList<QUrl> urls;
  urls.reserve(mixed.size());
  for (const auto& ind : mixed) {
    urls.append(QUrl::fromLocalFile(_model->filePath(ind)));
  }

  QMimeData* mime = new QMimeData;
  mime->setUrls(urls);
  QDrag drag(view);
  drag.setMimeData(mime);

  const QPixmap dragPixmap = View::PaintDraggedFilesFolders(urls[0].toLocalFile(), mixed.size());
  drag.setPixmap(dragPixmap);
  qDebug("mouseMoveEventCore urls.size() = %d", urls.size());
  drag.exec(Qt::DropAction::LinkAction | Qt::DropAction::CopyAction | Qt::DropAction::MoveAction);
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

void View::dropEventCore(QAbstractItemView* view, QDropEvent* event) {
  auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (_model == nullptr) {
    qDebug("_model is nullptr");
    return;
  }
  _model->DragRelease();
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  // can dropped and not contains
  if (not(_model->canItemsDroppedHere(ind) and not view->selectionModel()->selectedIndexes().contains(ind))) {
    qDebug("Ignore. Flags(ind)[%d] NOT Meet [%d].", int(_model->flags(ind)), int(Qt::ItemIsDropEnabled));
    event->ignore();
    return;
  }
  View::changeDropAction(event);
}

void View::dragEnterEventCore(QAbstractItemView* view, QDragEnterEvent* event) {
  auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (_model == nullptr) {
    qDebug("_model is nullptr");
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (not(_model->canItemsBeDragged(ind) and not view->selectionModel()->selectedIndexes().contains(ind))) {
    qDebug("Ignore. Flags(ind)[%d] NOT Meet [%d].", int(_model->flags(ind)), int(Qt::ItemIsDropEnabled));
    _model->DragRelease();
    event->ignore();
    return;
  }
  _model->DragHover(ind);
  View::changeDropAction(event);
}

void View::dragMoveEventCore(QAbstractItemView* view, QDragMoveEvent* event) {
  auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (_model == nullptr) {
    qDebug("_model is nullptr");
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (not(_model->canItemsDroppedHere(ind) and not view->selectionModel()->selectedIndexes().contains(ind))) {
    qDebug("Ignore. Flags(ind)[%d] NOT Meet [%d].", int(_model->flags(ind)), int(Qt::ItemIsDropEnabled));
    _model->DragRelease();
    event->ignore();
    return;
  }
  _model->DragHover(ind);
  View::changeDropAction(event);
  event->accept();
  qDebug("\tdragMoveEvent [%d]", int(event->dropAction()));
}

void View::dragLeaveEventCore(QAbstractItemView* view, QDragLeaveEvent* event) {
  auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (_model == nullptr) {
    qDebug("_model is nullptr");
    return;
  }
  _model->DragRelease();
  event->accept();
}
