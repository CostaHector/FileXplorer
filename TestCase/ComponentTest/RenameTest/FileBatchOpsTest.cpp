#include <QtTest/QtTest>
#include "PlainTestSuite.h"

#include "FileBatchOps.h"

#include "RenamerKey.h"
#include "Configuration.h"
#include "AdvanceRenamerTestTool.h"

#include "AdvanceRenamer.h"
#include "TDir.h"
#include "UndoRedo.h"
#include <QDialog>

#include <mockcpp/mokc.h>
#include <mockcpp/GlobalMockObject.h>
#include <mockcpp/MockObject.h>
#include <mockcpp/MockObjectHelper.h>
USING_MOCKCPP_NS

    using namespace FileBatchOps;

class FileBatchOpsTest : public PlainTestSuite {
  Q_OBJECT
public:
  TDir mTDir;
private slots:
  void initTestCase() {  //
    QVERIFY(mTDir.IsValid());
    //  space 0x20
    // . period 0x2E
    // 大写字母
    // _ underscore 0x5F
    // 小写字母
    const QList<FsNodeEntry> nodesEntries  //
        {
        {"replace/pattern/Chris Evans 2.png", false, ""},        //
        {"replace/pattern/Chris Evans.jpg", false, ""},          //
        {"replace/pattern/Chris Evans.json", false, ""},         //
        {"replace/pattern/Chris Evans_tn.jpg", false, ""},       //
        {"replace/pattern/Chris Hemsworth_a.pson", false, ""},   //
        {"replace/pattern/Jensen Ackles.mp4", false, ""},        //
        {"replace/pattern/Jensen Ackles.pson", false, ""},       //
        {"replace/pattern/Michael Fassbender.json", false, ""},  //
        };

    QCOMPARE(mTDir.createEntries(nodesEntries), 8);
  }

  void cleanupTestCase() { //
    Configuration().clear();
  }

  void init() {
    GlobalMockObject::reset();
    AdvanceRenamerTestTool::clear();
  }
  void cleanup() { //
    GlobalMockObject::verify();
  }

  void ExpandRelatedFiles_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");
    QStringList patterns{"Chris Evans.json", "Jensen Ackles.mp4", "Michael Fassbender.json"};
    QMap<QString, QString> file2Pattern;
    QMap<QString, QString> expectFile2Pattern
        {
         {"Chris Evans 2.png", "Chris Evans.json"},
         {"Chris Evans.jpg", "Chris Evans.json"},
         {"Chris Evans.json", "Chris Evans.json"},
         {"Chris Evans_tn.jpg", "Chris Evans.json"},
         {"Jensen Ackles.mp4", "Jensen Ackles.mp4"},
         {"Jensen Ackles.pson", "Jensen Ackles.mp4"},
         {"Michael Fassbender.json", "Michael Fassbender.json"},
         };
    const QStringList filesNeedRename{ExpandRelatedFiles(workPath, patterns, &file2Pattern)};
    const QStringList beforeSelectedNames{
        "Chris Evans 2.png",        //
        "Chris Evans.jpg",          //
        "Chris Evans.json",         //
        "Chris Evans_tn.jpg",       //
        "Jensen Ackles.mp4",        //
        "Jensen Ackles.pson",       //
        "Michael Fassbender.json",  //
    };
    QCOMPARE(beforeSelectedNames, filesNeedRename);
    QCOMPARE(expectFile2Pattern, file2Pattern);
  }

  void GetFilesNeedRename_subdirectory_ok() {
    QString workPath = mTDir.itemPath("replace");
    QStringList patterns{"pattern/Chris Evans.json", "pattern/Jensen Ackles.mp4"};
    QMap<QString, QString> file2Pattern;
    QMap<QString, QString> expectFile2Pattern
        {
         {"pattern/Chris Evans 2.png", "pattern/Chris Evans.json"},
         {"pattern/Chris Evans.jpg", "pattern/Chris Evans.json"},
         {"pattern/Chris Evans.json", "pattern/Chris Evans.json"},
         {"pattern/Chris Evans_tn.jpg", "pattern/Chris Evans.json"},
         {"pattern/Jensen Ackles.mp4", "pattern/Jensen Ackles.mp4"},
         {"pattern/Jensen Ackles.pson", "pattern/Jensen Ackles.mp4"},
         };
    const QStringList beforeSelectedNames{
        "pattern/Chris Evans 2.png",        //
        "pattern/Chris Evans.jpg",          //
        "pattern/Chris Evans.json",         //
        "pattern/Chris Evans_tn.jpg",       //
        "pattern/Jensen Ackles.mp4",        //
        "pattern/Jensen Ackles.pson",       //
    };
    const QStringList filesNeedRename{ExpandRelatedFiles(workPath, patterns, &file2Pattern)};
    QCOMPARE(beforeSelectedNames, filesNeedRename);
    QCOMPARE(expectFile2Pattern, file2Pattern);
  }

  void RenameSelectedReplace_ok() {
    using namespace AdvanceRenamerTestTool;
    set({QDialog::DialogCode::Rejected});
    MOCKER(AdvanceRenamer::execCore).expects(exactly(1)).will(invoke(execCoreMock));

    QString workPath = mTDir.itemPath("replace/pattern");
    const QSet<QString> snapshot1 = mTDir.SnapshotAtPath(workPath);

    QStringList beforePatterns{"Chris Evans.json", "Jensen Ackles.mp4", "Michael Fassbender.json"};
    const QStringList beforeSelectedNames{ExpandRelatedFiles(workPath, beforePatterns)};

    // 默认入参not crash
    QCOMPARE(RenameSelectedReplace({}, {}), OpStatus::SKIP);

    // user skip replace unchange
    QCOMPARE(RenameSelectedReplace(workPath, beforeSelectedNames), OpStatus::SKIP);
    const QSet<QString> snapshot2 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot2, snapshot1);
  }

  void RenameSelectedInsert_ok() {
    using namespace AdvanceRenamerTestTool;
    set({QDialog::DialogCode::Rejected});
    MOCKER(AdvanceRenamer::execCore).expects(exactly(1)).will(invoke(execCoreMock));

    QString workPath = mTDir.itemPath("replace/pattern");
    const QSet<QString> snapshot1 = mTDir.SnapshotAtPath(workPath);

    QStringList beforePatterns{"Chris Evans.json", "Jensen Ackles.mp4", "Michael Fassbender.json"};
    const QStringList beforeSelectedNames{ExpandRelatedFiles(workPath, beforePatterns)};

    // 默认入参not crash
    QCOMPARE(RenameSelectedInsert({}, {}), OpStatus::SKIP);

    QCOMPARE(RenameSelectedInsert(workPath, beforeSelectedNames), OpStatus::SKIP);
    const QSet<QString> snapshot2 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot2, snapshot1);  // user skip insert unchange
  }

  void RenameSelectedNumerizer_ok() {
    setConfig(RenamerKey::NAME_EXT_INDEPENDENT, true);
    setConfig(RenamerKey::NUMERIAZER_UNIQUE_EXT_COUNTER, false);
    setConfig(RenamerKey::NUMERIAZER_NO_FORMAT, " %1");
    setConfig(RenamerKey::NUMERIAZER_NO_FORMAT_DEFAULT_INDEX, 0);

    using namespace AdvanceRenamerTestTool;
    set({QDialog::DialogCode::Rejected, QDialog::DialogCode::Accepted});
    MOCKER(AdvanceRenamer::execCore).expects(exactly(2)).will(invoke(execCoreMock));

    QString workPath = mTDir.itemPath("replace/pattern");
    const QSet<QString> snapshot1 = mTDir.SnapshotAtPath(workPath);

    QStringList beforePatterns{"Chris Evans.json"};
    const QStringList beforeSelectedNames{ExpandRelatedFiles(workPath, beforePatterns)};

    // 默认入参not crash
    QCOMPARE(RenameSelectedNumerizer({}, {}), OpStatus::SKIP);

    QCOMPARE(RenameSelectedNumerizer(workPath, beforeSelectedNames), OpStatus::SKIP);
    const QSet<QString> snapshot2 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot2, snapshot1);  // user skip insert unchange

    // 编号, 后缀统一计数, 后缀和基础名分开, 从0开始计数, 格式为" %1", 预期只改两个文件名
    QCOMPARE(RenameSelectedNumerizer(workPath, beforeSelectedNames), OpStatus::ALL_SUCCEED);
    const QSet<QString> snapshot3 = mTDir.SnapshotAtPath(workPath);
    QVERIFY(snapshot3 != snapshot1);
    const QSet<QString> expectAfterRename{
        "Chris Evans 2 0.png",       //
        "Chris Evans 2 1.jpg",       //
        "Chris Evans 2 2.json",      //
        "Chris Evans 2 3.jpg",      //
        "Chris Hemsworth_a.pson",  //
        "Jensen Ackles.mp4",       //
        "Jensen Ackles.pson",      //
        "Michael Fassbender.json"  //
    };
    QCOMPARE(snapshot3, expectAfterRename);

    // undo it
    QCOMPARE(UndoRedo::on_Undo(), true);
    const QSet<QString> snapshot4 = mTDir.SnapshotAtPath(workPath);
    QCOMPARE(snapshot4, snapshot1);
  }

  void RenameWithPatternReplace_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");

    MOCKER(RenameSelected).expects(exactly(2)).will(returnValue(OpStatus::ALL_SUCCEED));
    // default not crash down
    QCOMPARE(RenameWithPatternReplace({}, {}), 0);

    const QStringList& json2Names{"Chris Evans.json", "Michael Fassbender.json"};
    const QStringList filesNeedRename_cnt3{ExpandRelatedFiles(workPath, json2Names)};
    QCOMPARE(filesNeedRename_cnt3.size(), 4 + 1); // 4Chris Evans+1Michael Fassbender
    QCOMPARE(RenameWithPatternReplace(workPath, json2Names), 4 + 1);
    // 3 file renamed by this 2 pattern

    const QStringList& json1Names{"Chris Evans.json"};
    const QStringList filesNeedRename_cnt2{ExpandRelatedFiles(workPath, json1Names)};
    QCOMPARE(filesNeedRename_cnt2.size(), 4);
    QCOMPARE(RenameWithPatternReplace(workPath, json1Names), 4);
    // 2 file renamed by this 1 pattern
  }

  void RenameWithPatternInsert_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");
    MOCKER(RenameSelected).expects(exactly(2)).will(returnValue(OpStatus::ALL_SUCCEED));

    // default not crash down
    QCOMPARE(RenameWithPatternInsert({}, {}), 0);

    const QStringList& json2Names{"Chris Evans.json", "Michael Fassbender.json"};
    const QStringList filesNeedRename_cnt3{ExpandRelatedFiles(workPath, json2Names)};
    QCOMPARE(filesNeedRename_cnt3.size(), 4 + 1);
    QCOMPARE(RenameWithPatternInsert(workPath, json2Names), 4 + 1);
    // 3 file renamed by this 2 pattern

    const QStringList& json1Names{"Chris Evans.json"};
    const QStringList filesNeedRename_cnt2{ExpandRelatedFiles(workPath, json1Names)};
    QCOMPARE(filesNeedRename_cnt2.size(), 4);
    QCOMPARE(RenameWithPatternInsert(workPath, json1Names), 4);
    // 2 file renamed by this 1 pattern
  }

  void NumerizerBySpecifiedJson_ok() {
    QString workPath = mTDir.itemPath("replace/pattern");
    MOCKER(RenameSelected).expects(exactly(2)).will(returnValue(OpStatus::ALL_SUCCEED));

    // default not crash down
    QCOMPARE(RenameWithPatternNumerizer({}, {}), 0);

    const QStringList& json2Names{"Chris Evans.json", "Michael Fassbender.json"};
    const QStringList filesNeedRename_cnt3{ExpandRelatedFiles(workPath, json2Names)};
    QCOMPARE(filesNeedRename_cnt3.size(), 4 + 1);
    QCOMPARE(RenameWithPatternNumerizer(workPath, json2Names), 4 + 1);
    // 3 file renamed by this 2 pattern

    const QStringList& json1Names{"Chris Evans.json"};
    const QStringList filesNeedRename_cnt2{ExpandRelatedFiles(workPath, json1Names)};
    QCOMPARE(filesNeedRename_cnt2.size(), 4);
    QCOMPARE(RenameWithPatternNumerizer(workPath, json1Names), 4);
    // 2 file renamed by this 1 pattern
  }
};

#include "FileBatchOpsTest.moc"
REGISTER_TEST(FileBatchOpsTest, false)
