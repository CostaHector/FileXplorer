#include "ViewHelper.h"

#include <QFileIconProvider>
#include "Tools/PathTool.h"

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

void View::changeDropAction(QDropEvent* event, QPoint pnt, const QString& name, QWidget* w) {
  if (event->keyboardModifiers().testFlag(Qt::AltModifier)) {
    event->setDropAction(Qt::DropAction::LinkAction);
    if (not name.isEmpty())
      QToolTip::showText(pnt, TOOLTIP_MSG_LINK.arg(name), w);
  } else if (event->keyboardModifiers() & Qt::ControlModifier) {
    event->setDropAction(Qt::DropAction::CopyAction);
    if (not name.isEmpty())
      QToolTip::showText(pnt, TOOLTIP_MSG_CP.arg(name), w);
  } else {
    event->setDropAction(Qt::DropAction::MoveAction);
    if (not name.isEmpty())
      QToolTip::showText(pnt, TOOLTIP_MSG_MV.arg(name), w);
  }
}

void View::dropEventCore(QAbstractItemView* view, QDropEvent* event) {
  auto* _model = dynamic_cast<MyQFileSystemModel*>(view->model());
  if (_model == nullptr) {
    qDebug("_model is nullptr");
    return;
  }
  _model->DragRelease();
  if (PATHTOOL::isRootOrEmpty(_model->rootPath())) {
    qDebug("Ignore. You cannot drag move/drag enter/drop into the path[%s]", _model->rootPath().toStdString().c_str());
    _model->DragRelease();
    event->ignore();
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);

  if (not(_model->flags(ind).testFlag(Qt::ItemIsDropEnabled))) {
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

  if (PATHTOOL::isRootOrEmpty(_model->rootPath())) {
    qDebug("Ignore. You cannot drag move/drag enter/drop into the path[%s]", _model->rootPath().toStdString().c_str());
    event->ignore();
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (not ind.isValid() or view->selectionModel()->selectedIndexes().contains(ind) or ind.column() != MainKey::Name) {
    _model->DragRelease();
    event->ignore();
    return;
  }
  if (not(_model->flags(ind).testFlag(Qt::ItemIsDropEnabled))) {
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
  if (PATHTOOL::isRootOrEmpty(_model->rootPath())) {
    qDebug("Ignore. You cannot drag move/drag enter/drop into the path[%s]", _model->rootPath().toStdString().c_str());
    event->ignore();
    return;
  }
  const QPoint& pnt = event->pos();
  const QModelIndex& ind = view->indexAt(pnt);
  if (not ind.isValid() or view->selectionModel()->selectedIndexes().contains(ind) or ind.column() != MainKey::Name) {
    _model->DragRelease();
    event->ignore();
    return;
  }
  if (not(_model->flags(ind).testFlag(Qt::ItemIsDropEnabled))) {
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
