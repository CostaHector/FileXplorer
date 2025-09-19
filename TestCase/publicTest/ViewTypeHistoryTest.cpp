#include <QCoreApplication>
#include <QtTest>
#include "PlainTestSuite.h"
#include "BeginToExposePrivateMember.h"
#include "ViewTypeHistory.h"
#include "EndToExposePrivateMember.h"

class ViewTypeHistoryTest : public PlainTestSuite {
  Q_OBJECT
public:
private slots:
  void testAddNewViewTypes() {
    ViewTypeHistory ladder;
    QVERIFY(!ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());

    // skip same
    bool result = ladder(ViewTypeTool::ViewType::TABLE);
    QVERIFY(!result);

    result = ladder(ViewTypeTool::ViewType::LIST);
    QVERIFY(result);
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());

    result = ladder(ViewTypeTool::ViewType::TREE);
    QVERIFY(result);
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());

    result = ladder(ViewTypeTool::ViewType::TREE);
    QVERIFY(!result);

    result = ladder(ViewTypeTool::ViewType::SEARCH);
    QVERIFY(result);
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());
  }

  void testUndoOperations() {
    ViewTypeHistory ladder;

    ladder(ViewTypeTool::ViewType::LIST);
    ladder(ViewTypeTool::ViewType::TREE);
    ladder(ViewTypeTool::ViewType::SEARCH);
    QVERIFY(ladder.undoViewAvailable());

    ViewTypeTool::ViewType result = ladder.undo();
    QCOMPARE(result, ViewTypeTool::ViewType::TREE);
    QVERIFY(ladder.undoViewAvailable());

    result = ladder.undo();
    QCOMPARE(result, ViewTypeTool::ViewType::LIST);
    QVERIFY(ladder.undoViewAvailable());

    result = ladder.undo();
    QCOMPARE(result, ViewTypeTool::ViewType::TABLE);
    QVERIFY(!ladder.undoViewAvailable()); // stack unchange when operation not available
    auto beforeUndo = ladder.undoStack;
    auto beforeRedo = ladder.redoStack;

    result = ladder.undo();
    QCOMPARE(result, ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(ladder.undoStack, beforeUndo);
    QCOMPARE(ladder.redoStack, beforeRedo);
  }

  void testRedoOperations() {
    ViewTypeHistory ladder;

    ladder(ViewTypeTool::ViewType::LIST);
    ladder(ViewTypeTool::ViewType::TREE);
    ladder(ViewTypeTool::ViewType::SEARCH);
    ladder.undo();
    ladder.undo();

    QVERIFY(ladder.redoViewAvailable());

    ViewTypeTool::ViewType result = ladder.redo();
    QCOMPARE(result, ViewTypeTool::ViewType::TREE);
    QVERIFY(ladder.redoViewAvailable());

    result = ladder.redo();
    QCOMPARE(result, ViewTypeTool::ViewType::SEARCH);
    QVERIFY(!ladder.redoViewAvailable()); // stack unchange when operation not available
    auto beforeUndo = ladder.undoStack;
    auto beforeRedo = ladder.redoStack;

    result = ladder.redo();
    QCOMPARE(result, ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(ladder.undoStack, beforeUndo);
    QCOMPARE(ladder.redoStack, beforeRedo);
  }

  void testAlternatingOperations() {
    ViewTypeHistory ladder;
    QVERIFY(!ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());

    // add LIST
    ladder(ViewTypeTool::ViewType::LIST); // undo[table, list], redo:[]
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());

    // add TREE
    ladder(ViewTypeTool::ViewType::TREE); // undo[table, list, tree], redo:[]
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable());

    // undo->LIST
    ViewTypeTool::ViewType result = ladder.undo(); // undo: [table, list], redo:[tree]
    QCOMPARE(result, ViewTypeTool::ViewType::LIST);
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(ladder.redoViewAvailable());

    // add SEARCH
    ladder(ViewTypeTool::ViewType::SEARCH); // undo: [table, list, search], redo:[tree]
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(ladder.redoViewAvailable());

    // redo -> tree
    result = ladder.redo();
    QCOMPARE(result, ViewTypeTool::ViewType::TREE); // undo: [table, list, search, tree], redo:[]
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(!ladder.redoViewAvailable()); // cannot redo
    auto beforeUndo = ladder.undoStack;
    auto beforeRedo = ladder.redoStack;

    // cannot redo, but force redo again, stack keeps unchange
    result = ladder.redo();
    QCOMPARE(result, ViewTypeTool::DEFAULT_VIEW_TYPE);
    QCOMPARE(ladder.undoStack, beforeUndo);
    QCOMPARE(ladder.redoStack, beforeRedo);

    // undo -> search
    result = ladder.undo();
    QCOMPARE(result, ViewTypeTool::ViewType::SEARCH); // undo: [table, list, search], redo:[tree]
    QVERIFY(ladder.undoViewAvailable());
    QVERIFY(ladder.redoViewAvailable());
    QCOMPARE(ladder.undoStack.size(), 3);
    QCOMPARE(ladder.redoStack.size(), 1);
  }
};

#include "ViewTypeHistoryTest.moc"
REGISTER_TEST(ViewTypeHistoryTest, false)
