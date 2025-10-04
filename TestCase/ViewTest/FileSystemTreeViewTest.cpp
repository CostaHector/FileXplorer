#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FileSystemTreeView.h"
#include "EndToExposePrivateMember.h"

#include "FileOpActs.h"
#include "ViewHelper.h"

class FileSystemTreeViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void testConstructor() {
    FileSystemModel fsModel;
    FileSystemTreeView treeView(&fsModel);

    QVERIFY(treeView._fsModel != nullptr);
    QVERIFY(treeView.m_fsMenu != nullptr);
    QCOMPARE(treeView.dragDropMode(), QAbstractItemView::DragDrop);
    QVERIFY(treeView.acceptDrops());
    QVERIFY(treeView.dragEnabled());
    QCOMPARE(treeView.selectionMode(), QAbstractItemView::ExtendedSelection);
    QCOMPARE(treeView.editTriggers(), QAbstractItemView::NoEditTriggers);
  }

  void testDragEvents() {
    FileSystemModel fsModel;
    FileSystemTreeView treeView(&fsModel);

    QPoint centerPnt = treeView.geometry().center();

    QMimeData mimeData;
    QDragEnterEvent dragEnterEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QDragMoveEvent dragMoveEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QDragLeaveEvent dragLeaveEvent;
    QDropEvent dropEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    treeView.dragEnterEvent(&dragEnterEvent);
    treeView.dragMoveEvent(&dragMoveEvent);
    treeView.dragLeaveEvent(&dragLeaveEvent);
    treeView.dropEvent(&dropEvent);
  }

  void testKeyEvents() {
    // 创建文件系统模型
    FileSystemModel fsModel;
    FileSystemTreeView treeView(&fsModel);

    // 测试 Delete 键触发
    QSignalSpy moveToTrashSpy(FileOpActs::GetInst().MOVE_TO_TRASHBIN, &QAction::triggered);

    QKeyEvent deleteEvent(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    treeView.keyPressEvent(&deleteEvent);
    QCOMPARE(moveToTrashSpy.count(), 1);

    // 测试其他键不触发
    moveToTrashSpy.clear();
    QKeyEvent enterEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    treeView.keyPressEvent(&enterEvent);
    QCOMPARE(moveToTrashSpy.count(), 0);
  }

  void testMouseEvents() {
    // 创建文件系统模型
    FileSystemModel fsModel;
    FileSystemTreeView treeView(&fsModel);

    // 测试左键按下设置拖拽起始位置
    const QPoint testPos(50, 60);
    QMouseEvent leftPressEvent(QEvent::MouseButtonPress, testPos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    treeView.mousePressEvent(&leftPressEvent);
    QCOMPARE(treeView.mDragStartPosition, testPos);

    // 测试其他按钮不设置拖拽起始位置
    const QPoint originalPos = treeView.mDragStartPosition;
    QMouseEvent rightPressEvent(QEvent::MouseButtonPress, QPoint(100, 120), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    treeView.mousePressEvent(&rightPressEvent);
    QCOMPARE(treeView.mDragStartPosition, originalPos);

    // 测试鼠标按下设置拖拽起始位置
    const QPoint startPos(50, 50);
    treeView.mDragStartPosition = startPos;

    // 测试未达到拖拽阈值 - 不触发拖拽
    QMouseEvent smallMoveEvent(QEvent::MouseMove, startPos + QPoint(View::START_DRAG_DIST_MIN - 1, 0), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    treeView.mouseMoveEvent(&smallMoveEvent);

    // 测试达到拖拽阈值 - 触发拖拽
    QMouseEvent largeMoveEvent(QEvent::MouseMove, startPos + QPoint(View::START_DRAG_DIST, 0), Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    treeView.mouseMoveEvent(&largeMoveEvent);
    QVERIFY(largeMoveEvent.isAccepted());

    // 测试非左键移动不触发拖拽
    QMouseEvent rightMoveEvent(QEvent::MouseMove, startPos + QPoint(View::START_DRAG_DIST, 0), Qt::NoButton, Qt::RightButton, Qt::NoModifier);
    treeView.mouseMoveEvent(&rightMoveEvent);
  }

  void testContextMenu() {
    FileSystemModel fsModel;
    FileSystemTreeView treeView(&fsModel);

    QVERIFY(treeView.m_fsMenu != nullptr);

    QContextMenuEvent contextEvent(QContextMenuEvent::Mouse, QPoint(100, 100));
    treeView.contextMenuEvent(&contextEvent);
  }

  void testViewSettings() {
    FileSystemModel fsModel;
    FileSystemTreeView treeView(&fsModel);

    treeView.InitViewSettings();

    QVERIFY(treeView.alternatingRowColors());
    QVERIFY(treeView.isSortingEnabled());
    QCOMPARE(treeView.selectionBehavior(), QAbstractItemView::SelectRows);

    int originalSize = treeView.font().pointSize();
    treeView.UpdateItemViewFontSize();
  }
};

#include "FileSystemTreeViewTest.moc"
REGISTER_TEST(FileSystemTreeViewTest, false)
