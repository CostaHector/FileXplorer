#ifndef VIEWHELPER_H
#define VIEWHELPER_H

#include <QAbstractItemView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include "FileStructurePolicy.h"

class FileSystemModel;

namespace ViewHelper {
bool onMouseSidekeyBackwardForward(Qt::KeyboardModifiers mods, Qt::MouseButton mousebutton);

bool onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& to, const FileStructurePolicy::FileStuctureModeE mode);

void changeDropAction(QDropEvent* event);

void dragEnterEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragEnterEvent* event);
void dragMoveEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragMoveEvent* event);
void dropEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDropEvent* event);
void dragLeaveEventCore(FileSystemModel* m_fsm, QDragLeaveEvent* event);

void startDragCore(QAbstractItemView* view, Qt::DropActions supportedActions);

bool keyPressEventCore(QKeyEvent* e);
}  // namespace View

#endif  // VIEWHELPER_H
