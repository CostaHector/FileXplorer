#include <QCoreApplication>
#include <QtTest>

// add necessary includes here

#include "UndoRedo.h"
#include "FileOperation/FileOperatorPub.h"
#include "pub/FileSystemRelatedTest.h"

using namespace FileOperatorType;

class UndoRedoTest : public FileSystemRelatedTest {
  Q_OBJECT

 public:
  UndoRedoTest() : FileSystemRelatedTest{"TestEnv_UndoRedo"} {}
 private slots:
  void init() { m_rootHelper << FileSystemNode{"abcFolder"} << FileSystemNode{"012.txt", false, ""}; }

  void test_RemovePermanently();
  void test_moveToTrash();
  void test_Rename();
};

void UndoRedoTest::test_RemovePermanently() {
  QVERIFY2(QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should exist");

  UndoRedo ur;
  QVERIFY(not ur.undoAvailable());
  QVERIFY(not ur.redoAvailable());

  BATCH_COMMAND_LIST_TYPE rmfileCommand{ACMD{RMFILE, {ROOT_DIR, "012.txt"}}};
  QVERIFY2(ur.Do(rmfileCommand), "rmfile \"012.txt\" should succeed");

  QVERIFY2(not QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should never exist");

  QVERIFY2(ur.undoAvailable(), "should undoAvailable");
  QVERIFY2(ur.Undo().first, "should succeed and file should not exist");

  QVERIFY2(not QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should can NOT restored");
}

void UndoRedoTest::test_moveToTrash() {
  QVERIFY2(QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should exist");

  UndoRedo ur;
  QVERIFY(not ur.undoAvailable());
  QVERIFY(not ur.redoAvailable());

  BATCH_COMMAND_LIST_TYPE moveToTrashCommand{ACMD{MOVETOTRASH, {ROOT_DIR, "012.txt"}}};
  QVERIFY2(ur.Do(moveToTrashCommand), "moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

  QVERIFY2(not QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should in trashbin not here");

  QVERIFY2(ur.undoAvailable(), "should undoAvailable");
  QVERIFY2(ur.Undo().first, "undo moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

  QVERIFY2(QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should be restored");
}

void UndoRedoTest::test_Rename() {
  QVERIFY2(QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should exist");
  QVERIFY2(not QDir(ROOT_DIR).exists("210.txt"), "file 210.txt should not exist");

  UndoRedo ur;

  // rename "012.txt" -> "210.txt"
  BATCH_COMMAND_LIST_TYPE renameCommands1{ACMD{RENAME, {ROOT_DIR, "012.txt", ROOT_DIR, "210.txt"}}};
  QVERIFY2(ur.Do(renameCommands1), "rename \"012.txt\" -> \"210.txt\" should succeed");
  QVERIFY2(not QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should not exist");
  QVERIFY2(QDir(ROOT_DIR).exists("210.txt"), "file 210.txt should exist");

  QVERIFY(ur.undoAvailable());
  QVERIFY(not ur.redoAvailable());

  // rename "012.txt" -> "210.txt" -> "999.txt"
  BATCH_COMMAND_LIST_TYPE renameCommands2{ACMD{RENAME, {ROOT_DIR, "210.txt", ROOT_DIR, "999.txt"}}};
  QVERIFY2(ur.Do(renameCommands2), "rename \"210.txt\" -> \"999.txt\" should succeed");
  QVERIFY2(not QDir(ROOT_DIR).exists("210.txt"), "file 210.txt should not exist");
  QVERIFY2(QDir(ROOT_DIR).exists("999.txt"), "file 999.txt should exist");
  QVERIFY(ur.undoAvailable());
  QVERIFY(not ur.redoAvailable());

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

  QVERIFY2(QDir(ROOT_DIR).exists("012.txt"), "file 012.txt should exist");
  QVERIFY2(not QDir(ROOT_DIR).exists("210.txt"), "file 210.txt should not exist");
}

//QTEST_MAIN(UndoRedoTest)
#include "UndoRedoTest.moc"
