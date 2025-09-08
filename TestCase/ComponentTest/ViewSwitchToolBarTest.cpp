#include <QtTest/QtTest>
#include "MyTestSuite.h"
#include "OnScopeExit.h"
#include <QTestEventList>
#include <QToolButton>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "Logger.h"
#include "BeginToExposePrivateMember.h"
#include "ViewActions.h"
#include "ViewSwitchToolBar.h"
#include "EndToExposePrivateMember.h"

class ViewSwitchToolBarTest : public MyTestSuite {
  Q_OBJECT
public:
  ViewSwitchToolBarTest() : MyTestSuite{false} {
    LOG_D("ViewSwitchToolBarTest object created\n");
  }
  ~ViewSwitchToolBarTest() {
    if (mTb != nullptr)
      delete mTb;
    mTb = nullptr;
  }

private slots:
  // constuctor, initTestCase, {{init, test_XXX,cleanup}_i}, cleanupTestCase, destructor
  void initTestCase() {
    QToolBar* baseTb = g_viewActions().GetViewTB();
    QVERIFY(baseTb != nullptr);

    mTb = dynamic_cast<ViewSwitchToolBar*>(baseTb);
    QVERIFY(mTb != nullptr);

    // Purpose: Click on QToolbar will emit 2 signal at 1 time
    // connect(mTb, &QToolBar::actionTriggered, this, [](QAction* pAct){ // service process;})
    // connect(actGrp, &QActionGroup::triggered, this, ViewSwitchToolBar::onViewActionInActionGroupTriggered);
    connect(mTb, &QToolBar::actionTriggered, this, [this](QAction* /*pAct*/){
      ++serviceLogicFunctionCallTime; // animate service process
    });
  }

  void cleanupTestCase() {
    if (mTb != nullptr) {
      delete mTb;
    }
    mTb = nullptr;
  }

  void init() {
    serviceLogicFunctionCallTime = 0;
    QVERIFY(mTb != nullptr);
    mTb->mViewRD = ViewTypeFormerLadder{};
  }

  void test_action_clicked_will_emit_2_signals_at_one_time_normal() {
    // Precondition: already in DEFAULT_VIEW_TYPE. there is 1 element in undoStack.
    QCOMPARE(mTb->mViewRD.undoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);

    auto& viewInst = g_viewActions();
    QToolButton* specificButton = qobject_cast<QToolButton*>(mTb->widgetForAction(viewInst._LIST_VIEW)); // 以_LIST_VIEW为例
    QVERIFY(specificButton != nullptr);

    // Test point 1: 用户点击mTb中的action, 预期同时触发 1)业务逻辑 2)mViewRD undo栈压入新元素, redo栈不变
    QSignalSpy spyActionTriggered(mTb, &QToolBar::actionTriggered);
    QSignalSpy spyGroupTriggered(mTb->mViewTypeIntAction.mActGrp, &QActionGroup::triggered);
    QTest::mouseClick(specificButton, Qt::LeftButton);
    QCOMPARE(spyActionTriggered.count(), 1);
    QCOMPARE(spyGroupTriggered.count(), 1);

    QCOMPARE(serviceLogicFunctionCallTime, 1);
    QCOMPARE(mTb->mViewRD.undoStack.size(), 2);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::ViewType::LIST);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);

    // Test point 2: 用户点击_VIEW_BACK_TO, 预期同时触发 1)业务逻辑 2)mViewRD undo栈弹出栈顶元素, redo栈压入刚弹出的元素
    // 1)业务逻辑由 undo栈弹出栈顶元素时 emit actionTriggered(checkedAction); 触发
    viewInst._VIEW_BACK_TO->setChecked(true);
    emit viewInst._VIEW_BACK_TO->triggered(true);
    QCOMPARE(serviceLogicFunctionCallTime, 2);
    QCOMPARE(mTb->mViewRD.undoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::ViewType::TABLE);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.redoStack.top(), ViewTypeTool::ViewType::LIST);

    // Test point 3: 用户点击_VIEW_FORWARD_TO, 预期同时触发 1)业务逻辑 2)mViewRD redo栈弹出栈顶元素, undo栈压入刚弹出的元素
    // 1)业务逻辑由 redo栈弹出栈顶元素时 emit actionTriggered(checkedAction); 触发
    viewInst._VIEW_FORWARD_TO->setChecked(true);
    emit viewInst._VIEW_FORWARD_TO->triggered(true);
    QCOMPARE(serviceLogicFunctionCallTime, 3);
    QCOMPARE(mTb->mViewRD.undoStack.size(), 2);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::ViewType::LIST);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);
  }

  void test_emit_action_directly_will_also_emit_2_signals_at_one_time_normal() {
    // both 2 signals: 1) undo stack+1 and 2) view Switched happen
    auto& viewInst = g_viewActions();
    viewInst._TABLE_VIEW->setChecked(true);

    QSignalSpy spyActionTriggered(mTb, &QToolBar::actionTriggered);
    QSignalSpy spyGroupTriggered(mTb->mViewTypeIntAction.mActGrp, &QActionGroup::triggered);

    viewInst._LIST_VIEW->setChecked(true);
    emit viewInst._LIST_VIEW->triggered(true); // must

    QCOMPARE(spyActionTriggered.count(), 1);
    QCOMPARE(spyGroupTriggered.count(), 1);
  }

  void test_cannot_undo_ok() {
    QCOMPARE(mTb->mViewRD.undoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);

    // Expect: cannot undo at all, skip all service logic and undo/redo stack proces
    auto& viewInst = g_viewActions();
    viewInst._VIEW_BACK_TO->setChecked(true);
    emit viewInst._VIEW_BACK_TO->triggered(true);

    QCOMPARE(serviceLogicFunctionCallTime, 0);
    QCOMPARE(mTb->mViewRD.undoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);
  }

  void test_cannot_redo_ok() {
    QCOMPARE(mTb->mViewRD.undoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);

    // Expect: cannot redo at all, skip all service logic and undo/redo stack proces
    auto& viewInst = g_viewActions();
    viewInst._VIEW_FORWARD_TO->setChecked(true);
    emit viewInst._VIEW_FORWARD_TO->triggered(true);

    QCOMPARE(serviceLogicFunctionCallTime, 0);
    QCOMPARE(mTb->mViewRD.undoStack.size(), 1);
    QCOMPARE(mTb->mViewRD.undoStack.top(), ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(mTb->mViewRD.redoStack.size(), 0);
  }

  void test_action_in_QToolBar_triggered_will_also_emit_actionTriggered() {
    QToolBar* tempTb = new (std::nothrow) QToolBar{mTb};
    QVERIFY(tempTb != nullptr);
    QAction* act = new (std::nothrow) QAction{mTb};
    QVERIFY(act != nullptr);
    act->setCheckable(true);
    tempTb->addAction(act);

    QSignalSpy spyActionTriggered(tempTb, &QToolBar::actionTriggered);
    QSignalSpy spyTriggered(act, &QAction::triggered);

    act->setChecked(true);
    emit act->triggered(true);

    QCOMPARE(spyActionTriggered.count(), 1);
    QCOMPARE(spyTriggered.count(), 1);
  }

private:
  ViewSwitchToolBar* mTb{nullptr};
  int serviceLogicFunctionCallTime{0};
};

#include "ViewSwitchToolBarTest.moc"
ViewSwitchToolBarTest g_ViewSwitchToolBarTest;
