#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

#include <QFileInfo>
#include "UndoRedo.h"
#include "PublicTool.h"

class UndoRedoTest : public QObject
{
    Q_OBJECT

public:

private slots:
    void initTestCase();
    void cleanupTestCase();

    void init();
    void cleanup();

    void test_RemovePermanently();
    void test_moveToTrash();
    void test_Rename();

};

const QString TEST_SRC_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_UndoRedo/DONT_CHANGE");
const QString TEST_DIR = QDir(QFileInfo(__FILE__).absolutePath()).absoluteFilePath("test/TestEnv_UndoRedo/COPY_REMOVABLE");

void UndoRedoTest::init(){
    if (QDir(TEST_DIR).exists()){
        QDir(TEST_DIR).removeRecursively();
    }
    auto ret = PublicTool::copyDirectoryFiles(TEST_SRC_DIR, TEST_DIR);
    assert(ret); // should copied ok
}
void UndoRedoTest::cleanup(){
    if (QDir(TEST_DIR).exists()){
        QDir(TEST_DIR).removeRecursively();
    }
}

void UndoRedoTest::test_RemovePermanently(){
    QVERIFY2(QDir(TEST_DIR).exists("012.txt"), "file 012.txt should exist");

    UndoRedo ur;
    QVERIFY(not ur.undoAvailable());
    QVERIFY(not ur.redoAvailable());

    FileOperation::BATCH_COMMAND_LIST_TYPE rmfileCommand{{"rmfile", TEST_DIR, "012.txt"}};
    QVERIFY2(ur.Do(rmfileCommand), "rmfile \"012.txt\" should succeed");

    QVERIFY2(not QDir(TEST_DIR).exists("012.txt"), "file 012.txt should never exist");

    QVERIFY2(ur.undoAvailable(), "should undoAvailable");
    QVERIFY2(ur.Undo().first, "should succeed and file should not exist");

    QVERIFY2(not QDir(TEST_DIR).exists("012.txt"), "file 012.txt should can NOT restored");
}

void UndoRedoTest::test_moveToTrash()
{
    QVERIFY2(QDir(TEST_DIR).exists("012.txt"), "file 012.txt should exist");

    UndoRedo ur;
    QVERIFY(not ur.undoAvailable());
    QVERIFY(not ur.redoAvailable());

    FileOperation::BATCH_COMMAND_LIST_TYPE moveToTrashCommand{{"moveToTrash", TEST_DIR, "012.txt"}};
    QVERIFY2(ur.Do(moveToTrashCommand), "moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

    QVERIFY2(not QDir(TEST_DIR).exists("012.txt"), "file 012.txt should in trashbin not here");

    QVERIFY2(ur.undoAvailable(), "should undoAvailable");
    QVERIFY2(ur.Undo().first, "undo moveToTrash \"012.txt\" -> \"trashbin\" should succeed");

    QVERIFY2(QDir(TEST_DIR).exists("012.txt"), "file 012.txt should be restored");
}

void UndoRedoTest::test_Rename()
{
    QVERIFY2(QDir(TEST_DIR).exists("012.txt"), "file 012.txt should exist");
    QVERIFY2(not QDir(TEST_DIR).exists("210.txt"), "file 210.txt should not exist");

    UndoRedo ur;

    // rename "012.txt" -> "210.txt"
    FileOperation::BATCH_COMMAND_LIST_TYPE renameCommands1{{"rename", TEST_DIR, "012.txt", TEST_DIR, "210.txt"}};
    QVERIFY2(ur.Do(renameCommands1), "rename \"012.txt\" -> \"210.txt\" should succeed");
    QVERIFY2(not QDir(TEST_DIR).exists("012.txt"), "file 012.txt should not exist");
    QVERIFY2(QDir(TEST_DIR).exists("210.txt"), "file 210.txt should exist");

    QVERIFY(ur.undoAvailable());
    QVERIFY(not ur.redoAvailable());

    // rename "012.txt" -> "210.txt" -> "999.txt"
    FileOperation::BATCH_COMMAND_LIST_TYPE renameCommands2{{"rename", TEST_DIR, "210.txt", TEST_DIR, "999.txt"}};
    QVERIFY2(ur.Do(renameCommands2), "rename \"210.txt\" -> \"999.txt\" should succeed");
    QVERIFY2(not QDir(TEST_DIR).exists("210.txt"), "file 210.txt should not exist");
    QVERIFY2(QDir(TEST_DIR).exists("999.txt"), "file 999.txt should exist");
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

    QVERIFY2(QDir(TEST_DIR).exists("012.txt"), "file 012.txt should exist");
    QVERIFY2(not QDir(TEST_DIR).exists("210.txt"), "file 210.txt should not exist");
}

void UndoRedoTest::initTestCase()
{

}

void UndoRedoTest::cleanupTestCase()
{

}


//QTEST_MAIN(UndoRedoTest)

#include "UndoRedoTest.moc"
