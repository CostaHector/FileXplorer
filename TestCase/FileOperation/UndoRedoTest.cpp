#include <QCoreApplication>
#include <QtTest>

// add necessary includes here

#include "public/UndoRedo.h"
#include "FileOperation/FileOperatorPub.h"
#include "TestCase/pub/MyTestSuite.h"
#include "TestCase/pub/TDir.h"

using namespace FileOperatorType;

class UndoRedoTest : public MyTestSuite {
  Q_OBJECT

 public:
  UndoRedoTest() : MyTestSuite{false} {}

 private slots:
  void test_RemovePermanently() {
    TDir mDir;
    QVERIFY(mDir.IsValid());
    QString mTestPath{mDir.path()};
    QVERIFY(mDir.touch("012.txt", "contents in 012.txt"));
    QVERIFY2(mDir.fileExists("012.txt", false), "file 012.txt should exist");

    UndoRedo ur;
    QVERIFY(!ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    BATCH_COMMAND_LIST_TYPE rmfileCommand{ACMD::GetInstRMFILE(mTestPath, "012.txt")};
    QVERIFY2(ur.Do(rmfileCommand), "rmfile \"012.txt\" should succeed");

    QVERIFY2(!mDir.fileExists("012.txt", false), "file 012.txt should never exist");

    QVERIFY2(ur.undoAvailable(), "should undoAvailable");
    QVERIFY2(ur.Undo(), "should succeed and file should not exist");

    QVERIFY2(!mDir.fileExists("012.txt", false), "file 012.txt should can NOT restored");
  }

  void test_moveToTrash() {
    TDir mDir;
    QVERIFY(mDir.IsValid());
    QString mTestPath{mDir.path()};
    QVERIFY(mDir.touch("012.txt", "contents in 012.txt"));

    UndoRedo ur;
    QVERIFY(!ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    BATCH_COMMAND_LIST_TYPE moveToTrashCommand{ACMD::GetInstMOVETOTRASH(mTestPath, "012.txt")};
    QVERIFY2(ur.Do(moveToTrashCommand), "moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

    QVERIFY2(!mDir.fileExists("012.txt", false), "file 012.txt should in trashbin not here");

    QVERIFY2(ur.undoAvailable(), "should undoAvailable");
    QVERIFY2(ur.Undo(), "undo moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

    QVERIFY2(mDir.fileExists("012.txt", false), "file 012.txt should be restored");
  }

  void test_Rename() {
    TDir mDir;
    QVERIFY(mDir.IsValid());
    QString mTestPath{mDir.path()};
    QVERIFY(mDir.touch("012.txt", "contents in 012.txt"));

    QVERIFY2(mDir.fileExists("012.txt", false), "file 012.txt should exist");
    QVERIFY2(!mDir.fileExists("210.txt", false), "file 210.txt should not exist");

    UndoRedo ur;

    // rename "012.txt" -> "210.txt"
    BATCH_COMMAND_LIST_TYPE renameCommands1{ACMD::GetInstRENAME(mTestPath, "012.txt", "210.txt")};
    QVERIFY2(ur.Do(renameCommands1), "rename \"012.txt\" -> \"210.txt\" should succeed");
    QVERIFY2(!mDir.fileExists("012.txt", false), "file 012.txt should not exist");
    QVERIFY2(mDir.fileExists("210.txt", false), "file 210.txt should exist");

    QVERIFY(ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    // rename "012.txt" -> "210.txt" -> "999.txt"
    BATCH_COMMAND_LIST_TYPE renameCommands2{ACMD::GetInstRENAME(mTestPath, "210.txt", "999.txt")};
    QVERIFY2(ur.Do(renameCommands2), "rename \"210.txt\" -> \"999.txt\" should succeed");
    QVERIFY2(!mDir.fileExists("210.txt", false), "file 210.txt should not exist");
    QVERIFY2(mDir.fileExists("999.txt", false), "file 999.txt should exist");
    QVERIFY(ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    // rename "210.txt" <- "999.txt"
    QVERIFY2(ur.Undo(), "undo rename \"210.txt\" <- \"999.txt\" should succeed");
    QVERIFY(ur.undoAvailable());
    QVERIFY(ur.redoAvailable());

    // rename "210.txt" -> "999.txt"
    QVERIFY2(ur.Redo(), "redo rename \"210.txt\" -> \"999.txt\" should succeed");
    QVERIFY(ur.undoAvailable());
    QVERIFY(not ur.redoAvailable());

    QVERIFY2(ur.Undo(), "undo rename \"210.txt\" <- \"999.txt\" should succeed");
    QVERIFY2(ur.Undo(), "undo rename \"012.txt\" <- \"210.txt\" <- \"999.txt\" should succeed");

    QVERIFY2(mDir.fileExists("012.txt", false), "file 012.txt should exist");
    QVERIFY2(!mDir.fileExists("210.txt", false), "file 210.txt should not exist");
  }
};

#include "UndoRedoTest.moc"
UndoRedoTest g_UndoRedoTest;
