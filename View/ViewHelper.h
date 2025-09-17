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

void UpdateItemViewFontSizeCore(QAbstractItemView* view);

bool onDropMimeData(const QMimeData* data, const Qt::DropAction action, const QString& to);

void changeDropAction(QDropEvent* event);

void dragEnterEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragEnterEvent* event);
void dragMoveEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDragMoveEvent* event);
void dropEventCore(QAbstractItemView* view, FileSystemModel* m_fsm, QDropEvent* event);
void dragLeaveEventCore(FileSystemModel* m_fsm, QDragLeaveEvent* event);

void mouseMoveEventCore(QAbstractItemView* view, QMouseEvent* event);

QPixmap PaintDraggedFilesFolders(const QString& firstSelectedAbsPath, const int selectedCnt);
constexpr int START_DRAG_DIST{32};  // QApplication::startDragDistance()
constexpr int START_DRAG_DIST_MIN{10};
constexpr Qt::MouseButtons MOUSE_NAVI_BTN = Qt::BackButton | Qt::ForwardButton;
}  // namespace View

#endif  // VIEWHELPER_H
