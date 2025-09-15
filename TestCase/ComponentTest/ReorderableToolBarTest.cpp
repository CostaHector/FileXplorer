#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QToolButton>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "ReorderableToolBar.h"
#include "EndToExposePrivateMember.h"

class ReorderableToolBarTest : public PlainTestSuite {
  Q_OBJECT
public:
  ReorderableToolBarTest() : PlainTestSuite{} {
    LOG_D("ReorderableToolBarTest object created\n");
  }
private slots:
  void toolButton_no_need_moveWidget() {
    QAction act0("0");
    QAction act1("1");

    ReorderableToolBar toolbar("ToolButton in ToolBar no need move at all");
    toolbar.setOrientation(Qt::Horizontal);
    toolbar.addDraggableAction(&act0);
    toolbar.addAction(&act1);
    toolbar.show();
    QCOMPARE(toolbar.mCollectPathAgs->actions().size(), 2);
    QCOMPARE(QTest::qWaitForWindowExposed(&toolbar), true);

    auto noNeedMoveAtAll = [&](int sourceAlsoTargetIndex, int deviation)->void{
      QList<QAction*> actions = toolbar.actions();
      QCOMPARE(actions.size(), 2);
      QWidget* sourceAlsoTargetWidget = toolbar.widgetForAction(actions[sourceAlsoTargetIndex]);
      QVERIFY(sourceAlsoTargetWidget != nullptr);
      toolbar.mSourceObject = sourceAlsoTargetWidget;
      auto widgetGeo = sourceAlsoTargetWidget->geometry();
      QVERIFY(widgetGeo.width() > 10);
      // 2 widget, each one at least > 10 pixel,
      // deviate 10 pixel from one widget center will not over next widget center or last widget center
      QPoint dropPos = widgetGeo.center() + QPoint{deviation, 0};

      QSignalSpy spy(&toolbar, &ReorderableToolBar::widgetMoved);
      QMimeData mimeData;
      mimeData.setText("1");
      QDropEvent dropEvent(dropPos, Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
      toolbar.dropEvent(&dropEvent);
      QCOMPARE(spy.count(), 0);

      // 验证未移动
      actions = toolbar.actions();
      QCOMPARE(actions.size(), 2);

      QWidget* firstWidget = toolbar.widgetForAction(actions[0]);
      auto* firstTbutton = dynamic_cast<QToolButton*>(firstWidget);
      QVERIFY(firstTbutton != nullptr);
      auto* firstDefaultAct = firstTbutton->defaultAction();
      QVERIFY(firstDefaultAct != nullptr);
      QCOMPARE(firstDefaultAct->text(), "0");

      QWidget* secondWidget = toolbar.widgetForAction(actions[1]);
      auto* secondTbutton = dynamic_cast<QToolButton*>(secondWidget);
      QVERIFY(secondTbutton != nullptr);
      auto* secondDefaultAct = secondTbutton->defaultAction();
      QVERIFY(secondDefaultAct != nullptr);
      QCOMPARE(secondDefaultAct->text(), "1");

      toolbar.mSourceObject = nullptr;
    };
    noNeedMoveAtAll(0, 10);
    noNeedMoveAtAll(0, -10);
    noNeedMoveAtAll(1, 10);
    noNeedMoveAtAll(1, -10);
  }

  void toolbutton_DropEvent_ok_EmitSignal_ok() {
    QAction act0("0");
    QAction act1("1");

    ReorderableToolBar toolbar("ToolButton in ToolBar moved left and right");
    toolbar.setOrientation(Qt::Horizontal);
    toolbar.addAction(&act0);
    toolbar.addDraggableAction(&act1);
    toolbar.show();
    QCOMPARE(toolbar.mCollectPathAgs->actions().size(), 2);
    QCOMPARE(QTest::qWaitForWindowExposed(&toolbar), true);

    auto moveInFrontOf = [&](int sourceIndex, int targetIndex) {
      bool bMoveRight = sourceIndex < targetIndex;
      QList<QAction*> actions = toolbar.actions();
      QCOMPARE(actions.size(), 2);
      QWidget* sourceWidget = toolbar.widgetForAction(actions[sourceIndex]);
      QVERIFY(sourceWidget != nullptr);

      QWidget* targetWidget = toolbar.widgetForAction(actions[targetIndex]);
      QVERIFY(targetWidget != nullptr);

      toolbar.mSourceObject = sourceWidget;
      // deviate 2 pixel right than center if move to right, otherwise, deviate -2 pixel
      QPoint dropPos = targetWidget->geometry().center() + QPoint{(bMoveRight ? 2 : -2), 0};

      QSignalSpy spy(&toolbar, &ReorderableToolBar::widgetMoved);
      QMimeData mimeData;
      mimeData.setText("1");
      QDropEvent dropEvent(dropPos, Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
      toolbar.dropEvent(&dropEvent);
      QCOMPARE(spy.count(), 1);
      QList<QVariant> widgetMovedArguments = spy.last();
      QCOMPARE(widgetMovedArguments.size(), 2);
      // right: 0 2; left: 1 0
      QCOMPARE(widgetMovedArguments[0].toInt(), sourceIndex); // fromIndex
      QCOMPARE(widgetMovedArguments[1].toInt(), (targetIndex + (bMoveRight ? 1 : 0))); // destIndex

      // 验证移动后的顺序
      actions = toolbar.actions();
      QCOMPARE(actions.size(), 2);

      QWidget* firstWidget = toolbar.widgetForAction(actions[0]);
      auto* firstTbutton = dynamic_cast<QToolButton*>(firstWidget);
      QVERIFY(firstTbutton != nullptr);
      auto* firstDefaultAct = firstTbutton->defaultAction();
      QVERIFY(firstDefaultAct != nullptr);
      QCOMPARE(firstDefaultAct->text(), QString::number(targetIndex));

      QWidget* secondWidget = toolbar.widgetForAction(actions[1]);
      auto* secondTbutton = dynamic_cast<QToolButton*>(secondWidget);
      QVERIFY(secondTbutton != nullptr);
      auto* secondDefaultAct = secondTbutton->defaultAction();
      QVERIFY(secondDefaultAct != nullptr);
      QCOMPARE(secondDefaultAct->text(), QString::number(sourceIndex));

      toolbar.mSourceObject = nullptr;
    };

    moveInFrontOf(0, 1);
    moveInFrontOf(1, 0);
  }
};

#include "ReorderableToolBarTest.moc"
REGISTER_TEST(ReorderableToolBarTest, true)
