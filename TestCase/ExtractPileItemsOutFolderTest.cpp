#include <QCoreApplication>
#include <QtTest>
#include "pub/MyTestSuite.h"
#include "FileOperation/FileOperatorPub.h"
#include "pub/BeginToExposePrivateMember.h"
#include "Tools/ExtractPileItemsOutFolder.h"
#include "pub/EndToExposePrivateMember.h"

class ExtractPileItemsOutFolderTest : public MyTestSuite {
  Q_OBJECT
 public:
 private slots:
  void initTestCase() {}
  void cleanupTestCase() {}

  void init() {}
  void cleanup() {}

  void test_cannotExtract_ok() {
    QStringList noStrictAPileFiles;
    noStrictAPileFiles << "random name.mp4";
    noStrictAPileFiles << "another random name.mp4";
    QVERIFY(!ExtractPileItemsOutFolder::CanExtractOut(noStrictAPileFiles));
  }

  void test_canExtract_ok() {
    QStringList aPileFiles;
    aPileFiles << "name.mp4"
               << "name.mkv"
               << "name.ts"
               << "name.avi"
               << "name.wmv"
               << "name.json"
               << "name.jpg"
               << "name 1.jpeg"
               << "name 2.webp"
               << "name 10.png"
               << "name - 10.png";
    QVERIFY(ExtractPileItemsOutFolder::CanExtractOut(aPileFiles));
  }

  void test_foldersCntNeedExtract_ok() {
    QMap<QString, QStringList> folder2PileItems;
    folder2PileItems["a_pile"] << "name.mp4"        // 1
                               << "name.mkv"        // 2
                               << "name.ts"         // 3
                               << "name.avi"        // 4
                               << "name.wmv"        // 5
                               << "name.json"       // 6
                               << "name.jpg"        // 7
                               << "name 1.jpeg"     // 8
                               << "name 2.webp"     // 9
                               << "name 10.png"     // 10
                               << "name - 10.png";  // already exist in parent folder, will not extract out
    folder2PileItems["not_a_pile"] << "random name.mp4"
                                   << "another random name.mp4";
    const QStringList alreadyExistItems{"name - 10.png"};
    const int foldersNeedExtractCnt = 1;
    const QString curPath{"not existed out directory"};

    ExtractPileItemsOutFolder epiof;
    QCOMPARE(epiof(curPath, folder2PileItems, alreadyExistItems),  //
             foldersNeedExtractCnt);
    using namespace FileOperatorType;
    QCOMPARE(epiof.m_cmds.size(), 11);  // 10 move out commands, 1 remove commands
    QVERIFY(epiof.m_cmds.contains(ACMD{MOVETOTRASH, {curPath, "a_pile"}}));
    QVERIFY(epiof.m_cmds.contains(ACMD{RENAME, {curPath + "/a_pile", "name.mp4", curPath, "name.mp4"}}));
    QVERIFY(epiof.m_cmds.contains(ACMD{RENAME, {curPath + "/a_pile", "name 10.png", curPath, "name 10.png"}}));
    bool ret = epiof.m_cmds.contains(ACMD{RENAME, {curPath + "/a_pile", "name - 10.png", curPath, "name - 10.png"}});
    QVERIFY(!ret);
  }
};

ExtractPileItemsOutFolderTest g_ExtractPileItemsOutFolderTest;
#include "ExtractPileItemsOutFolderTest.moc"
