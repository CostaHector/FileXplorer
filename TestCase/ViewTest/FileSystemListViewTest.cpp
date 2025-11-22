#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FileSystemListView.h"
#include "EndToExposePrivateMember.h"
#include "FileOpActs.h"
#include "ViewHelper.h"

class FileSystemListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void testConstructor() {
    // 创建文件系统模型
    FileSystemModel fsModel;

    // 创建列表视图
    FileSystemListView listView(&fsModel);

    // 验证基本属性
    QVERIFY(listView._fsModel != nullptr);
    QCOMPARE(listView.dragDropMode(), QAbstractItemView::DragDrop);
    QVERIFY(listView.acceptDrops());
    QVERIFY(listView.dragEnabled());
  }
  void testDragEvents() {
    // 创建文件系统模型
    FileSystemModel fsModel;
    FileSystemListView listView(&fsModel);

    QPoint centerPnt = listView.geometry().center();

    // 创建模拟事件
    QMimeData mimeData;
    QDragEnterEvent dragEnterEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QDragMoveEvent dragMoveEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QDragLeaveEvent dragLeaveEvent;
    QDropEvent dropEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    // 验证事件处理不会崩溃
    listView.dragEnterEvent(&dragEnterEvent);
    listView.dragMoveEvent(&dragMoveEvent);
    listView.dragLeaveEvent(&dragLeaveEvent);
    listView.dropEvent(&dropEvent);
  }

  void testKeyEvents() {
    FileSystemModel fsModel;
    FileSystemListView listView(&fsModel);

    // 测试 Delete 键触发
    QSignalSpy moveToTrashSpy(FileOpActs::GetInst().MOVE_TO_TRASHBIN, &QAction::triggered);
    QKeyEvent deleteEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    listView.keyPressEvent(&deleteEvent);
    QCOMPARE(moveToTrashSpy.count(), 1);

    // 测试其他键不触发
    moveToTrashSpy.clear();
    QKeyEvent enterEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    listView.keyPressEvent(&enterEvent);
    QCOMPARE(moveToTrashSpy.count(), 0);
  }

  void testMouseEvents() {
    // 创建文件系统模型
    FileSystemModel fsModel;
    FileSystemListView listView(&fsModel);

    // 测试左键按下设置拖拽起始位置
    const QPoint testPos(50, 60);
    QMouseEvent leftPressEvent(QEvent::MouseButtonPress, testPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    listView.mousePressEvent(&leftPressEvent);
    QCOMPARE(listView.mDragStartPosition, testPos);

    // 测试其他按钮不设置拖拽起始位置
    const QPoint originalPos = listView.mDragStartPosition;
    QMouseEvent rightPressEvent(QEvent::MouseButtonPress, QPoint(100, 120), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    listView.mousePressEvent(&rightPressEvent);
    QCOMPARE(listView.mDragStartPosition, originalPos);

    // 测试鼠标按下设置拖拽起始位置
    const QPoint startPos(50, 50);
    listView.mDragStartPosition = startPos;

    // 测试未达到拖拽阈值 - 不触发拖拽
    QMouseEvent smallMoveEvent(QEvent::MouseMove, startPos + QPoint(View::START_DRAG_DIST_MIN - 1, 0), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    listView.mouseMoveEvent(&smallMoveEvent);
    QVERIFY(!smallMoveEvent.isAccepted());

    // 测试达到拖拽阈值 - 触发拖拽
    QMouseEvent largeMoveEvent(QEvent::MouseMove, startPos + QPoint(View::START_DRAG_DIST, 0), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    listView.mouseMoveEvent(&largeMoveEvent);
    QVERIFY(largeMoveEvent.isAccepted());

    // 测试非左键移动不触发拖拽
    QMouseEvent rightMoveEvent(QEvent::MouseMove, startPos + QPoint(View::START_DRAG_DIST, 0), Qt::NoButton, Qt::RightButton, Qt::NoModifier);
    listView.mouseMoveEvent(&rightMoveEvent);
  }
};
#include "FileSystemListViewTest.moc"
REGISTER_TEST(FileSystemListViewTest, false)
