#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "Logger.h"
#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "PathUndoRedoer.h"
#include "EndToExposePrivateMember.h"


class PathUndoRedoerTest : public PlainTestSuite {
  Q_OBJECT
 public:
 private slots:
  void test_undo_only() {
    PathUndoRedoer pUR;
    QCOMPARE(pUR.undoPathAvailable(), false);
    pUR("1");
    QCOMPARE(pUR.undoPathAvailable(), false);
    QCOMPARE(pUR.undoStack.size(), 1); // "1", "2"
    QCOMPARE(pUR.undoStack.top(), "1");

    pUR("2");
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 2); // "1", "2"
    QCOMPARE(pUR.undoStack.top(), "2");
    QCOMPARE(pUR.redoPathAvailable(), false);
    QCOMPARE(pUR.redoStack.isEmpty(), true);

    QCOMPARE(pUR.onRedoPath(), ""); // cannot redo, try redo, nothing happend
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 2); // "1", "2"
    QCOMPARE(pUR.undoStack.top(), "2");
    QCOMPARE(pUR.redoPathAvailable(), false);
    QCOMPARE(pUR.redoStack.isEmpty(), true);

    QCOMPARE(pUR.onUndoPath(), "1");
    QCOMPARE(pUR.undoPathAvailable(), false);
    QCOMPARE(pUR.undoStack.size(), 1); // "1"
    QCOMPARE(pUR.undoStack.top(), "1");
    QCOMPARE(pUR.redoPathAvailable(), true);
    QCOMPARE(pUR.redoStack.size(), 1); // "2"
    QCOMPARE(pUR.redoStack.top(), "2");

    pUR.onRedoPath();
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 2); // "1", "2"
    QCOMPARE(pUR.undoStack.top(), "2");
    QCOMPARE(pUR.redoPathAvailable(), false);
    QCOMPARE(pUR.redoStack.isEmpty(), true); // ""

    pUR("3");
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 3); // "1", "2", "3"
    QCOMPARE(pUR.undoStack.top(), "3");
    QCOMPARE(pUR.redoPathAvailable(), false);
    QCOMPARE(pUR.redoStack.isEmpty(), true); // ""

    pUR("4");
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 4); // "1", "2", "3", "4"
    QCOMPARE(pUR.undoStack.top(), "4");
    QCOMPARE(pUR.redoPathAvailable(), false);
    QCOMPARE(pUR.redoStack.isEmpty(), true); // ""

    QCOMPARE(pUR.onUndoPath(), "3");
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 3); // "1", "2", "3"
    QCOMPARE(pUR.undoStack.top(), "3");
    QCOMPARE(pUR.redoPathAvailable(), true);
    QCOMPARE(pUR.redoStack.size(), 1); // "4"
    QCOMPARE(pUR.redoStack.top(), "4");

    QCOMPARE(pUR.onUndoPath(), "2");
    QCOMPARE(pUR.undoPathAvailable(), true);
    QCOMPARE(pUR.undoStack.size(), 2); // "1", "2"
    QCOMPARE(pUR.undoStack.top(), "2");
    QCOMPARE(pUR.redoPathAvailable(), true);
    QCOMPARE(pUR.redoStack.size(), 2); // "4", "3"
    QCOMPARE(pUR.redoStack.top(), "3");

    QCOMPARE(pUR.onUndoPath(), "1");
    QCOMPARE(pUR.undoPathAvailable(), false);
    QCOMPARE(pUR.undoStack.size(), 1); // "1"
    QCOMPARE(pUR.undoStack.top(), "1");
    QCOMPARE(pUR.redoPathAvailable(), true);
    QCOMPARE(pUR.redoStack.size(), 3); // "4", "3", "2"
    QCOMPARE(pUR.redoStack.top(), "2");

    QCOMPARE(pUR.onUndoPath(), ""); // cannot undo, try undo, nothing happend
    QCOMPARE(pUR.undoPathAvailable(), false);
    QCOMPARE(pUR.undoStack.size(), 1); // "1"
    QCOMPARE(pUR.undoStack.top(), "1");
    QCOMPARE(pUR.redoPathAvailable(), true);
    QCOMPARE(pUR.redoStack.size(), 3); // "4", "3", "2"
    QCOMPARE(pUR.redoStack.top(), "2");
  }
};

#include "PathUndoRedoerTest.moc"
REGISTER_TEST(PathUndoRedoerTest, false)

