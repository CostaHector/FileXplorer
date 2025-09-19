#include <QCoreApplication>
#include <QtTest>

#include "PlainTestSuite.h"
#include "TDir.h"
#include "BeginToExposePrivateMember.h"
#include "UndoRedo.h"
#include "EndToExposePrivateMember.h"
#include "GlbDataProtect.h"
#include "SyncModifiyFileSystem.h"

using namespace FileOperatorType;

class UndoRedoTest : public PlainTestSuite {
  Q_OBJECT

 public:
  UndoRedoTest() : PlainTestSuite{} {}

 private slots:
  void test_rmFolderForce_cannot_recover() {
    TDir mDir;
    QVERIFY(mDir.IsValid());
    QString mTestPath{mDir.path()};
    QVERIFY(mDir.touch("home/to/file.txt", "contents in file.txt"));

    UndoRedo ur;
    BATCH_COMMAND_LIST_TYPE rmfileCommand{ACMD::GetInstRMFOLDERFORCE(mTestPath, "home")};
    QVERIFY(ur.Do(rmfileCommand));

    QVERIFY(ur.undoAvailable());
    QVERIFY(ur.Undo());

    QVERIFY2(QFile::exists(mTestPath), "parent folder should not removed");
    QVERIFY2(!mDir.dirExists("home", false), "cannot recover");
  }

  void test_rmfile_cannot_recover() {
    TDir mDir;
    QVERIFY(mDir.IsValid());
    QString mTestPath{mDir.path()};
    QVERIFY(mDir.touch("012.txt", "contents in 012.txt"));
    QVERIFY2(mDir.fileExists("012.txt", false), "file 012.txt should exist");

    UndoRedo ur;
    QVERIFY(!ur.undoAvailable());
    QVERIFY(!ur.redoAvailable());

    BATCH_COMMAND_LIST_TYPE rmfileCommand{ACMD::GetInstRMFILE(mTestPath, "012.txt")};
    QVERIFY(ur.Do(rmfileCommand));

    QVERIFY(!mDir.fileExists("012.txt", false));

    QVERIFY(ur.undoAvailable());
    QVERIFY2(ur.Undo(), "succeed and cannot recover silent");
    QVERIFY2(!mDir.fileExists("012.txt", false), "file 012.txt can NOT recover");
  }

  void test_moveToTrash_recover_ok() {
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

  void test_rename_ok() {
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

  void test_mixed_recoverable_and_recoverable_cmds() {
    TDir mDir;
    QVERIFY(mDir.IsValid());
    QString mTestPath{mDir.path()};
    QVERIFY(mDir.touch("file.txt", "contents in file.txt"));
    QVERIFY(mDir.touch("fileToBeDeleted.txt", "contents in fileToBeDeleted.txt"));

    const ACMD& renameCmd = ACMD::GetInstRENAME(mTestPath, "file.txt", "newFileName.txt");
    const ACMD& rmfileCmd = ACMD::GetInstRMFILE(mTestPath, "fileToBeDeleted.txt");

    UndoRedo ur;
    QVERIFY(ur.Do({renameCmd}));                         // file.txt->newFileName.txt
    QVERIFY(!mDir.fileExists("file.txt", false));        //
    QVERIFY(mDir.fileExists("newFileName.txt", false));  //

    QVERIFY(ur.Do({rmfileCmd}));      // rmfile fileToBeDeleted.txt
    QCOMPARE(ur.mUndoStk.size(), 2);  // size = do times = ctrl+Z times

    QVERIFY(ur.undoAvailable());
    QVERIFY(ur.mUndoStk.top().isEmpty());
    QCOMPARE(ur.Undo(), true);                                // undo of file delete operation in silent. empty cmds in fact
    QVERIFY(!mDir.fileExists("fileToBeDeleted.txt", false));  //

    QVERIFY(!mDir.fileExists("file.txt", false));        //
    QVERIFY(mDir.fileExists("newFileName.txt", false));  //

    QVERIFY(ur.undoAvailable());
    QCOMPARE(ur.Undo(), true);                            // newFileName.txt->file.txt
    QVERIFY(!mDir.fileExists("newFileName.txt", false));  //
    QVERIFY(mDir.fileExists("file.txt", false));          //

    QCOMPARE(ur.mRedoStk.size(), 2);  // size = ctrl+Y times
    QVERIFY(ur.redoAvailable());
    QCOMPARE(ur.Redo(), true);                           // file.txt->newFileName.txt
    QVERIFY(!mDir.fileExists("file.txt", false));        //
    QVERIFY(mDir.fileExists("newFileName.txt", false));  //
    QVERIFY(ur.redoAvailable());
    QVERIFY(ur.mRedoStk.top().isEmpty());
    QCOMPARE(ur.Redo(), true);                                // redo of file delete operation in silent. empty cmds in fact
    QVERIFY(!mDir.fileExists("fileToBeDeleted.txt", false));  //
  }

  void test_sync_edit_switch_off_switch_on_two_direction_ok() {
    GlbDataProtect<bool> bkp{SyncModifiyFileSystem::m_syncOperationSw};
    SyncModifiyFileSystem::m_syncOperationSw = false;
    GlbDataProtect<bool> bkpBackDirection{SyncModifiyFileSystem::m_syncBackSw};
    SyncModifiyFileSystem::m_syncBackSw = false;

    // switch off
    TDir mdir;
    QVERIFY(mdir.IsValid());
    QString mTestPath{mdir.path()};
    QVERIFY(mdir.touch("from/anyfile.txt"));
    QVERIFY(mdir.touch("dest/anyfile.txt"));

    const QString basicPath{mTestPath + "/from"};
    const QString syncPath{mTestPath + "/dest"};
    QVERIFY(SyncModifiyFileSystem::SetBasicPath(basicPath));
    QVERIFY(SyncModifiyFileSystem::SetSynchronizedToPaths(syncPath));

    const ACMD& renameCmd = ACMD::GetInstRENAME(basicPath, "anyfile.txt", "newAnyfile.txt");
    const ACMD& syncRenameCmd = ACMD::GetInstRENAME(syncPath, "anyfile.txt", "newAnyfile.txt");

    const ACMD& recoverCmd = ACMD::GetInstRENAME(basicPath, "newAnyfile.txt", "anyfile.txt");
    const ACMD& syncRecoverCmd = ACMD::GetInstRENAME(syncPath, "newAnyfile.txt", "anyfile.txt");

    UndoRedo ur;
    QVERIFY(ur.Do({renameCmd}));
    QVERIFY(ur.undoAvailable());
    QCOMPARE(ur.mUndoStk.size(), 1);
    QCOMPARE(ur.mUndoStk.top(), (BATCH_COMMAND_LIST_TYPE{recoverCmd}));
    QVERIFY(ur.Undo());

    QVERIFY(ur.redoAvailable());
    QCOMPARE(ur.mRedoStk.size(), 1);
    QCOMPARE(ur.mRedoStk.top(), (BATCH_COMMAND_LIST_TYPE{renameCmd}));
    QVERIFY(mdir.fileExists("from/anyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/anyfile.txt", false));

    ur.clear(); // clear two stack

    // switch on
    SyncModifiyFileSystem::m_syncOperationSw = true;

    // sync-back off
    // from -> dest: ok, dest -> from: nok
    SyncModifiyFileSystem::m_syncBackSw = false;

    QVERIFY(ur.Do({renameCmd}));
    QVERIFY(mdir.fileExists("from/newAnyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/newAnyfile.txt", false));

    QVERIFY(ur.undoAvailable());
    QCOMPARE(ur.mUndoStk.size(), 1);
    QCOMPARE(ur.mUndoStk.top(), (BATCH_COMMAND_LIST_TYPE{recoverCmd, syncRecoverCmd}));
    QVERIFY(ur.Undo());
    QVERIFY(mdir.fileExists("from/anyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/anyfile.txt", false));

    QVERIFY(ur.redoAvailable());
    QCOMPARE(ur.mRedoStk.size(), 1);
    QCOMPARE(ur.mRedoStk.top(), (BATCH_COMMAND_LIST_TYPE{syncRenameCmd, renameCmd}));

    ur.clear();

    // dest -> from: nok
    QVERIFY(ur.Do({syncRenameCmd}));
    QVERIFY(!mdir.fileExists("from/newAnyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/newAnyfile.txt", false));

    QVERIFY(ur.undoAvailable());
    QCOMPARE(ur.mUndoStk.size(), 1);
    QCOMPARE(ur.mUndoStk.top(), (BATCH_COMMAND_LIST_TYPE{syncRecoverCmd}));
    QVERIFY(ur.Undo());
    QVERIFY(mdir.fileExists("from/anyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/anyfile.txt", false));

    QVERIFY(ur.redoAvailable());
    QCOMPARE(ur.mRedoStk.size(), 1);
    QCOMPARE(ur.mRedoStk.top(), (BATCH_COMMAND_LIST_TYPE{syncRenameCmd}));

    ur.clear();

    // sync-back on
    // from -> dest: ok, dest -> from: ok
    SyncModifiyFileSystem::m_syncBackSw = true;
    QVERIFY(ur.Do({syncRenameCmd}));
    QVERIFY(mdir.fileExists("from/newAnyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/newAnyfile.txt", false));

    QVERIFY(ur.undoAvailable());
    QCOMPARE(ur.mUndoStk.size(), 1);
    QCOMPARE(ur.mUndoStk.top(), (BATCH_COMMAND_LIST_TYPE{syncRecoverCmd, recoverCmd}));
    QVERIFY(ur.Undo());
    QVERIFY(mdir.fileExists("from/anyfile.txt", false));
    QVERIFY(mdir.fileExists("dest/anyfile.txt", false));

    QVERIFY(ur.redoAvailable());
    QCOMPARE(ur.mRedoStk.size(), 1);
    QCOMPARE(ur.mRedoStk.top(), (BATCH_COMMAND_LIST_TYPE{renameCmd, syncRenameCmd}));
  }

  void global_undoRedo_inst_ok() {
    auto& inst = UndoRedo::GetInst();
    inst.clear();
    QCOMPARE(inst.undoAvailable(), false);
    QCOMPARE(inst.on_Undo(), true); // just skip
    QCOMPARE(inst.redoAvailable(), false);
    QCOMPARE(inst.on_Redo(), true); // just skip
  }
};

#include "UndoRedoTest.moc"
REGISTER_TEST(UndoRedoTest, false)
