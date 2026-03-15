#include <QtTest/QtTest>
#include "PlainTestSuite.h"
#include <QTestEventList>
#include <QSignalSpy>

#include "MemoryKey.h"
#include "BeginToExposePrivateMember.h"
#include "BatchRenameBy.h"
#include "EndToExposePrivateMember.h"
#include "AdvanceRenamerTestTool.h"

#include "AdvanceRenamer.h"
#include "TDir.h"
#include <QDialog>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

using namespace BatchRenameBy;

class BatchRenameByTest : public PlainTestSuite {
  Q_OBJECT
 public:
  TDir mTDir;
 private slots:
  void initTestCase() {  //
    QVERIFY(mTDir.IsValid());
    const QList<FsNodeEntry> nodesEntries  //
        {
            {"replace/pattern/Chris Evans.jpg", false, ""},          //
            {"replace/pattern/Chris Evans.json", false, ""},         //
            {"replace/pattern/Chris Hemsworth_a.pson", false, ""},   //
            {"replace/pattern/Jensen Ackles.mp4", false, ""},        //
            {"replace/pattern/Jensen Ackles.pson", false, ""},       //
            {"replace/pattern/Michael Fassbender.json", false, ""},  //
        };

    QCOMPARE(mTDir.createEntries(nodesEntries), 6);
  }

  void init() {
    GlobalMockObject::reset();
    AdvanceRenamerTestTool::clear();
  }
  void cleanup() { GlobalMockObject::verify(); }

  void GetFilesNeedRename_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");
    QStringList patterns{"Chris Evans.json", "Jensen Ackles.mp4", "Michael Fassbender.json"};
    const QStringList filesNeedRename{GetFilesNeedRename(workPath, patterns)};
    const QStringList beforeSelectedNames{
        "Chris Evans.jpg",          //
        "Chris Evans.json",         //
        "Jensen Ackles.mp4",        //
        "Jensen Ackles.pson",       //
        "Michael Fassbender.json",  //
    };
    QCOMPARE(beforeSelectedNames, filesNeedRename);
  }

  void ReplaceQueryAndConfirm_ok() {
    using namespace AdvanceRenamerTestTool;
    set({QDialog::DialogCode::Rejected, QDialog::DialogCode::Accepted, QDialog::DialogCode::Accepted});
    MOCKER(AdvanceRenamer::execCore).expects(exactly(3)).will(invoke(execCoreMock));

    QString workPath = mTDir.itemPath("replace/pattern");
    const QSet<QString> snapshot1 = mTDir.SnapshotAtPath(workPath);

    QStringList beforePatterns{"Chris Evans.json", "Jensen Ackles.mp4", "Michael Fassbender.json"};
    const QStringList beforeSelectedNames{GetFilesNeedRename(workPath, beforePatterns)};

    // 字符串替换"a"->"X"->"a", 恢复成功
    QString defOldName{"a"};
    QString defNewName{"X"};
    bool disableOldNameEdit{false};

    // 默认入参not crash
    QCOMPARE(ReplaceQueryAndConfirm({}, {}, {}, {}, false), RnmResult::SKIP);

    QCOMPARE(ReplaceQueryAndConfirm(workPath, beforeSelectedNames, defOldName, defNewName, disableOldNameEdit), RnmResult::SKIP);
    const QSet<QString> snapshot2 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot2, snapshot1);  // user skip replace unchange

    QCOMPARE(ReplaceQueryAndConfirm(workPath, beforeSelectedNames, defOldName, defNewName, disableOldNameEdit), RnmResult::ALL_SUCCEED);
    const QSet<QString> snapshot3 = mTDir.SnapshotAtPath(workPath);
    QVERIFY(snapshot3 != snapshot1);
    const QSet<QString> expectAfterRename{
        "Chris EvXns.jpg",          //
        "Chris EvXns.json",         //
        "Chris Hemsworth_a.pson",   //
        "Jensen Ackles.mp4",        //
        "Jensen Ackles.pson",       //
        "MichXel FXssbender.json",  //
    };
    QCOMPARE(snapshot3, expectAfterRename);

    QStringList afterPatterns{"Chris EvXns.json", "Jensen Ackles.mp4", "MichXel FXssbender.json"};
    const QStringList afterSelectedNames{GetFilesNeedRename(workPath, afterPatterns)};

    QCOMPARE(ReplaceQueryAndConfirm(workPath, afterSelectedNames, defNewName, defOldName, disableOldNameEdit), RnmResult::ALL_SUCCEED);

    const QSet<QString> snapshot4 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot4, snapshot1);
  }

  void InsertQueryAndConfirm_ok() {
    using namespace AdvanceRenamerTestTool;
    set({QDialog::DialogCode::Rejected, QDialog::DialogCode::Accepted, QDialog::DialogCode::Accepted});
    MOCKER(AdvanceRenamer::execCore).expects(exactly(3)).will(invoke(execCoreMock));

    QString workPath = mTDir.itemPath("replace/pattern");
    const QSet<QString> snapshot1 = mTDir.SnapshotAtPath(workPath);

    QStringList beforePatterns{"Chris Evans.json", "Jensen Ackles.mp4", "Michael Fassbender.json"};
    const QStringList beforeSelectedNames{GetFilesNeedRename(workPath, beforePatterns)};

    QString strInsert{"best of "};
    int insertAtIndex{0};

    // 默认入参not crash
    QCOMPARE(InsertQueryAndConfirm({}, {}, {}, 0), RnmResult::SKIP);

    QCOMPARE(InsertQueryAndConfirm(workPath, beforeSelectedNames, strInsert, insertAtIndex), RnmResult::SKIP);
    const QSet<QString> snapshot2 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot2, snapshot1);  // user skip insert unchange

    // 1st. 字符串插入"best of ", 0, ok
    QCOMPARE(InsertQueryAndConfirm(workPath, beforeSelectedNames, strInsert, insertAtIndex), RnmResult::ALL_SUCCEED);
    const QSet<QString> snapshot3 = mTDir.SnapshotAtPath(workPath);
    QVERIFY(snapshot3 != snapshot1);
    const QSet<QString> expectAfterRename{
        "Chris Hemsworth_a.pson",           //
        "best of Chris Evans.jpg",          //
        "best of Chris Evans.json",         //
        "best of Jensen Ackles.mp4",        //
        "best of Jensen Ackles.pson",       //
        "best of Michael Fassbender.json",  //
    };
    QCOMPARE(snapshot3, expectAfterRename);

    // 2nd. replace "best of " by "", recover ok
    QStringList afterPatterns{"best of Chris Evans.json", "best of Jensen Ackles.mp4", "best of Michael Fassbender.json"};
    const QStringList afterSelectedNames{GetFilesNeedRename(workPath, afterPatterns)};
    QCOMPARE(ReplaceQueryAndConfirm(workPath, afterSelectedNames, "best of ", "", true), RnmResult::ALL_SUCCEED);
    const QSet<QString> snapshot4 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot4, snapshot1);
  }

  void ReplaceBySpecifiedJson_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");
    MOCKER(ReplaceQueryAndConfirm).expects(exactly(2)).will(returnValue(RnmResult::ALL_SUCCEED));

    // default not crash down
    QCOMPARE(ReplaceBySpecifiedJson({}, {}), 0);

    const QStringList& json2Names{"Chris Evans.json", "Michael Fassbender.json"};
    const QStringList filesNeedRename_cnt3{GetFilesNeedRename(workPath, json2Names)};
    QCOMPARE(filesNeedRename_cnt3.size(), 3);
    QCOMPARE(ReplaceBySpecifiedJson(workPath, json2Names), 3);
    // 3 file renamed by this 2 pattern

    const QStringList& json1Names{"Chris Evans.json"};
    const QStringList filesNeedRename_cnt2{GetFilesNeedRename(workPath, json1Names)};
    QCOMPARE(filesNeedRename_cnt2.size(), 2);
    QCOMPARE(ReplaceBySpecifiedJson(workPath, json1Names), 2);
    // 2 file renamed by this 1 pattern
  }

  void InsertBySpecifiedJson_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");
    MOCKER(InsertQueryAndConfirm).expects(exactly(2)).will(returnValue(RnmResult::ALL_SUCCEED));

    // default not crash down
    QCOMPARE(InsertBySpecifiedJson({}, {}), 0);

    const QStringList& json2Names{"Chris Evans.json", "Michael Fassbender.json"};
    const QStringList filesNeedRename_cnt3{GetFilesNeedRename(workPath, json2Names)};
    QCOMPARE(filesNeedRename_cnt3.size(), 3);
    QCOMPARE(InsertBySpecifiedJson(workPath, json2Names), 3);
    // 3 file renamed by this 2 pattern

    const QStringList& json1Names{"Chris Evans.json"};
    const QStringList filesNeedRename_cnt2{GetFilesNeedRename(workPath, json1Names)};
    QCOMPARE(filesNeedRename_cnt2.size(), 2);
    QCOMPARE(InsertBySpecifiedJson(workPath, json1Names), 2);
    // 2 file renamed by this 1 pattern
  }
};

#include "BatchRenameByTest.moc"
REGISTER_TEST(BatchRenameByTest, false)
