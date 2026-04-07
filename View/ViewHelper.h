#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QAbstractItemView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>

class FileSystemModel;

namespace View {
bool onMouseSidekeyBackwardForward(Qt::KeyboardModifiers mods, Qt::MouseButton mousebutton);

bool onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& to);

void changeDropAction(QDropEvent* event);

void dragEnterEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragEnterEvent* event);
void dragMoveEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragMoveEvent* event);
void dropEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDropEvent* event);
void dragLeaveEventCore(FileSystemModel* m_fsm, QDragLeaveEvent* event);

void mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event);
}  // namespace View

#endif  // VIEWHELPER_H
