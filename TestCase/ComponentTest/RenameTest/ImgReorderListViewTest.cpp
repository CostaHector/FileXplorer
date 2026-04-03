#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ImgReorderListView.h"
#include "EndToExposePrivateMember.h"
#include "FileTool.h"
#include "MemoryKey.h"

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

class ImgReorderListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() { GlobalMockObject::reset(); }
  void cleanupTestCase() { GlobalMockObject::verify(); }

  void default_ok() {
    Configuration().clear();
    ImgReorderListView reorderList;
    reorderList.InitListView();
    QVERIFY(reorderList.mImgReorderListModel != nullptr);
    QVERIFY(reorderList.mBatchShiftRight100 != nullptr);
    QVERIFY(reorderList.mBatchShiftLeft100 != nullptr);
    QVERIFY(reorderList.mNormalizeKeepRelativeOrder != nullptr);
    QVERIFY(reorderList.mOpenInSystemApplication != nullptr);
    const QList<QAction*> actionsInMenu = reorderList.m_menu->actions();
    QVERIFY(actionsInMenu.contains(reorderList.mBatchShiftRight100));
    QVERIFY(actionsInMenu.contains(reorderList.mBatchShiftLeft100));
    QVERIFY(actionsInMenu.contains(reorderList.mNormalizeKeepRelativeOrder));
    QVERIFY(actionsInMenu.contains(reorderList.mOpenInSystemApplication));

    const int N = reorderList.mImgReorderListModel->rowCount();
    QCOMPARE(N, 0);
    QCOMPARE(reorderList.selectionModel()->hasSelection(), false);
    QCOMPARE(reorderList.onBatchShiftSelectedRowsByStep(), false);
    QCOMPARE(reorderList.onNormalizeKeepRelativeOrder(), false);
    QCOMPARE(reorderList.currentIndex().isValid(), false);
    QCOMPARE(reorderList.onOpenCurrentIndexInSystemApplication(), false);

    // properties ok
    QCOMPARE(reorderList.isWrapping(), true);
    QCOMPARE(reorderList.flow(), QListView::Flow::LeftToRight);
    QCOMPARE(reorderList.uniformItemSizes(), false);
    QCOMPARE(reorderList.viewMode(), QListView::ViewMode::IconMode);
    QCOMPARE(reorderList.selectionMode(), QAbstractItemView::SelectionMode::ExtendedSelection);
    QCOMPARE(reorderList.editTriggers().testFlag(QAbstractItemView::EditTrigger::EditKeyPressed), true);

    QCOMPARE(reorderList.showDropIndicator(), true);
    QCOMPARE(reorderList.dragEnabled(), true);
    QCOMPARE(reorderList.dragDropMode(), QAbstractItemView::DragDropMode::DragDrop);

    // will not crash
    reorderList.dropEvent(nullptr);

    QMimeData mimeDataNoMimeType;

    QDropEvent noMimeTypeDropEvent{reorderList.geometry().center(), Qt::DropAction::MoveAction, &mimeDataNoMimeType, Qt::MouseButton::LeftButton,
                                   Qt::KeyboardModifier::NoModifier};
    reorderList.dropEvent(&noMimeTypeDropEvent);
    QCOMPARE(noMimeTypeDropEvent.isAccepted(), false);

    QCOMPARE(reorderList.calculateInsertionRow(QPoint{0, 0}), N);
    QCOMPARE(reorderList.calculateInsertionRow(QPoint{99, 99}), N);

    QCOMPARE(reorderList.setImagesToReorder(QStringList{}, QString{}, 0, "invalid pattern. not contain format percentage1"), false);
  }

  void basic_function_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg", "/Cristiano Ronaldo.txt", "/Robert Lewandowski.png"};
    const QString baseName{"Kaka"};
    const int startNo{0};
    const QString namePattern{" %1"};

    ImgReorderListView reorderList;
    reorderList.InitListView();
    QCOMPARE(reorderList.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);
    const int N = reorderList.mImgReorderListModel->rowCount();
    QCOMPARE(N, 3);

    {
      // 释放点 合法索引0
      QByteArray encodedLast2RowsData;
      QDataStream stream(&encodedLast2RowsData, QIODevice::WriteOnly);
      stream << (QList<int>{1, 2});

      QMimeData mimeData;
      mimeData.setData(ImgReorderListModel::MIME_TYPE, encodedLast2RowsData);
      // {1,2} moved to front 0
      QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));
      QDropEvent validDropEvent{QPoint{0, 0}, Qt::DropAction::MoveAction, &mimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};
      reorderList.dropEvent(&validDropEvent);
      QCOMPARE(validDropEvent.isAccepted(), true);
      QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 1", "Kaka 2", "Kaka 0"}));
    }

    // recover
    QCOMPARE(reorderList.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);
    {
      // 释放点 无合法索引
      QByteArray encodedFirst2RowsData;
      QDataStream stream(&encodedFirst2RowsData, QIODevice::WriteOnly);
      stream << (QList<int>{0, 1});

      QMimeData mimeData;
      mimeData.setData(ImgReorderListModel::MIME_TYPE, encodedFirst2RowsData);
      // {0, 1} moved to after the end
      QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));
      QDropEvent validDropEvent{QPoint{-1, -1}, Qt::DropAction::MoveAction, &mimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};
      reorderList.dropEvent(&validDropEvent);
      QCOMPARE(validDropEvent.isAccepted(), true);
      QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 2", "Kaka 0", "Kaka 1"}));
    }

    // recover
    QCOMPARE(reorderList.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);
    {
      // 无需 移动, 事件未接受
      QByteArray encodedAll3RowsData;
      QDataStream stream(&encodedAll3RowsData, QIODevice::WriteOnly);
      stream << (QList<int>{0, 1, 2});

      QMimeData mimeData;
      mimeData.setData(ImgReorderListModel::MIME_TYPE, encodedAll3RowsData);
      // {0, 1} moved to after the end
      QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));
      QDropEvent validDropEvent{QPoint{-1, -1}, Qt::DropAction::MoveAction, &mimeData, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::NoModifier};
      reorderList.dropEvent(&validDropEvent);
      QCOMPARE(validDropEvent.isAccepted(), false);
      QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));
    }
  }

  void subscribe_event_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg", "/Cristiano Ronaldo.txt", "/Robert Lewandowski.png"};
    const QString baseName{"Kaka"};
    const int startNo{0};
    const QString namePattern{" %1"};

    ImgReorderListView reorderList;
    reorderList.InitListView();
    QCOMPARE(reorderList.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));

    reorderList.clearSelection();
    QCOMPARE(reorderList.onBatchShiftSelectedRowsByStep(), false);

    reorderList.selectAll();
    QCOMPARE(reorderList.onBatchShiftSelectedRowsByStep(10), true);
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 10", "Kaka 11", "Kaka 12"}));

    reorderList.clearSelection();
    QCOMPARE(reorderList.onNormalizeKeepRelativeOrder(), true);
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));

    reorderList.selectAll();
    reorderList.mBatchShiftRight100->trigger();
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 100", "Kaka 101", "Kaka 102"}));

    reorderList.selectAll();
    reorderList.mBatchShiftLeft100->trigger();
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));

    reorderList.selectAll();
    QCOMPARE(reorderList.onBatchShiftSelectedRowsByStep(-10), true);
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka -10", "Kaka -9", "Kaka -8"}));  // sort by number not string, so -10<-9<-8

    reorderList.clearSelection();
    reorderList.mNormalizeKeepRelativeOrder->trigger();
    QCOMPARE(reorderList.getOrderedNames(), (QStringList{"Kaka 0", "Kaka 1", "Kaka 2"}));
  }

  void double_clicked_to_open_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg"};
    const QString baseName{"Kaka"};
    const int startNo{0};
    const QString namePattern{" %1"};

    ImgReorderListView reorderList;
    reorderList.InitListView();
    QCOMPARE(reorderList.setImagesToReorder(filesMixedWithImages, baseName, startNo, namePattern), true);

    MOCKER(FileTool::OpenLocalFileUsingDesktopService).expects(exactly(2)).with(eq(QString{"/Ricardo Leite.jpg"})).will(returnValue(false));

    const QModelIndex kakaIndex{reorderList.mImgReorderListModel->index(0)};
    // invalid index
    reorderList.setCurrentIndex({});
    QCOMPARE(reorderList.onOpenCurrentIndexInSystemApplication(), false);

    // valid index but file not exist
    reorderList.setCurrentIndex(kakaIndex);
    QCOMPARE(reorderList.onOpenCurrentIndexInSystemApplication(), false);

    emit reorderList.doubleClicked(kakaIndex);
  }
};

#include "ImgReorderListViewTest.moc"
REGISTER_TEST(ImgReorderListViewTest, false)
