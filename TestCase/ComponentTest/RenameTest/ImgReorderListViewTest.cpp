#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "BeginToExposePrivateMember.h"
#include "ImgReorderListView.h"
#include "EndToExposePrivateMember.h"

class ImgReorderListViewTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void default_ok() {
    ImgReorderListView reorderList;
    QVERIFY(reorderList.mImgReorderListModel != nullptr);
    const int N = reorderList.mImgReorderListModel->rowCount();
    QCOMPARE(N, 0);

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

    QCOMPARE(reorderList.setImagesToReorder(QStringList{}, QString{}, 0, "invalid not contain percentage1 format"), false);
  }

  void basic_function_ok() {
    const QStringList filesMixedWithImages{"/Ricardo Leite.jpg", "/Cristiano Ronaldo.txt", "/Robert Lewandowski.png"};
    const QString baseName{"Kaka"};
    const int startNo{0};
    const QString namePattern{" %1"};

    ImgReorderListView reorderList;
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
};

#include "ImgReorderListViewTest.moc"
REGISTER_TEST(ImgReorderListViewTest, false)
