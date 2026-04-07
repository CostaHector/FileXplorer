#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "FileSystemListView.h"
#include "EndToExposePrivateMember.h"

#include "FileOpActs.h"
#include "ViewHelper.h"
#include <QSignalSpy>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class FileSystemListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void init() { GlobalMockObject::reset(); }
  void cleanup() { GlobalMockObject::verify(); }

  void default_constructor_ok() {
    FileSystemModel fsModel;

    FileSystemListView listView(&fsModel);
    listView.InitListView();

    QVERIFY(listView._fsModel != nullptr);
    QCOMPARE(listView.dragDropMode(), QAbstractItemView::DragDrop);
    QVERIFY(listView.acceptDrops());
    QVERIFY(listView.dragEnabled());
  }

  void testDragEvents() {
    FileSystemModel fsModel;
    FileSystemListView listView(&fsModel);
    listView.InitListView();
    QPoint centerPnt = listView.geometry().center();

    QMimeData mimeData;
    QDragEnterEvent dragEnterEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QDragMoveEvent dragMoveEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QDragLeaveEvent dragLeaveEvent;
    QDropEvent dropEvent(centerPnt, Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);

    listView.dragEnterEvent(&dragEnterEvent);
    listView.dragMoveEvent(&dragMoveEvent);
    listView.dragLeaveEvent(&dragLeaveEvent);
    listView.dropEvent(&dropEvent);
  }

  void keyPressEvent_ok() {
    FileSystemModel fsModel;
    FileSystemListView listView(&fsModel);
    listView.InitListView();

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

  void mouseKeyEvents_ok() {
    FileSystemModel fsModel;
    FileSystemListView listView(&fsModel);
    listView.InitListView();

    listView.mousePressEvent(nullptr);

    MOCKER(View::onMouseSidekeyBackwardForward)  //
        .expects(exactly(2))                     //
        .will(returnValue(true))                 //
        .then(returnValue(false));               //
    QMouseEvent randomEvent(QEvent::MouseButtonPress, QPoint(50, 60), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    listView.mousePressEvent(&randomEvent);
    QVERIFY(randomEvent.isAccepted());

    randomEvent.setAccepted(false);
    listView.mousePressEvent(&randomEvent);
    QVERIFY(!randomEvent.isAccepted());
  }
};

#include "FileSystemListViewTest.moc"
REGISTER_TEST(FileSystemListViewTest, false)
