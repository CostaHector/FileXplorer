#include <QCoreApplication>
#include <QtTest>

// add necessary includes here

#include "public/UndoRedo.h"
#include "FileOperation/FileOperatorPub.h"
#include "pub/FileSystemTestSuite.h"

using namespace FileOperatorType;

class UndoRedoTest : public FileSystemTestSuite {
  Q_OBJECT

 public:
  UndoRedoTest() : FileSystemTestSuite{"TestEnv_UndoRedo", true, false} {}
 private slots:
  void init() {  //
    m_rootHelper << FileSystemNode{"abcFolder"} << FileSystemNode{"012.txt", false, ""};
  }

  void test_RemovePermanently() {
    QVERIFY2(QDir(mTestPath).exists("012.txt"), "file 012.txt should exist");

    UndoRedo ur;
    QVERIFY(!ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    BATCH_COMMAND_LIST_TYPE rmfileCommand{ACMD::GetInstRMFILE(mTestPath, "012.txt")};
    QVERIFY2(ur.Do(rmfileCommand), "rmfile \"012.txt\" should succeed");

    QVERIFY2(not QDir(mTestPath).exists("012.txt"), "file 012.txt should never exist");

    QVERIFY2(ur.undoAvailable(), "should undoAvailable");
    QVERIFY2(ur.Undo().first, "should succeed and file should not exist");

    QVERIFY2(!QDir(mTestPath).exists("012.txt"), "file 012.txt should can NOT restored");
  }

  void test_moveToTrash() {
    QVERIFY2(QDir(mTestPath).exists("012.txt"), "file 012.txt should exist");

    UndoRedo ur;
    QVERIFY(!ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    BATCH_COMMAND_LIST_TYPE moveToTrashCommand{ACMD::GetInstMOVETOTRASH(mTestPath, "012.txt")};
    QVERIFY2(ur.Do(moveToTrashCommand), "moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

    QVERIFY2(not QDir(mTestPath).exists("012.txt"), "file 012.txt should in trashbin not here");

    QVERIFY2(ur.undoAvailable(), "should undoAvailable");
    QVERIFY2(ur.Undo().first, "undo moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

    QVERIFY2(QDir(mTestPath).exists("012.txt"), "file 012.txt should be restored");
  }

  void test_Rename() {
    QVERIFY2(QDir(mTestPath).exists("012.txt"), "file 012.txt should exist");
    QVERIFY2(!QDir(mTestPath).exists("210.txt"), "file 210.txt should not exist");

    UndoRedo ur;

    // rename "012.txt" -> "210.txt"
    BATCH_COMMAND_LIST_TYPE renameCommands1{ACMD::GetInstRENAME(mTestPath, "012.txt", "210.txt")};
    QVERIFY2(ur.Do(renameCommands1), "rename \"012.txt\" -> \"210.txt\" should succeed");
    QVERIFY2(!QDir(mTestPath).exists("012.txt"), "file 012.txt should not exist");
    QVERIFY2(QDir(mTestPath).exists("210.txt"), "file 210.txt should exist");

    QVERIFY(ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    // rename "012.txt" -> "210.txt" -> "999.txt"
    BATCH_COMMAND_LIST_TYPE renameCommands2{ACMD::GetInstRENAME(mTestPath, "210.txt", "999.txt")};
    QVERIFY2(ur.Do(renameCommands2), "rename \"210.txt\" -> \"999.txt\" should succeed");
    QVERIFY2(!QDir(mTestPath).exists("210.txt"), "file 210.txt should not exist");
    QVERIFY2(QDir(mTestPath).exists("999.txt"), "file 999.txt should exist");
    QVERIFY(ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    // rename "210.txt" <- "999.txt"
    QVERIFY2(ur.Undo().first, "undo rename \"210.txt\" <- \"999.txt\" should succeed");
    QVERIFY(ur.undoAvailable());
    QVERIFY(ur.redoAvailable());

    // rename "210.txt" -> "999.txt"
    QVERIFY2(ur.Redo().first, "redo rename \"210.txt\" -> \"999.txt\" should succeed");
    QVERIFY(ur.undoAvailable());
    QVERIFY(not ur.redoAvailable());

    QVERIFY2(ur.Undo().first, "undo rename \"210.txt\" <- \"999.txt\" should succeed");
    QVERIFY2(ur.Undo().first, "undo rename \"012.txt\" <- \"210.txt\" <- \"999.txt\" should succeed");

    QVERIFY2(QDir(mTestPath).exists("012.txt"), "file 012.txt should exist");
    QVERIFY2(!QDir(mTestPath).exists("210.txt"), "file 210.txt should not exist");
  }
};

#include "UndoRedoTest.moc"
UndoRedoTest g_UndoRedoTest;
